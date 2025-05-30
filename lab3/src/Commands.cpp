#include "Commands.h"
#include "BNLJoinOp.h"
#include "ProjectOp.h"
#include "ScanOp.h"
#include "SelectOp.h"
#include "WorkedOnMaterialOp.h"
#include <BufferManager.h>
#include <Row.h>
#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>

using namespace std;

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
        movieTsvFile = "movie_clean100000.tsv";
        workedonTsvFile = "workedon_clean100000.tsv";
        peopleTsvFile = "people_clean100000.tsv";
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
               bool test, double *selectivity, int *join1Page, int *ioCount)
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

    auto movieScan = new ScanOp<MovieRow>(bm, movieFile);
    // Movies WHERE title BETWEEN start..end, title is the second column in movie row
    auto movieSelect = new SelectOp(movieScan, [start_range, end_range](const Tuple &t) {
        return t.fields[1] >= start_range && t.fields[1] <= end_range;
    });
    // project (movieId, title)
    auto movieProject = new ProjectOp(movieSelect, {0, 1});
    auto workedonScan = new ScanOp<WorkedOnRow>(bm, workedonFile);
    auto selectWorkedOn =
        new SelectOp(workedonScan, [](const Tuple &t) { return t.fields[2] == "director"; });

    // project (movieId, personId, category), pick movieId and personId
    auto workedonProject = new ProjectOp(selectWorkedOn, {0, 1});

    // Materialize the workedonProject into a file
    // By materializing it into its own file once, we pay the filter+projection cost just once, and
    // then the join operator can scan that materialized table as many times as it wants, without
    // re-invoking the original select.
    auto workedonMaterialOp =
        new WorkedOnMaterialOp(workedonProject, bm, "workedon_materialized.bin");

    // Join the two tables on movieId
    // Each join assign (buffer_size - 6) / 2 frames.
    // Since materialization will be done first, so we don't need to keep its frames.
    // 3 for each join (left, right, and output), so we need 6 frames in total.
    std::unordered_map<std::string, int> idx_map = {{"movieId", 0}, {"title", 1}};
    auto joinOp1 = new BNLJoinOp<std::string, MovieRow>(
        bm, movieProject, workedonMaterialOp, (buffer_size - 6) / 2, "-1.bin",
        [](const Tuple &t) { return t.fields[0]; }, [](const Tuple &t) { return t.fields[0]; },
        idx_map);

    auto peopleScan = new ScanOp<PersonRow>(bm, peopleFile);

    std::unordered_map<std::string, int> idx_map2 = {{"movieId", 0}, {"title", 1}, {"personId", 3}};
    auto joinOp2 = new BNLJoinOp<std::string, MovieWorkedOnRow>(
        bm, joinOp1, peopleScan, (buffer_size - 6) / 2, "-2.bin",
        [](const Tuple &t) { return t.fields[3]; }, [](const Tuple &t) { return t.fields[0]; },
        idx_map2);

    auto finalProject = new ProjectOp(joinOp2, {1, 4});
    // finalProject->open();
    // Tuple finalOut;
    // int finalCounter = 0;
    // while (finalProject->next(finalOut)) {
    //     finalCounter++;
    //     cout << "Final: " << finalOut.fields[0] << ", " << finalOut.fields[1] << endl;
    // }
    // cout << "Final count = " << finalCounter << endl;
    // finalProject->close();

    finalProject->open();
    Tuple joinOut;
    std::ofstream fout("cpp_join_out.tsv");
    fout << "title\tname\n";

    while (finalProject->next(joinOut)) {
        fout << joinOut.fields[0] << '\t' << joinOut.fields[1] << '\n';
    }
    finalProject->close();
    fout.close();

    std::cout << "Selectivity for movieId: " << movieSelect->getSelectivity() << "\n";
    std::cout << "Selectivity for workedOn: " << selectWorkedOn->getSelectivity() << "\n";
    std::cout << "Join1 pages " << joinOp1->getTotalOut() << "\n";

    std::cout << bm.getIOCount() << " IOs in total.\n";

    if (selectivity) {
        *selectivity = movieSelect->getSelectivity();
    }
    if (ioCount) {
        *ioCount = bm.getIOCount();
    }
    if (join1Page) {
        *join1Page = joinOp1->getTotalOut();
    }
}