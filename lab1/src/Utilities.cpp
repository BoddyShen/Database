#include "Utilities.h"
#include <algorithm>
#include <cstring>
#include <iostream>
#include <string>

using namespace std;

// Helper function to convert a Row to a formatted string.
string Utilities::rowToString(const Row &row)
{
    // Convert movieId to string.
    string movieIdStr(reinterpret_cast<const char *>(row.movieId.data()), row.movieId.size());
    // Remove any trailing null characters.
    movieIdStr.erase(find(movieIdStr.begin(), movieIdStr.end(), '\0'), movieIdStr.end());

    // Convert title to string.
    string titleStr(reinterpret_cast<const char *>(row.title.data()), row.title.size());
    titleStr.erase(find(titleStr.begin(), titleStr.end(), '\0'), titleStr.end());

    return "Movie ID: " + movieIdStr + ", Title: " + titleStr;
}


int Utilities::findEmptyFrame(pageTable) {
    // find an empty frame and if frames are full then evict a page
    for (int i = 0; i < pageTable.size(); i++) {
        if (pageTable == -1) {
            return i;
        }
    }

    // evict a page using LRU

    return -1;
}