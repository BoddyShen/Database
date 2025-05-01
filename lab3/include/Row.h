#ifndef ROW_H
#define ROW_H

#include "Constants.h"
#include "FixedSizeString.h"
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <iostream>

using FixedTitleSizeString = FixedSizeString<TITLE_SIZE>;
using FixedMovieIdString = FixedSizeString<MOVIE_ID_SIZE>;

using FixedPersonIdString = FixedSizeString<PERSON_ID_SIZE>;
using FixedCategorySizeString = FixedSizeString<CATEGORY_SIZE>;
using FixedNameSizeString = FixedSizeString<NAME_SIZE>;

// Extract fields from RowType to Tuple
struct Tuple {
    std::vector<std::string> fields;
};

// Movie Row
struct MovieRow {
    std::array<uint8_t, MOVIE_ID_SIZE> movieId;
    std::array<uint8_t, TITLE_SIZE> title;

    MovieRow() = default;

    // new constructor taking string_views:
    MovieRow(const std::string &movieIdStr, const std::string &titleStr)
    {
        // zero‐fill
        movieId.fill(0);
        title.fill(0);

        // copy up to MOVIE_ID_SIZE
        auto mlen = std::min(movieIdStr.size(), size_t(MOVIE_ID_SIZE));
        std::copy_n(movieIdStr.data(), mlen, movieId.begin());

        // copy up to TITLE_SIZE
        auto tlen = std::min(titleStr.size(), size_t(TITLE_SIZE));
        std::copy_n(titleStr.data(), tlen, title.begin());
    }

    explicit MovieRow(Tuple const &t, std::unordered_map<std::string, int> const &idx)
        : MovieRow(t.fields.at(idx.at("movieId")), t.fields.at(idx.at("title")))
    {
    }

    Tuple toTuple() const
    {
        std::string m(reinterpret_cast<const char *>(movieId.data()),
                      strnlen(reinterpret_cast<const char *>(movieId.data()), MOVIE_ID_SIZE));
        std::string t(reinterpret_cast<const char *>(title.data()),
                      strnlen(reinterpret_cast<const char *>(title.data()), TITLE_SIZE));

        return {{m, t}};
    }
};

// Used for previous code
using Row = MovieRow;

// WorkedOn Row
struct WorkedOnRow {
    std::array<uint8_t, MOVIE_ID_SIZE> movieId;
    std::array<uint8_t, PERSON_ID_SIZE> personId;
    std::array<uint8_t, CATEGORY_SIZE> category;

    WorkedOnRow() = default;

    // new constructor taking string_views:
    WorkedOnRow(const std::string &movieIdStr, const std::string &personIdStr,
                const std::string &categoryStr)
    {
        // zero‐fill
        movieId.fill(0);
        personId.fill(0);
        category.fill(0);

        // copy up to MOVIE_ID_SIZE
        auto mlen = std::min(movieIdStr.size(), size_t(MOVIE_ID_SIZE));
        std::copy_n(movieIdStr.data(), mlen, movieId.begin());

        // copy up to PERSON_ID_SIZE
        auto plen = std::min(personIdStr.size(), size_t(PERSON_ID_SIZE));
        std::copy_n(personIdStr.data(), plen, personId.begin());

        // copy up to CATEGORY_SIZE
        auto clen = std::min(categoryStr.size(), size_t(CATEGORY_SIZE));
        std::copy_n(categoryStr.data(), clen, category.begin());
    }

    Tuple toTuple() const
    {
        std::string m(reinterpret_cast<const char *>(movieId.data()),
                      strnlen(reinterpret_cast<const char *>(movieId.data()), MOVIE_ID_SIZE));
        std::string p(reinterpret_cast<const char *>(personId.data()),
                      strnlen(reinterpret_cast<const char *>(personId.data()), PERSON_ID_SIZE));
        std::string c(reinterpret_cast<const char *>(category.data()),
                      strnlen(reinterpret_cast<const char *>(category.data()), CATEGORY_SIZE));

        return {{m, p, c}};
    }
};

// Person Row
struct PersonRow {
    std::array<uint8_t, PERSON_ID_SIZE> personId;
    std::array<uint8_t, NAME_SIZE> name;

    PersonRow() = default;

    PersonRow(const std::string &personIdStr, const std::string &nameStr)
    {
        personId.fill(0);
        name.fill(0);

        auto plen = std::min(personIdStr.size(), size_t(PERSON_ID_SIZE));
        std::copy_n(personIdStr.data(), plen, personId.begin());

        auto nlen = std::min(nameStr.size(), size_t(NAME_SIZE));
        std::copy_n(nameStr.data(), nlen, name.begin());
    }

    Tuple toTuple() const
    {
        std::string p(reinterpret_cast<const char *>(personId.data()),
                      strnlen(reinterpret_cast<const char *>(personId.data()), PERSON_ID_SIZE));
        std::string n(reinterpret_cast<const char *>(name.data()),
                      strnlen(reinterpret_cast<const char *>(name.data()), NAME_SIZE));
        return {{p, n}};
    }
};

// movieId + personId for Materialized
struct WorkedOnKeyRow {
    FixedMovieIdString movieId;
    FixedPersonIdString personId;

    WorkedOnKeyRow() = default;
    WorkedOnKeyRow(std::string const &m, std::string const &p) : movieId(m), personId(p) {}

    // idx_map indicate which Tuple field to pull for each member.
    WorkedOnKeyRow(Tuple const &t, std::unordered_map<std::string, int> const &idx_map)
    {
        movieId = t.fields.at(idx_map.at("movieId"));
        personId = t.fields.at(idx_map.at("personId"));
    }

    Tuple toTuple() const { return {{movieId.toString(), personId.toString()}}; }
};

struct MovieWorkedOnRow {
    FixedMovieIdString movieId;
    FixedTitleSizeString title;
    FixedPersonIdString personId;

    MovieWorkedOnRow() = default;

    explicit MovieWorkedOnRow(Tuple const &t, std::unordered_map<std::string, int> const &idx)
    {
        movieId = t.fields.at(idx.at("movieId"));
        title = t.fields.at(idx.at("title"));
        personId = t.fields.at(idx.at("personId"));
    }

    // Turn it back into a Tuple if you need to feed it downstream
    Tuple toTuple() const { return {{movieId.toString(), title.toString(), personId.toString()}}; }
};

#endif // ROW_H