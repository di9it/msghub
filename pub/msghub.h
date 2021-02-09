#pragma once

#include <boost/system/error_code.hpp>
#include <memory>
#include <boost/asio.hpp>
#include <string>
#include <string_view>
#include "hub_error.h"
#include "span.h"

namespace msghublib {

    class msghub
    {
      public:
        typedef std::function< void(std::string_view topic, span<char const> message) > onmessage;
        
      public:
        explicit msghub(boost::asio::any_io_executor);
        ~msghub();

        //void connect(const std::string& hostip, uint16_t port);
        void connect(const std::string& hostip, uint16_t port, error_code& ec);
        void create(uint16_t port, error_code& ec);

        void unsubscribe(const std::string& topic, error_code& ec);
        void subscribe(const std::string& topic, onmessage handler, error_code& ec);
        void publish(std::string_view topic, span<char const> message, error_code& ec);

        // Treat string literals specially, not including the terminating NUL
        template <size_t N>
        void publish(std::string_view topic, char const (&literal)[N], error_code& ec) {
            static_assert(N>0);
            publish(topic, span<char const>{literal, N-1}, ec);
        }

        void stop();

        // convenience throwing wrappers
        void connect(const std::string& hostip, uint16_t port);
        void create(uint16_t port);
        void unsubscribe(const std::string& topic);
        void subscribe(const std::string& topic, onmessage handler);
        void publish(std::string_view topic, span<char const> message);

        // Treat string literals specially, not including the terminating NUL
        template <size_t N>
        void publish(std::string_view topic, char const (&literal)[N]) {
            error_code ec;
            publish(topic, literal, ec);
            if (ec) throw system_error(ec);
        }
        
      private:
        class impl;
        std::shared_ptr<impl> pimpl;
    };
}
