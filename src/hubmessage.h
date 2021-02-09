#ifndef _MSGHUB_HUBMESSAGE_H_
#define _MSGHUB_HUBMESSAGE_H_

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <iterator>
#include <memory>

#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/vector.hpp>

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
	const char* data() const;
	char* data();

	hubmessage();
	
	bool verify() const;

	size_t header_length() const;
	size_t length() const;
	//const char* payload() const;
	
	char* payload();
	size_t payload_length() const;
	
	size_t topic_length() const;
	char* topic();
	
	action get_action() const;
	void set_action(action action);

    packet::headers_t& headers();
    packet::headers_t const& headers() const;
	char* body();
	size_t body_length() const;
	void set_message(const std::string& subj);
	void set_message(const std::string& subj, const std::vector<char>& msg);
};

typedef std::deque<hubmessage> hubmessage_queue;

#endif
