#ifndef ROW_H
#define ROW_H

#include "Constants.h"
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <iostream>

/**
 * Struct representing a database row containing primitive data types ONLY
 * to enable serialization. The fixed sizes are defined in Constants.h.
 */

struct FixedTitleSizeString {
    std::array<char, TITLE_SIZE> data;

    // Default constructor fills with null characters.
    FixedTitleSizeString() { data.fill('\0'); }

    // Construct from std::string, truncating or padding as needed.
    FixedTitleSizeString(const std::string &str)
    {
        data.fill('\0');
        size_t len = std::min(str.size(), TITLE_SIZE);
        std::copy(str.begin(), str.begin() + len, data.begin());
    }

    // Convert to std::string (for printing or other operations)
    std::string toString() const
    {
        // Use strnlen to handle non-null-terminated arrays.
        return std::string(data.data(), strnlen(data.data(), TITLE_SIZE));
    }

    // Comparison operator for ordering in the BTree.
    bool operator<(const FixedTitleSizeString &other) const
    {
        // Compare fixed TITLE_SIZE bytes.
        return std::strncmp(data.data(), other.data.data(), TITLE_SIZE) < 0;
    }

    bool operator>(const FixedTitleSizeString &other) const
    {
        return std::strncmp(data.data(), other.data.data(), TITLE_SIZE) > 0;
    }

    bool operator==(const FixedTitleSizeString &other) const
    {
        return std::strncmp(data.data(), other.data.data(), TITLE_SIZE) == 0;
    }

    bool operator<=(const FixedTitleSizeString &other) const
    {
        return *this < other || *this == other;
    }

    bool operator>=(const FixedTitleSizeString &other) const
    {
        return *this > other || *this == other;
    }
};

struct FixedMovieIdString {
    std::array<char, MOVIE_ID_SIZE> data;

    // Default constructor fills with null characters.
    FixedMovieIdString() { data.fill('\0'); }

    // Construct from std::string, truncating or padding as needed.
    FixedMovieIdString(const std::string &str)
    {
        data.fill('\0');
        size_t len = std::min(str.size(), MOVIE_ID_SIZE);
        std::copy(str.begin(), str.begin() + len, data.begin());
    }

    // Convert to std::string (for printing or other operations)
    std::string toString() const
    {
        // Use strnlen to handle non-null-terminated arrays.
        return std::string(data.data(), strnlen(data.data(), MOVIE_ID_SIZE));
    }

    // Comparison operator for ordering in the BTree.
    bool operator<(const FixedMovieIdString &other) const
    {
        return std::strncmp(data.data(), other.data.data(), MOVIE_ID_SIZE) < 0;
    }

    bool operator>(const FixedMovieIdString &other) const
    {
        return std::strncmp(data.data(), other.data.data(), MOVIE_ID_SIZE) > 0;
    }

    bool operator==(const FixedMovieIdString &other) const
    {
        return std::strncmp(data.data(), other.data.data(), MOVIE_ID_SIZE) == 0;
    }

    bool operator<=(const FixedMovieIdString &other) const
    {
        return *this < other || *this == other;
    }

    bool operator>=(const FixedMovieIdString &other) const
    {
        return *this > other || *this == other;
    }
};

struct Row {
    std::array<uint8_t, MOVIE_ID_SIZE> movieId;
    std::array<uint8_t, TITLE_SIZE> title;

    Row() = default;

    Row(const std::array<uint8_t, MOVIE_ID_SIZE> &movieId,
        const std::array<uint8_t, TITLE_SIZE> &title)
        : movieId(movieId), title(title)
    {
    }

    // Additional constructor that accepts C-string literals.
    Row(const char *movieIdStr, const char *titleStr)
    {
        movieId.fill(0);
        title.fill(0);
        // Copy at most MOVIE_ID_SIZE bytes from movieIdStr.
        size_t lenMovie = std::min(std::strlen(movieIdStr), static_cast<size_t>(MOVIE_ID_SIZE));
        std::copy(movieIdStr, movieIdStr + lenMovie, movieId.begin());
        // Copy at most TITLE_SIZE bytes from titleStr.
        size_t lenTitle = std::min(std::strlen(titleStr), static_cast<size_t>(TITLE_SIZE));
        std::copy(titleStr, titleStr + lenTitle, title.begin());
    }
};

#endif // ROW_H