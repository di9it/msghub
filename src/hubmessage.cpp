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
	return offsetof(packet, payload);
}

size_t hubmessage::length() const
{
	return header_length() + payload_length();
}

char* hubmessage::payload()
{
	return data_.payload;
}

char* hubmessage::topic()
{
	return payload();
}

//char* hubmessage::payload() const
//{
//	return data_.payload;
//}

size_t hubmessage::payload_length() const
{
	return data_.topiclen + data_.bodylen;
}

size_t hubmessage::topic_length() const
{
	return data_.topiclen;
}

char* hubmessage::body()
{
	return payload() + topic_length();
}

hubmessage::action hubmessage::get_action() const
{
	return data_.msgaction;
}

void hubmessage::set_action(action action)
{
	data_.msgaction = action;
}

void hubmessage::set_message(const std::string& subj)
{
	memcpy(payload(), subj.c_str(), subj.length());
	data_.topiclen = subj.length();
	data_.bodylen = 0;
	data_.magic = cookie;
}

void hubmessage::set_message(const std::string& subj, const std::vector<char>& msg)
{
	set_message(subj);
	data_.bodylen = msg.size();
	if (header_length() + msg.size() > messagesize)
	{
		throw std::out_of_range("Povided message is too big");
	}

	memcpy(payload() + data_.topiclen, msg.data(), msg.size());
}

size_t hubmessage::body_length() const
{
	return data_.bodylen;
}

bool hubmessage::verify() const
{
	if (data_.magic != cookie)
		return false;

	return true;
}
