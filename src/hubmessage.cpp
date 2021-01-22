#include "hubmessage.h"
#include <charbuf.h>
#include <string_view>
#include <variant>

hubmessage::hubmessage()
{}

char const* hubmessage::data() const
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

charbuf hubmessage::payload()
{
	return {headers().payload, payload_length()};
}

std::string_view hubmessage::topic() const
{
	return {headers().payload, topic_length()};
}

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

const_charbuf hubmessage::body() const
{
	return {headers().payload + topic_length(), body_length()};
}

charbuf hubmessage::body()
{
	return {headers().payload + topic_length(), body_length()};
}

hubmessage::action hubmessage::get_action() const
{
	return headers().msgaction;
}

void hubmessage::set_action(action action)
{
	headers().msgaction = action;
}

void hubmessage::set_message(std::string_view topic)
{
	memcpy(payload().data(), topic.data(), topic.length());
	headers().topiclen = topic.length();
	headers().bodylen = 0;
	headers().magic = cookie;
}

void hubmessage::set_message(std::string_view topic, const_charbuf msg)
{
	set_message(topic);
	headers().bodylen = msg.size();
	if (header_length() + msg.size() + topic_length() > messagesize) {
		throw std::out_of_range("Povided message is too big");
	}

	memcpy(payload().data() + topic_length(), msg.data(), msg.size());
}

size_t hubmessage::body_length() const
{
	return headers().bodylen;
}

bool hubmessage::verify() const
{
    return headers().magic == cookie;
}
