#include "msghub_impl.h"

#include <atomic>
#include "ihub.h"
#include "hubconnection.h"
#include "hubclient.h"
#include "hubmessage.h"

#include <mutex>
#include <span.h>
#include <memory>
#include <functional>
#include <map>

#include <boost/asio.hpp>

namespace msghublib { namespace detail {

    using boost::asio::ip::tcp;

    msghub_impl::msghub_impl(boost::asio::any_io_executor executor)
        : executor_(executor)
        , acceptor_(make_strand(executor))
    {}

    void msghub_impl::stop()
    {
        {
            std::shared_ptr<hubconnection> rhub;
            if (auto p = std::atomic_exchange(&remote_hub_, rhub))
                p->close(false);
        }

        if (!weak_from_this().expired()) {
            post(acceptor_.get_executor(), [this, self = shared_from_this()] {
                if (acceptor_.is_open())
                    acceptor_.cancel();
            });
        } else {
            if (acceptor_.is_open())
                acceptor_.cancel();
        }

        if (0) {
            std::lock_guard lk(mutex_);
            for (auto& [_, client] : remote_subs_)
                if (auto alive = client.lock())
                    alive->stop();
        }

        work_.reset();
    }

    msghub_impl::~msghub_impl()
    {
        stop();
    }

    bool msghub_impl::connect(const std::string& hostip, uint16_t port)
    {
        auto p = std::make_shared<hubconnection>(executor_, *this);

        if (p->init(hostip, port)) {
            atomic_store(&remote_hub_, p);
            return true;
        }

        return false;
    }

    bool msghub_impl::create(uint16_t port)
    {
        try {
            acceptor_.open(tcp::v4());
            acceptor_.set_option(tcp::acceptor::reuse_address(true));
            acceptor_.bind({{}, port});
            acceptor_.listen();

            accept_next();

            auto p = std::make_shared<hubconnection>(executor_, *this);

            if (p->init("localhost", port)) {
                atomic_store(&remote_hub_, p);
                return true;
            }
        } catch(boost::system::system_error const&) { }
        return false;
    }

    bool msghub_impl::publish(std::string_view topic, span<char const> message)
    {
        if (auto p = atomic_load(&remote_hub_)) {
            return p->write({ hubmessage::action::publish, topic, message });
        }
        return false;
    }

    msghub::onmessage const& msghub_impl::lookup_handler(std::string_view topic) const {
        static const msghub::onmessage no_handler = [](auto...) {};

        std::lock_guard lk(mutex_);
        auto it = local_subs_.find(std::string(topic));
        return (it == local_subs_.end())
            ? no_handler
            : it->second;
    }

    bool msghub_impl::unsubscribe(const std::string& topic)
    {
        std::unique_lock lk(mutex_);
        if (auto it = local_subs_.find(topic); it != local_subs_.end()) {
            /*it =*/ local_subs_.erase(it);
            lk.unlock();

            if (auto p = atomic_load(&remote_hub_)) {
                return p->write({hubmessage::action::unsubscribe, topic}, true);
            }
        }
        return false;
    }

    bool msghub_impl::subscribe(const std::string& topic, msghub::onmessage handler)
    {
        std::unique_lock lk(mutex_);
        if (auto [it,ins] = local_subs_.emplace(topic, handler); ins) {
            lk.unlock();
            if (auto p = atomic_load(&remote_hub_)) {
                return p->write({hubmessage::action::subscribe, topic}, true);
                // TODO: wait feedback from server here?
            }
        } else {
            // just update the handler
            it->second = handler; // overwrite
            return true;
        }

        return false;
    }

    void msghub_impl::distribute(std::shared_ptr<hubclient> const& subscriber, hubmessage const& msg)
    {
        std::string topic(msg.topic());
        std::unique_lock lk(mutex_);
        auto range = remote_subs_.equal_range(topic);

        switch (msg.get_action())
        {
        case hubmessage::action::publish:
            for (auto it = range.first; it != range.second;)
            {
                if (auto alive = it->second.lock()) {
                    alive->write(msg);
                    ++it;
                }
                else it = remote_subs_.erase(it);
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
                if (auto alive = it->second.lock(); !alive || alive == subscriber)
                    it = remote_subs_.erase(it);
                else ++it;
            }
            break;

        default:
            break;
        }
    }

    void msghub_impl::deliver(hubmessage const& msg)
    { 
        lookup_handler(msg.topic())(msg.topic(), msg.body());
    }

    void msghub_impl::accept_next()
    {
        auto subscriber = std::make_shared<hubclient>(acceptor_.get_executor(), *this);

        // Schedule next accept
        acceptor_.async_accept(subscriber->socket(),
            [=, this, self = shared_from_this()](boost::system::error_code ec) {
                handle_accept(subscriber, ec);
            });
    }

    void msghub_impl::handle_accept(std::shared_ptr<hubclient> const& client, const boost::system::error_code& error)
    {
        if (!error)
        {
            client->start();
            accept_next();
        }
        else
        {
            //// TODO: Handle IO error - on thread exit
            //int e = error.value();
        }
    }

} }
