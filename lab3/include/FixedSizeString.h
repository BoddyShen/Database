#ifndef FIXED_SIZE_STRING_H
#define FIXED_SIZE_STRING_H

#include <algorithm>
#include <array>
#include <cassert>
#include <cstring>
#include <string>

template <std::size_t N> struct FixedSizeString {
    std::array<char, N> data;

    // 1) default: all ’\0’
    FixedSizeString() { data.fill('\0'); }

    // 2) construct from std::string (truncate or pad)
    FixedSizeString(const std::string &s)
    {
        data.fill('\0');
        auto len = std::min(s.size(), N);
        std::copy_n(s.data(), len, data.data());
    }

    // convert back to std::string
    std::string toString() const
    {
        // strnlen will stop at N or the first '\0'
        return std::string(data.data(), strnlen(data.data(), N));
    }

    // lexicographic compare via memcmp (zero‐padded)
    bool operator<(const FixedSizeString &o) const
    {
        return std::memcmp(data.data(), o.data.data(), N) < 0;
    }
    bool operator==(const FixedSizeString &o) const
    {
        return std::memcmp(data.data(), o.data.data(), N) == 0;
    }
    bool operator!=(const FixedSizeString &o) const { return !(*this == o); }
    bool operator<=(const FixedSizeString &o) const { return (*this < o) || (*this == o); }
    bool operator>(const FixedSizeString &o) const { return o < *this; }
    bool operator>=(const FixedSizeString &o) const { return (*this > o) || (*this == o); }
};

#endif // FIXED_SIZE_STRING_H
