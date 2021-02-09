#include <boost/asio.hpp>
#include <boost/asio/signal_set.hpp>
#include <msghub.h>
#include <iostream>
#include <iomanip>
#include <atomic>
#include <boost/histogram.hpp>
#include <boost/spirit/home/x3.hpp>
#include <mutex>
#include <thread>
namespace mh = msghublib;
namespace bh = boost::histogram;
namespace x3 = boost::spirit::x3;

namespace {
    constexpr auto io_threads = 10;
    std::mutex mx;
    auto h = bh::make_profile(bh::axis::integer<>(0, io_threads) );
} // namespace

static void on_message(std::string_view topic, mh::span<char const> message)
{
    size_t value = 0;
    x3::parse(message.begin(), message.end(), x3::seek[x3::uint_], value);

    static std::atomic_int counter(0);
    if (0 == (counter++ % 1'000)) {
        std::string_view sm(message.data(), message.size());
        std::cout << (counter-1) << " callback: " << topic << " " << sm << "\n";
    }

    {
        static std::atomic_int tid_gen(0);
        thread_local const int tid = tid_gen++;

        std::lock_guard lk(mx);
        h(tid, bh::sample(value));
    }

    //if (counter>9'000) throw "TOO DAMN HIGH";
}

int main()
{
    try {
        boost::asio::thread_pool io(io_threads);
        // Create hub to listen on 0xbee port
        mh::msghub hub(io.get_executor());
        hub.create(1334);
        // Subscribe on "any topic"
        hub.subscribe("Publish", on_message);
        // Current or any another client
        //hub.publish("Publish", "hello");

        boost::asio::signal_set ss(io, SIGINT);
        ss.async_wait([&](auto ec, int sig) {
            if (!ec) {
                std::cerr << "Shutdown requested (" << ::strsignal(sig) << ", " << ec.message() << ")\n";
                hub.stop();
            }
        });

        io.join(); // blocks
    } catch(std::exception const& e) {
        std::cerr << "Exception " << e.what() << "\n";
    } catch(...) {
        std::cerr << "Exception\n";
    }

    for (auto&& x : indexed(h)) {
        if (x->count() != 0.0) { 
            std::cout
                << "thread " << x.index()
                << "\tcount " << x->count()
                << "\tmean " << x->value()
                << "\tstddev " << std::sqrt(x->variance())
                << "\n";
        }
    }
}
