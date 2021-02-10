#include "msghub.h"

#include <utility>

#include "hubclient.h"
#include "hubconnection.h"
#include "ihub.h"

using boost::asio::ip::tcp;

namespace msghublib {

    class msghub::impl : public detail::ihub,
                         public std::enable_shared_from_this<impl> {
      public:
        using any_io_executor = boost::asio::any_io_executor;
        using hubclient = detail::hubclient;
        using hubconnection = detail::hubconnection;

      private:
        any_io_executor executor_;
        boost::asio::executor_work_guard<any_io_executor> work_ =
            make_work_guard(executor_);
        tcp::acceptor acceptor_;
        std::shared_ptr<hubconnection> remote_hub_; // using std::atomic_* accessors

        // the subscriptions are under mutex
        std::mutex mutable mutex_;
        std::map<std::string, msghub::onmessage> local_subs_;
        std::multimap<std::string, std::weak_ptr<hubclient>> remote_subs_;

      public:
        explicit impl(any_io_executor const& executor)
            : executor_(executor)
            , acceptor_(make_strand(executor))
        {}

        void stop() {
            {
                std::shared_ptr<hubconnection> rhub;
                if (auto p = std::atomic_exchange(&remote_hub_, rhub)) {
                    p->close(false);
                }
            }

            if (!weak_from_this().expired()) {
                post(acceptor_.get_executor(),
                     [this, self = shared_from_this()] {
                         if (acceptor_.is_open()) {
                             acceptor_.cancel();
                         }
                     });
            } else if (acceptor_.is_open()) {
                acceptor_.cancel();
            }

            work_.reset();
        }

        ~impl() { stop(); }

        void connect(const std::string& hostip, uint16_t port, error_code& ec) {
            ec = {};
            auto p = std::make_shared<hubconnection>(executor_, *this);

            if (p->init(hostip, port, ec); !ec) {
                atomic_store(&remote_hub_, p);
            }
        }

        void create(uint16_t port, error_code& ec) {
            ec = {};
            try {
                acceptor_.open(tcp::v4(), ec);
                if (!ec) acceptor_.set_option(tcp::acceptor::reuse_address(true), ec);
                if (!ec) acceptor_.bind({ {}, port }, ec);
                if (!ec) acceptor_.listen(acceptor_.max_listen_connections, ec);

                if (!ec) {
                    accept_next();

                    auto p = std::make_shared<hubconnection>(executor_, *this);

                    if (p->init("localhost", port, ec); !ec) {
                        atomic_store(&remote_hub_, p);
                    }
                }
            } catch (system_error const& se) {
                ec = se.code();
            }
        }

        void publish(std::string_view topic, span<char const> message, error_code& ec) {
            ec = {};
            if (auto p = atomic_load(&remote_hub_)) {
                p->async_send({ hubmessage::action::publish, topic, message });
            } else {
                ec = hub_errc::hub_not_connected;
            }
        }

        void unsubscribe(const std::string& topic, error_code& ec) {
            ec = {};
            std::unique_lock lk(mutex_);
            if (auto it = local_subs_.find(topic); it != local_subs_.end()) {
                /*it =*/local_subs_.erase(it);
                lk.unlock();

                if (auto p = atomic_load(&remote_hub_)) {
                    p->send({ hubmessage::action::unsubscribe, topic }, ec);
                } else {
                    ec = hub_errc::hub_not_connected;
                }
            }
        }

        void subscribe(const std::string& topic, const msghub::onmessage& handler, error_code& ec) {
            ec = {};
            std::unique_lock lk(mutex_);
            if (auto [it, ins] = local_subs_.emplace(topic, handler); ins) {
                lk.unlock();
                if (auto p = atomic_load(&remote_hub_)) {
                    p->send({ hubmessage::action::subscribe, topic }, ec);
                    // TODO(sehe): wait feedback from server here?
                } else {
                    ec = hub_errc::hub_not_connected;
                }
            } else {
                // just update the handler
                it->second = handler; // overwrite
            }
        }

      private:
        msghub::onmessage const& lookup_handler(std::string_view topic) const {
            static const msghub::onmessage no_handler = [](auto... /*unused*/) {};

            std::lock_guard lk(mutex_);
            auto it = local_subs_.find(std::string(topic));
            return (it == local_subs_.end()) ? no_handler : it->second;
        }

        void distribute(std::shared_ptr<hubclient> const& subscriber, hubmessage const& msg) override {
            std::string topic(msg.topic());
            std::unique_lock lk(mutex_);
            auto range = remote_subs_.equal_range(topic);

            switch (msg.get_action()) {
            case hubmessage::action::publish:
                for (auto it = range.first; it != range.second;) {
                    if (auto alive = it->second.lock()) {
                        alive->send(msg);
                        ++it;
                    } else {
                        it = remote_subs_.erase(it);
                    }
                }
                break;

            case hubmessage::action::subscribe:
#if __cpp_lib_erase_if // allows us to write that in one go:
                std::erase_if(remote_subs_, [](auto& p) { return p.second.expired(); });
#endif
                remote_subs_.emplace(topic, subscriber);
                break;

            case hubmessage::action::unsubscribe:
                for (auto it = range.first; it != range.second;) {
                    if (auto alive = it->second.lock();
                        !alive || alive == subscriber) {
                        it = remote_subs_.erase(it);
                    } else {
                        ++it;
                    }
                }
                break;

            default:
                break;
            }
        }

        void deliver(hubmessage const& msg) override {
            lookup_handler(msg.topic())(msg.topic(), msg.body());
        }

        void accept_next() {
            auto subscriber =
                std::make_shared<hubclient>(acceptor_.get_executor(), *this);

            // Schedule next accept
            acceptor_.async_accept(
                subscriber->socket(),
                [=, this, self = shared_from_this()](error_code ec) {
                    handle_accept(subscriber, ec);
                });
        }

        void handle_accept(std::shared_ptr<hubclient> const& client, error_code error) {
            if (!error) {
                client->start();
                accept_next();
            } else {
                //// TODO: Handle IO error - on thread exit
            }
        }
    };

} // namespace msghublib

