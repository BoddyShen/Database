## Prerequisites

- A C++ compiler (e.g., g++)
- GNU CMake
- Add `title.basics.tsv` in lab2 folder (Ref: https://datasets.imdbws.com/)
- python, panda, matplotlib

## How to Build and Run

This project uses a CMake to manage the build process.

### Build the Project

To compile the project, enter the lab2 folder and run:

```
mkdir -p build && cd build && cmake .. && make
```

To run a simple unit test on B+ tree, run:

```
./test_unit
```

To run the correctness and performance test on small dataset with 2000 rows, run:

```
./test_end2end_small_dataset
```

To run the correctness and performance test, run:

```
./test_end2end
```

To plot a graph of the result from test_end2end, run:

```
python3 plot_results.py
```
