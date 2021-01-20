#include "hubmessage.h"
//#include "msghub_types.h"
//
//#include <cstdint>
//#include <cstdio>
//#include <cstdlib>
//#include <cstring>
//#include <deque>
//#include <iterator>
//
//#include <memory>
//#include <boost/iostreams/device/array.hpp>
//#include <boost/iostreams/stream.hpp>
//#include <boost/archive/binary_oarchive.hpp>
//#include <boost/archive/binary_iarchive.hpp>
//#include <boost/serialization/vector.hpp>

hubmessage::hubmessage()
{}

const char* hubmessage::data() const
{
	return data_.data;
}

char* hubmessage::data()
{
	return data_.data;
}

size_t hubmessage::header_length() const
{
    static_assert(0 == offsetof(packet, headers));
    return offsetof(packet::headers_t, payload);
}

size_t hubmessage::length() const
{
	return header_length() + payload_length();
}

char* hubmessage::payload()
{
	return headers().payload;
}

char* hubmessage::topic()
{
	return payload();
}

//char* hubmessage::payload() const
//{
//	return headers().payload;
//}

size_t hubmessage::payload_length() const
{
	return headers().topiclen + headers().bodylen;
}

size_t hubmessage::topic_length() const
{
	return headers().topiclen;
}

hubmessage::packet::headers_t& hubmessage::headers()
{
	return data_.headers;
}

hubmessage::packet::headers_t const& hubmessage::headers() const
{
	return data_.headers;
}

char* hubmessage::body()
{
	return payload() + topic_length();
}

hubmessage::action hubmessage::get_action() const
{
	return headers().msgaction;
}

void hubmessage::set_action(action action)
{
	headers().msgaction = action;
}

void hubmessage::set_message(const std::string& subj)
{
	memcpy(payload(), subj.c_str(), subj.length());
	headers().topiclen = subj.length();
	headers().bodylen = 0;
	headers().magic = cookie;
}

void hubmessage::set_message(const std::string& subj, const std::vector<char>& msg)
{
	set_message(subj);
	headers().bodylen = msg.size();
	if (header_length() + msg.size() > messagesize)
	{
		throw std::out_of_range("Povided message is too big");
	}

	memcpy(payload() + headers().topiclen, msg.data(), msg.size());
}

size_t hubmessage::body_length() const
{
	return headers().bodylen;
}

bool hubmessage::verify() const
{
	if (headers().magic != cookie)
		return false;

	return true;
}