namespace msghublib {

    /*explicit*/ msghub::msghub(boost::asio::any_io_executor executor)
        : pimpl(std::make_shared<impl>(executor)) {}

    msghub::~msghub() = default;

    // pimpl relays
    void msghub::stop()
        { return pimpl->stop();                            } 
    void msghub::connect(const std::string& hostip, uint16_t port, error_code& ec)
        { pimpl->connect(hostip, port, ec);                } 
    void msghub::create(uint16_t port, error_code& ec)
        { pimpl->create(port, ec);                         } 
    void msghub::unsubscribe(const std::string& topic, error_code& ec)
        { pimpl->unsubscribe(topic, ec);                   } 
    void msghub::subscribe(const std::string& topic, onmessage handler, error_code& ec)
        { pimpl->subscribe(topic, std::move(handler), ec); } 
    void msghub::publish(std::string_view topic, span<char const> message, error_code& ec)
        { pimpl->publish(topic, message, ec);              } 

    // convenience throwing wrappers
    void msghub::connect(const std::string& hostip, uint16_t port) {
        error_code ec;
        connect(hostip, port, ec);
        if (ec) throw system_error(ec);
    }

    void msghub::create(uint16_t port) {
        error_code ec;
        create(port, ec);
        if (ec) throw system_error(ec);
    }

    void msghub::unsubscribe(const std::string& topic) {
        error_code ec;
        unsubscribe(topic, ec);
        if (ec) throw system_error(ec);
    }

    void msghub::subscribe(const std::string& topic, onmessage handler) {
        error_code ec;
        subscribe(topic, handler, ec);
        if (ec) throw system_error(ec);
    }

    void msghub::publish(std::string_view topic, span<char const> message) {
        error_code ec;
        publish(topic, message, ec);
        if (ec) throw system_error(ec);
    }

} // namespace msghublib
