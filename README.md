MSGHUB
======
This is a lightweight asynchronous message bus C++ library based on [Boost.Asio](http://www.boost.org/doc/libs/1_56_0/doc/html/boost_asio.html). It allows to connect, send and receive custom messages.

The idea is simple... there are topics, subscribers and publishers.

One client create a **msghub** instance and others are connecting to it.

Subscriber specify the topic name of its interest.
All clients publish message into a topic and hub distribute messages across all subscribers.

Message is a byte array or a string.

Array may contain any serialized user data, but it's out of scope here.

Examples
--------

I. Create hub, subscribe on "any topic" and publish "new message" into "any topic":
```c++
	// Message handler
	void on_message(const std::string& topic, std::vector<char>& message)
	{
	   // handle message
	}

	int main()
	{
		boost::asio::io_service io_service;
		// Create hub to listen on 0xbee port
		msghub msghub(io_service);
		msghub.create(0xbee);
		// Subscribe on "any topic"
		msghub.subscribe("any topic", on_message);
		// Current or any another client
		msghub.publish("any topic", "new message");
		io_service.run();
	}
```
II. Connect to hub on "localhost" and publish "new message" into "any topic":
```c++
	int main()
	{
		boost::asio::io_service io_service;
		msghub msghub(io_service);
		msghub.connect("localhost", 0xbee);
		msghub.publish("any topic", "new message");
		io_service.run();
	}
```
