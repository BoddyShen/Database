#include "Commands.h"
#include "ProjectOperator.h"
#include "ScanOperator.h"
#include "SelectOperator.h"
#include <BufferManager.h>
#include <Row.h>
#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>

using namespace std;

// int loadMovieData(const string &tsvFileName, const string &dbFileName)
// {
//     // Remove existing database file to start with a clean slate.
//     remove(dbFileName.c_str());
//     BufferManager bm(FRAME_SIZE);
//     bm.registerFile(dbFileName);

//     // Open the TSV file
//     ifstream tsvFile("../" + tsvFileName);
//     if (!tsvFile.is_open()) {
//         cerr << "Failed to open " + tsvFileName << endl;
//         return 0;
//     }

//     string header;
//     getline(tsvFile, header);
//     cout << "Header: " << header << endl;

//     // Create the first append page; all data will be inserted into this page.
//     Page<MovieRow> *appendPage = bm.createPage<MovieRow>(dbFileName);
//     int appendPid = appendPage->getPid();
//     cout << "Initial append page id: " << appendPid << endl;

//     // Read the TSV file line by line.
//     string line;
//     int loadedRows = 0;
//     while (getline(tsvFile, line)) {
//         loadedRows++;
//         // Parse the line using tab as a delimiter.
//         istringstream iss(line);
//         vector<string> tokens;
//         string token;
//         while (getline(iss, token, '\t')) {
//             tokens.push_back(token);
//         }

//         if (tokens.size() < 3) continue;
//         string movieId = tokens[0];
//         string title = tokens[2];

//         // Truncate data to fixed length: movieId to 9 characters, title to 30 characters.
//         if (movieId.size() > MOVIE_ID_SIZE) movieId = movieId.substr(0, MOVIE_ID_SIZE);
//         if (title.size() > TITLE_SIZE) title = title.substr(0, TITLE_SIZE);

//         // Create a Row object, assuming Row has a constructor accepting C-string.
//         MovieRow row(movieId.c_str(), title.c_str());

//         // If the current page is full, unpin it and create a new page.
//         if (appendPage->isFull()) {
//             bm.unpinPage(appendPid, dbFileName);
//             appendPage = bm.createPage<MovieRow>(dbFileName);
//             appendPid = appendPage->getPid();
//             cout << "Loaded " << loadedRows << " rows" << endl;
//             cout << "Created new append page, id: " << appendPid << endl;
//         }

//         // Insert the row into the current append page.
//         int rowId = appendPage->insertRow(row);
//         if (rowId == -1) {
//             cerr << "Failed to insert row into page " << appendPid << endl;
//         }
//     }

//     tsvFile.close();

//     // After loading, unpin the last append page.
//     bm.unpinPage(appendPid, dbFileName);

//     cout << "Loaded " << loadedRows << " rows into the Movies table." << endl;
//     return appendPid;
// }

// -----------------------------------------------------------------------------
// loadData
//
//  • RowType        — the fixed‐size row struct (MovieRow, WorkedOnRow, …).
//  • makeRow        — a callable that accepts vector<string> tokens and
//                     returns a RowType, truncating/padding as needed.
// -----------------------------------------------------------------------------
template <typename RowType>
int loadData(const std::string &tsvFileName, const std::string &dbFileName,
             std::function<RowType(const std::vector<std::string> &)> makeRow)
{
    // 1) clean slate
    std::remove(dbFileName.c_str());
    BufferManager bm(FRAME_SIZE);
    bm.registerFile(dbFileName);

    // 2) open TSV
    std::ifstream tsv("../" + tsvFileName);
    if (!tsv.is_open()) {
        std::cerr << "Failed to open " << tsvFileName << "\n";
        return -1;
    }
    std::string header;
    std::getline(tsv, header);
    std::cout << "Header: " << header << "\n";

    // 3) first append page
    auto *page = bm.createPage<RowType>(dbFileName);
    int pid = page->getPid();
    std::cout << "Initial append page id: " << pid << "\n";

    // 4) read & insert
    std::string line;
    int count = 0;
    while (std::getline(tsv, line)) {
        ++count;
        // split tabs
        std::vector<std::string> tokens;
        std::istringstream iss(line);
        for (std::string f; std::getline(iss, f, '\t');) tokens.push_back(std::move(f));

        // build your RowType however you please
        RowType row = makeRow(tokens);

        // rotate pages if full
        if (page->isFull()) {
            bm.unpinPage(pid, dbFileName);
            page = bm.createPage<RowType>(dbFileName);
            pid = page->getPid();
            std::cout << "Created new append page, id: " << pid << " after " << count << " rows\n";
        }
        if (page->insertRow(row) < 0) std::cerr << "  ❌ failed to insert row " << count << "\n";
    }

    // 5) final unpin
    bm.unpinPage(pid, dbFileName);
    std::cout << "Loaded " << count << " rows into " << dbFileName << "\n";
    return pid;
}

