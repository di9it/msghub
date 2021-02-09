#pragma once
#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>
#include <type_traits>

namespace msghublib {
    using boost::system::error_code;
    using boost::system::error_category;
    using boost::system::system_error;

    enum hub_errc {
        hub_connection_failed = 1,
        hub_creation_failed = 2,
        hub_not_connected = 3,
    };
}

template <> struct boost::system::is_error_code_enum<msghublib::hub_errc>
        : std::true_type {};

namespace msghublib {
    struct msghub_category : error_category {
        virtual char const* name() const noexcept override { return "msghub"; }
        virtual std::string message(int ev) const override {
            switch (ev) {
                case hub_errc::hub_connection_failed: return "hub_connection_failed";
                case hub_errc::hub_creation_failed:   return "hub_creation_failed";
                case hub_errc::hub_not_connected:     return "hub_not_connected";
            }
            return "unknown";
        }
    };

    static inline error_category const& msghub_category() {
        static constexpr struct msghub_category s_msghub_category_instance;
        return s_msghub_category_instance;
    };

    [[maybe_unused]] static inline error_code make_error_code(hub_errc e) {
        return {e, msghub_category()};
    }
}

