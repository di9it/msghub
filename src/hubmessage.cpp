#include "hubmessage.h"
#include <charbuf.h>
#include <string_view>

hubmessage::hubmessage(action action_, std::string_view topic, const_charbuf msg)
    : headers_ {}, payload_{}
{
	if (topic.size() + msg.size() > payload_.max_size()) {
		throw std::length_error("messagesize");
	}

	headers_.topiclen  = topic.length();
	headers_.bodylen   = msg.size();
	headers_.msgaction = action_;
	headers_.magic     = cookie;

    auto out = payload_.data();
    out = std::copy_n(topic.data(), topic.size(), out);
    out = std::copy_n(msg.data(),   msg.size(),   out);
}

bool hubmessage::verify() const
{
    return headers_.magic == cookie;
}

hubmessage::action hubmessage::get_action() const {
    return headers_.msgaction;
}

std::string_view hubmessage::topic() const {
    return
        std::string_view(payload_.data(), payload_.size())
        .substr(0, headers_.topiclen);
}

const_charbuf hubmessage::body() const {
    return
        std::string_view(payload_.data(), payload_.size())
        .substr(headers_.topiclen, headers_.bodylen);
}
