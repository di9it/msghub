#pragma once

#include <iterator>

#include <version>
#if __cpp_lib_span // c++20 library feature
    #include <span>
    using std::span;
#else
    namespace msghub_detail {
        template <typename T> struct span {
            T* _data;
            size_t _size;

            constexpr span(T* data, size_t size) : _data(data), _size(size) {}

            template <size_t N>
                constexpr span(T (&arr)[N]) : span(arr, N) {}
            template <size_t N>
                constexpr span(std::array<T, N>& arr) : span(arr.data(), arr.size()) {}

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
    using msghub_detail::span;
#endif

#include <boost/asio/buffer.hpp>
namespace boost::asio {
    template <typename T>
    auto buffer(span<T> span) {
        return buffer(span.data(), span.size() * sizeof(T));
    }
}
