## Prerequisites

- A C++ compiler (e.g., g++)
- GNU CMake
- Add `title.basics.tsv` in lab3 folder (Ref: https://datasets.imdbws.com/)
- python, panda, matplotlib

## How to Build and Run

This project uses a CMake to manage the build process.

### Build the Project

To compile the project, enter the lab3 folder and run:

```
mkdir -p build && cd build && cmake .. && make
```

To run unit test, run:

```
make run_unit_tests
```

To run the database, run:

```
./db_engine
```

In the interactive mode,

For testing purpose, we prepare 100000 rows of each tsv file,

To run pre_process in test mode, run:

```
pre_process test
```

To run run_query in test mode, run:

```
run_query <start_range> <end_range> <buffer_size> test
e.g. run_query A Z 20 test
```

To compare cpp query result and PostgreSQL result (correctness test), run:
// please make sure the path is under the folder /lab3
```
$ pwd 
/UMass-CS645/lab3

$ ./test/run_and_compare.sh [start_range] [end_range]

// like
$ ./test/run_and_compare.sh A C
```

At the bottom of the output, it would indicate if two results are the same, see below:
```
$ ./test/run_and_compare.sh A C
...
...
...
Congratulation! The two are the same!
--- PostgreSQL script finished. ---
```

If interested in actual output of each:

- Cpp output file path: `/lab3/sorted_cpp.tsv` (sorted), `/lab3/build/cpp_join_out.tsv`   
- postgreSQL output file path: `/lab3/sorted_psql.tsv` (sorted), `lab3/postgreSQL_output.tsv` 


