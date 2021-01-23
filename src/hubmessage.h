#ifndef _MSGHUB_HUBMESSAGE_H_
#define _MSGHUB_HUBMESSAGE_H_

#include <array>
#include <cstddef>
#include <functional>
#include <string_view>
#include <iterator>
#include <vector>

#include "charbuf.h"

class hubmessage
{
  public:
	enum action : char { subscribe, unsubscribe, publish };
	enum { version = 0x1 };
	enum { cookie = 0xF00D ^ (version << 8) };
	enum { messagesize = 0x2000 };

    hubmessage(action a={}, std::string_view topic={}, const_charbuf msg = {});

	bool             verify()     const;

	action           get_action() const;
	std::string_view topic()      const;
	const_charbuf    body()       const;

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
    std::array<char, messagesize - sizeof(headers_)> payload_;

  public:
    // input buffer views
    auto header_buf() {
        return boost::asio::buffer(&headers_, sizeof(headers_));
    }

    auto payload_area() {
        return boost::asio::buffer(payload_, headers_.topiclen + headers_.bodylen);
    }

    // output buffer views
    auto on_the_wire() const {
        return std::vector { 
            boost::asio::buffer(&headers_, sizeof(headers_)),
            boost::asio::buffer(payload_, headers_.topiclen + headers_.bodylen),
        };
    }
};

#include <deque>
typedef std::deque<hubmessage> hubmessage_queue;

#endif