void pre_process(bool test)
{
    cout << "Start Pre-processing." << endl;
    string movieFile;
    string workedonFile;
    string peopleFile;
    string movieTsvFile;
    string workedonTsvFile;
    string peopleTsvFile;

    if (test) {
        cout << "Test mode, using test files." << endl;
        movieFile = "movie100000.bin";
        workedonFile = "workedon100000.bin";
        peopleFile = "people100000.bin";
        movieTsvFile = "title.basics100000.tsv";
        workedonTsvFile = "title.principals100000.tsv";
        peopleTsvFile = "name.basics100000.tsv";
    } else {
        movieFile = "movie.bin";
        workedonFile = "workedon.bin";
        peopleFile = "people.bin";
        movieTsvFile = "title.basics.tsv";
        workedonTsvFile = "title.principals.tsv";
        peopleTsvFile = "name.basics.tsv";
    }

    // check if movie.bin exists, if not, load movie data
    ifstream f(movieFile, ios::binary);
    if (!f.good()) {
        cout << "movie.bin not found, start loading Movie data..." << endl;
        loadData<MovieRow>(movieTsvFile, movieFile, [](auto const &tok) {
            std::string m = tok[0], t = tok[2];
            if (m.size() > MOVIE_ID_SIZE) m.resize(MOVIE_ID_SIZE);
            if (t.size() > TITLE_SIZE) t.resize(TITLE_SIZE);
            return MovieRow{m, t};
        });
        cout << "Finished loading movie data." << endl;
    }
    f.close();

    // check if workedon.bin exists, if not, load workedon data
    ifstream f_w(workedonFile, ios::binary);
    if (!f_w.good()) {
        cout << "workedon.bin not found, start loading WorkedOn data..." << endl;
        loadData<WorkedOnRow>(workedonTsvFile, workedonFile, [](auto const &tok) {
            std::string m = tok[0], p = tok[2], c = tok[3];
            if (m.size() > MOVIE_ID_SIZE) m.resize(MOVIE_ID_SIZE);
            if (p.size() > PERSON_ID_SIZE) p.resize(PERSON_ID_SIZE);
            if (c.size() > CATEGORY_SIZE) c.resize(CATEGORY_SIZE);
            return WorkedOnRow{m, p, c};
        });
        cout << "Finished loading WorkedOn data." << endl;
    }
    f_w.close();

    // check if people.bin exists, if not, load people data
    ifstream f_p(peopleFile, ios::binary);
    if (!f_p.good()) {
        cout << "people.bin not found, start loading People data..." << endl;
        loadData<PersonRow>(peopleTsvFile, peopleFile, [](auto const &tok) {
            std::string p = tok[0], n = tok[1];
            if (p.size() > PERSON_ID_SIZE) p.resize(PERSON_ID_SIZE);
            if (n.size() > NAME_SIZE) n.resize(NAME_SIZE);
            return PersonRow{p, n};
        });
        cout << "Finished loading People data." << endl;
    }
    std::cout << "All tables loaded.\n";
}

void run_query(const std::string &start_range, const std::string &end_range, int buffer_size,
               bool test)
{
    BufferManager bm(buffer_size);

    string movieFile;
    string workedonFile;
    string peopleFile;

    if (test) {
        cout << "Test mode, using test files." << endl;
        movieFile = "movie100000.bin";
        workedonFile = "workedon100000.bin";
        peopleFile = "people100000.bin";
    } else {
        movieFile = "movie.bin";
        workedonFile = "workedon.bin";
        peopleFile = "people.bin";
    }

    bm.registerFile(movieFile);
    bm.registerFile(workedonFile);
    bm.registerFile(peopleFile);

    auto movieScan = new ScanOperator<MovieRow>(bm, movieFile);
    // title is the second column in movie row
    auto movieSelect = new SelectOperator(movieScan, [start_range, end_range](const Tuple &t) {
        return t.fields[1] >= start_range && t.fields[1] <= end_range;
    });
    // project (movieId, title)
    auto movieProject = new ProjectOperator(movieSelect, {0, 1});

    movieProject->open();
    Tuple movieOut;
    while (movieProject->next(movieOut)) {
        cout << "movie size = " << movieOut.fields.size() << endl;
        cout << "Movie: " << movieOut.fields[0] << ", " << movieOut.fields[1] << endl;
    }
    cout << "Finished scanning movies." << endl;
    movieProject->close();
}