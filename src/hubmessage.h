#ifndef _MSGHUB_HUBMESSAGE_H_
#define _MSGHUB_HUBMESSAGE_H_

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <iterator>
#include <memory>

#include "charbuf.h"

class hubmessage
{
public:
	enum action : char { subscribe, unsubscribe, publish };
	enum { version = 0x1 };
	enum { cookie = 0xF00D | (version << 8) };
	enum { messagesize = 0x2000 };

private:
	#pragma pack(push, 1)
	union packet
	{
		struct headers_t
		{
			uint16_t	topiclen;
			uint16_t	bodylen;
			action		msgaction;
			uint16_t	magic;
			char		payload[1];
		} headers;
		char data[messagesize];
	};
	#pragma pack(pop)
	packet data_;

public:
	char const* data() const;
	char* data();

	hubmessage();
	
	bool verify() const;

	size_t header_length() const;
	size_t length() const;
	//const char* payload() const;
	
    charbuf payload();
    const_charbuf payload() const;
	size_t payload_length() const;
	
	size_t topic_length() const;
    std::string_view topic() const;
	
	action get_action() const;
	void set_action(action action);

    packet::headers_t& headers();
    packet::headers_t const& headers() const;

    charbuf body();
    const_charbuf body() const;
	size_t body_length() const;
    void set_message(std::string_view topic);
    void set_message(std::string_view topic, const_charbuf msg);
};

typedef std::deque<hubmessage> hubmessage_queue;

#endif
