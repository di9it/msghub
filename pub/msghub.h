#ifndef _MSGHUB_MSGHUB_H_
#define _MSGHUB_MSGHUB_H_

#include <string>
#include <vector>
#include <cstdint>

#include <memory>
#include <boost/asio.hpp>

class msghub_impl;
class msghub
{
  public:
    typedef std::function< void(const std::string& topic, std::vector<char> const& message) > onmessage;
    
  public:
    explicit msghub(boost::asio::any_io_executor);
    ~msghub();

    bool connect(const std::string& hostip, uint16_t port);
    bool create(uint16_t port);

    bool unsubscribe(const std::string& topic);
    bool subscribe(const std::string& topic, onmessage handler);
    bool publish(const std::string& topic, const std::vector<char>& message);
	bool publish(const std::string& topic, const std::string& message);

    void stop();
    
  private:
    std::unique_ptr<msghub_impl> pimpl;
};

#endif
