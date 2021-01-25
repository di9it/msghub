#pragma once

#include "ihub.h"
#include "msghub.h"
#include "hubconnection.h"
#include "hubclient.h"
#include "hubmessage.h"

#include <memory>
#include <string_view>
#include <vector>
#include <map>
#include <mutex>

using boost::asio::ip::tcp;

namespace msghublib { namespace detail {

    class msghub_impl
      : public ihub,
        public std::enable_shared_from_this<msghub_impl>
    {
      public:
        using any_io_executor = boost::asio::any_io_executor;

      private:
        std::mutex mutable mutex_;
        std::map<std::string, msghub::onmessage>              local_subs_;
        std::multimap<std::string, std::weak_ptr<hubclient> > remote_subs_;

        msghub::onmessage const& lookup_handler(std::string_view topic) const;
      private:
        any_io_executor executor_;
        boost::asio::executor_work_guard<any_io_executor>
            work_ = make_work_guard(executor_);
        tcp::acceptor acceptor_;
        std::shared_ptr<hubconnection> remote_hub_;

      public:
        msghub_impl(any_io_executor io_service);
        ~msghub_impl();
        bool connect(const std::string& hostip, uint16_t port);
        bool create(uint16_t port);
        bool publish(std::string_view topic, span<char const> message);

        bool unsubscribe(const std::string& topic);
        bool subscribe(const std::string& topic, msghub::onmessage handler);

        void stop();

      private:

        void distribute(std::shared_ptr<hubclient> const& subscriber, hubmessage const& msg);
        void deliver(hubmessage const& msg);
        void accept_next();
        void handle_accept(std::shared_ptr<hubclient> const& session, const boost::system::error_code& error);
    };

} }
