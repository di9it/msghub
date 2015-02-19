# msghub
Lightweight publisher/subscriber C++ library based on boost asio

One instance could be used as server+client or client.

Server+Client:
msghub msghub(io_service);
msghub.create(0xBEE);

Client:
msghub msghub(io_service);
msghub.connect("localhost", 0xBEE);

Sending/receiving messages:
void on_message(const std::string& topic, std::vector<char>& message)
{
	// Message handling
}

msghub.subscribe("test_topic", on_message);
msghub.publish("test_topic", "test message here");