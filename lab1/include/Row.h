#ifndef ROW_H
#define ROW_H

#include "Constants.h"
#include <array>
#include <cstdint>

/**
 * Struct representing a database row containing primitive data types ONLY
 * to enable serialization. The fixed sizes are defined in Constants.h.
 */
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