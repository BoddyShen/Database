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
};

#endif // ROW_H