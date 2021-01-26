MSGHUB
======
This is a lightweight asynchronous message bus C++ library based on [Boost.Asio](http://www.boost.org/doc/libs/1_56_0/doc/html/boost_asio.html). It allows to connect, send and receive custom messages to virtually any number of local or network clients.

The idea is simple... there are topics, subscribers and publishers.

One client create a **msghub** instance and others are connecting to it.

Subscriber specify the topic name of its interest.
All clients publish message into a topic and hub distribute messages across all subscribers.

Message is a byte array or a string.

Array may contain any serialized user data, but it's out of scope here.

Examples
--------

 1. Create hub, subscribe on "any topic" and publish "new message" into "any topic":

    ```c++
    namespace mh = msghublib;

    // Message handler
    void on_message(std::string_view topic, mh::span<char const> message)
    {
        // handle message
    }

    int main()
    {
        boost::asio::io_context io;
        // Create hub to listen on 0xbee port
        mh::msghub hub(io.get_executor());
        hub.create(0xbee);

        // Subscribe on "any topic"
        hub.subscribe("any topic", on_message);

        // Current or any another client
        hub.publish("any topic", "new message");

        io.run(); // keep server active, if created
    }
    ```
 2. Connect to hub on "localhost" and publish "new message" into "any topic":

    ```c++
    namespace mh = msghublib;

    int main()
    {
        boost::asio::io_context io;
        mh::msghub hub(io.get_executor());
        hub.connect("localhost", 0xbee);
        hub.publish("any topic", "new message");

        hub.stop();
        io.run();
    }
    ```

 3. Using multiple threads

    ```c++
    namespace mh = msghublib;

    int main()
    {
        boost::asio::thread_pool io(5); // count optional
        mh::msghub hub(io.get_executor());
        hub.connect("localhost", 0xbee);
        hub.publish("any topic", "new message");

        hub.stop();
        io.join();
    }
    ```
Note: `span<char const>` is `std::span<char const>` on c++20 capable compilers.
