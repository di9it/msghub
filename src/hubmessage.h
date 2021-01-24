#ifndef _MSGHUB_HUBMESSAGE_H_
#define _MSGHUB_HUBMESSAGE_H_

#include <array>
#include <cstddef>
#include <functional>
#include <string_view>
#include <iterator>
#include <vector>

#include "span.h"
#include <boost/container/small_vector.hpp>

class hubmessage
{
  public:
	enum action : char { subscribe, unsubscribe, publish };
	enum { version = 0x1 };
	enum { cookie = 0xF00D ^ (version << 8) };
	enum { messagesize = 0x2000 };

    hubmessage(action a={}, std::string_view topic={}, span<char const> msg = {});

	bool             verify()     const;

	action           get_action() const;
	std::string_view topic()      const;
	span<char const> body()       const;

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
    boost::container::small_vector<char, 242> payload_;

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
    auto on_the_wire() const {
        return std::vector { 
            boost::asio::buffer(&headers_, sizeof(headers_)),
            boost::asio::buffer(payload_.data(), payload_.size())
        };
    }
};

#include <deque>
typedef std::deque<hubmessage> hubmessage_queue;

#endif
