#pragma once

#include <iterator>
#include <version>
#if __cpp_lib_span // c++20 library feature
    #include <span>
    namespace msghublib {
        using std::span;
    }
#else
    #include <string_view>
    #include <string>
    namespace msghublib {
        namespace detail {
            template <typename T> struct span {
                T* _data;
                size_t _size;

                constexpr span(T* data = nullptr, size_t size = 0ull) : _data(data), _size(size) {}

                template <typename C>
                constexpr span(std::basic_string_view<C> const& sv)
                        : span(sv.data(), sv.size()) {}
                template <typename... C>
                constexpr span(std::basic_string<C...> const& s)
                        : span(s.data(), s.size()) {}
                template <size_t N>
                constexpr span(T (&arr)[N]) : span(arr, N) {}
                template <size_t N>
                constexpr span(std::array<T, N>& arr)
                        : span(arr.data(), arr.size()) {}

                template <typename It>
                    constexpr span(It first, size_t n) : span(std::addressof(*first), n) {}

                constexpr auto size()  const { return _size; }
                constexpr auto data()  const { return _data; }

                constexpr auto begin() const { return data(); }
                constexpr auto end()   const { return data() + size(); }

                constexpr auto rbegin() const { return std::make_reverse_iterator(end()); }
                constexpr auto rend() const { return std::make_reverse_iterator(begin()); }

                constexpr auto& front() const { return *(begin()); }
                constexpr auto& back()  const { return *(end() - 1); }
            };
        }
        using detail::span;
    }
#endif

#include <boost/asio/buffer.hpp>
namespace boost::asio {
    template <typename T>
    auto buffer(::msghublib::span<T> span) {
        return buffer(span.data(), span.size() * sizeof(T));
    }
}
