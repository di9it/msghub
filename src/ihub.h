#pragma once

#include <memory>

namespace msghublib {
    class hubmessage;

    namespace detail {
        class hubclient;
        class hubconnection;

        struct ihub
        {
            virtual void distribute(std::shared_ptr<hubclient> const& subscriber, hubmessage const& msg) = 0;
            virtual void deliver(hubmessage const& msg) = 0;
        };
    }
}  // namespace msghublib
