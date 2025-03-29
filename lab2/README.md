## Prerequisites

- A C++ compiler (e.g., g++)
- GNU CMake
- Add `title.basics.tsv` in lab2 folder (Ref: https://datasets.imdbws.com/)

## How to Build and Run

This project uses a CMake to manage the build process.

### Build the Project

To compile the project, enter the lab2 folder and run:

```
mkdir -p build && cd build && cmake .. && make
```

To run a simple test on B+ tree, run:
```
./main
```

To run the correctness and performance test, run:
```
g++ -o test_all test/test_all.cpp src/BufferManager.cpp src/Page.cpp src/Utilities.cpp src/LRUCache.cpp -I include/ 
./test_all
```
