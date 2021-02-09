#include <msghub.h>
#include <iostream>

int main() {
    boost::asio::io_service io_service;
    msghub msghub(io_service);
    if (!msghub.connect("localhost", 1334)) {
        std::cerr << "Unable to connect to hub\n";
    } else {
        if (!msghub.publish("Publish", "new message")) {
            std::cerr << "Unable to post message on Publish channel\n";
        }
    }

    //msghub.join(); // implied in destructor
}
