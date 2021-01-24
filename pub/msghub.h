#pragma once

#include <string>
#include <vector>
#include <cstdint>

#include <memory>
#include <boost/asio.hpp>
#include "span.h"

namespace msghublib {

    namespace detail { class msghub_impl; }

    class msghub
    {
      public:
        typedef std::function< void(std::string_view topic, span<char const> message) > onmessage;
        
      public:
        explicit msghub(boost::asio::any_io_executor);
        ~msghub();

        bool connect(const std::string& hostip, uint16_t port);
        bool create(uint16_t port);

        bool unsubscribe(const std::string& topic);
        bool subscribe(const std::string& topic, onmessage handler);
        bool publish(std::string_view topic, span<char const> message);

        // Treat string literals specially, not including the terminating NUL
        template <size_t N>
        bool publish(std::string_view topic, char const (&literal)[N]) {
            static_assert(N>0);
            return publish(topic, span<char const>{literal, N-1});
        }

        void stop();
        
      private:
        std::shared_ptr<detail::msghub_impl> pimpl;
    };
}
