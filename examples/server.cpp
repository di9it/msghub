#include <boost/asio.hpp>
#include <msghub.h>
#include <iostream>
#include <iomanip>

void on_message(const std::string& topic, std::vector<char> const& message)
{
    std::cout << "on_message: " << std::quoted(topic) << " " <<
        std::quoted(std::string(message.begin(), message.end())) << std::endl;
}

int main()
{
    boost::asio::io_service io_service;
    boost::asio::io_service::work  work(io_service);
    // Create hub to listen on 0xbee port
    msghub msghub(io_service);
    if (!msghub.create(1334))
        std::cerr << "Error creating hub server\n";
    // Subscribe on "any topic"
    if (!msghub.subscribe("Publish", on_message))
        std::cerr << "Error creating subscription\n";

    // Current or any another client
    //msghub.publish("Publish", "hello");

    io_service.run(); // blocks for-ever
}
