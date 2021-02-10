#pragma once

#include <boost/asio/buffer.hpp>
#include <functional>
#include <string_view>

#include "span.h"
#include <boost/container/small_vector.hpp>
#include <deque>

namespace msghublib {

class hubmessage
{
  public:
    // the following affect on-the-wire compatiblity
	enum action : char { subscribe, unsubscribe, publish };
	enum { version = 0x1 };
	enum { cookie = 0xF00D ^ (version << 8) };
	enum { messagesize = 0x2000 };
    // the following does NOT affect on-the-wire compatiblity
    enum { preallocated = 196 };

    hubmessage(action a={}, std::string_view topic={}, span<char const> msg = {});

	[[nodiscard]] bool             verify()     const;
	[[nodiscard]] action           get_action() const;
	[[nodiscard]] std::string_view topic()      const;
	[[nodiscard]] span<char const> body()       const;

  private:
	#pragma pack(push, 1)
    struct headers_t {
        uint16_t	topiclen;
        uint16_t	bodylen;
        action		msgaction;
        uint16_t	magic;
    };
	#pragma pack(pop)

    headers_t headers_;
    boost::container::small_vector<char, preallocated> payload_;

  public:
    // input buffer views
    auto header_buf() {
        return boost::asio::buffer(&headers_, sizeof(headers_));
    }

    auto payload_area() {
        payload_.resize(headers_.topiclen + headers_.bodylen);
        return boost::asio::buffer(payload_.data(), payload_.size());
    }

    // output buffer views
    [[nodiscard]] auto on_the_wire() const {
        return std::array<boost::asio::const_buffer, 2> { 
            boost::asio::buffer(&headers_, sizeof(headers_)),
            boost::asio::buffer(payload_.data(), payload_.size())
        };
    }
};

using hubmessage_queue = std::deque<hubmessage>;

}  // namespace msghublib
