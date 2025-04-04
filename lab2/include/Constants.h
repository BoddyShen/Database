#ifndef CONSTANTS_H
#define CONSTANTS_H
#include <cstddef>
#include <string>

constexpr size_t MOVIE_ID_SIZE = 9;
constexpr size_t TITLE_SIZE = 30;
constexpr size_t RECORD_SIZE = MOVIE_ID_SIZE + TITLE_SIZE;
constexpr size_t MAX_PAGE_SIZE = 4096;
constexpr size_t FRAME_SIZE = 24;
constexpr size_t BUFFER_SIZE = MAX_PAGE_SIZE * 24;

const std::string MOVIE_DB_FILE = "movie.bin";
const std::string TEST_DB_FILE = "test_movie.bin";

#endif // CONSTANTS_H