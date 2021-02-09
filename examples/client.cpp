#include <msghub.h>
#include <boost/date_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/container/flat_map.hpp>
#include <boost/container/static_vector.hpp>
#include <iostream>
#include <system_error>
#include <thread>
using namespace std::chrono_literals;
using boost::filesystem::last_write_time;
using boost::posix_time::from_time_t;

namespace mh = msghublib;
int main(int /*argc*/, char const** argv) {
    static auto const bin_date = to_simple_string(from_time_t(last_write_time(argv[0])));

    boost::asio::thread_pool io(10);
    mh::msghub hub(io.get_executor());

    static std::atomic_int batches = 0;
    try {
        hub.connect("localhost", 1334);

        hub.publish("Publish", bin_date);
        for (auto i = 0; i < 1'000; ++i) {
            ++batches;
            post(io, [&hub, n = i * 100] {
                mh::error_code ec;
                for (int i = n; i < n + 100; ++i) {
                    hub.publish("Publish", "message " + std::to_string(i), ec);
                    if (ec) {
                        std::cerr << "Couldn't publish message " << i << " (" << ec.message() << ")\n";
                    }
                }
                --batches;
            });
        }
    } catch (mh::system_error const& se) {
        std::cerr << "Error " << se.code().message() << "\n";
    }

    while (batches > 0) { std::this_thread::sleep_for(10ms); }
    hub.stop();
    io.join();
}
