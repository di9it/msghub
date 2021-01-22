#ifndef _MSGHUB_MSGHUB_H_
#define _MSGHUB_MSGHUB_H_

#include <string>
#include <vector>
#include <cstdint>

#include <memory>
#include <boost/asio.hpp>
#include "charbuf.h"

class msghub_impl;
class msghub
{
  public:
    typedef std::function< void(std::string_view topic, const_charbuf message) > onmessage;
    
  public:
    explicit msghub(boost::asio::any_io_executor);
    ~msghub();

    bool connect(const std::string& hostip, uint16_t port);
    bool create(uint16_t port);

    bool unsubscribe(const std::string& topic);
    bool subscribe(const std::string& topic, onmessage handler);
    bool publish(std::string_view topic, const_charbuf message);

    // Treat string literals specially, not including the terminating NUL
    template <size_t N>
	bool publish(std::string_view topic, char const (&literal)[N]) {
        static_assert(N>0);
        return publish(topic, const_charbuf{literal, N-1});
    }

    void stop();
    
  private:
    std::shared_ptr<msghub_impl> pimpl;
};

#endif
