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
```
