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

To run the correctness and performance test on small dataset with 2000 rows, run:

```
./test_lab2_end2end_small_dataset
```

To run the correctness and performance test, run:

```
./test_lab2_end2end
```

To plot a graph of the result from test_end2end, run:

```
python3 plot_results.py
```
