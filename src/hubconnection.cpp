#include "hubconnection.h"
#include <boost/system/detail/error_code.hpp>
#include <hub_error.h>

namespace msghublib::detail {

    auto hubconnection::bind(void (hubconnection::*handler)(error_code)) {
#pragma GCC diagnostic ignored "-Wdeprecated" // implicit this-capture
        return [=, self=shared_from_this()](error_code ec, size_t /*transferred*/) {
            (this->*handler)(ec);
        };
    }

    void hubconnection::init(const std::string& host, uint16_t port, error_code& ec) {
        try {
            error_code ec;
            tcp::resolver resolver(socket_.get_executor());
            tcp::resolver::results_type results =
                resolver.resolve(host, std::to_string(port), ec);

            // Do blocking connect (connection is more important than
            // subscription here)
            if (!ec)
                connect(socket_, results, ec);

            // Schedule packet read
            if (!ec)
                async_read(socket_, inmsg_.header_buf(),
                           bind(&hubconnection::handle_read_header));
        } catch (system_error const& se) {
            ec = se.code();
        } catch (...) {
            ec = hub_errc::hub_connection_failed;
        }
    }

    void hubconnection::async_send(const hubmessage& msg) {
#pragma GCC diagnostic ignored "-Wdeprecated" // implicit this-capture
        post(socket_.get_executor(), [=, self = shared_from_this()]() mutable {
            do_send(std::move(msg));
        });
    }

    void hubconnection::send(const hubmessage& msg, error_code& ec) {
        write(socket_, msg.on_the_wire(), ec);
    }

    void hubconnection::close(bool forced) {
#pragma GCC diagnostic ignored "-Wdeprecated" // implicit this-capture
        post(socket_.get_executor(),
             [=, self = shared_from_this()] { do_close(forced); });
    }

    void hubconnection::handle_read_header(error_code error) {
        if (!error && inmsg_.verify()) {
            async_read(socket_, inmsg_.payload_area(),
                       bind(&hubconnection::handle_read_body));
        } else {
            do_close(true);
        }
    }

    void hubconnection::handle_read_body(error_code error) {
        if (!error) {
            courier_.deliver(inmsg_);
            async_read(socket_, inmsg_.header_buf(),
                       bind(&hubconnection::handle_read_header));
        } else {
            do_close(true);
        }
    }

    void hubconnection::do_send(hubmessage msg) {
        if (outmsg_queue_.push_back(std::move(msg));
            1 == outmsg_queue_.size())
        {
            async_write(socket_, outmsg_queue_.front().on_the_wire(),
                        bind(&hubconnection::handle_write));
        }
    }

    void hubconnection::handle_write(error_code error)
    {
        if (!error)
        {
            if (outmsg_queue_.pop_front(); !outmsg_queue_.empty())
            {
                async_write(socket_,
                    outmsg_queue_.front().on_the_wire(),
                    bind(&hubconnection::handle_write));
            } else if (is_closing) {
                do_close(false);
            }
        }
        else
        {
            do_close(true);
        }
    }

    void hubconnection::do_close(bool forced)
    {
        is_closing = true; // atomic

        // TODO(sehe): Unsubscribe?

        if (forced || outmsg_queue_.empty()) {
            if (socket_.is_open()) {
                error_code ec;
                socket_.close(ec);
            }
        }
    }

} // namespace msghublib::detail
