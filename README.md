## Prerequisites

- A C++ compiler (e.g., g++)
- GNU Make
- Add `title.basics.tsv` in lab1 folder (Ref: https://datasets.imdbws.com/)

## How to Build and Run

This project uses a Makefile to manage the build process.

### Build the Project

To compile the project, enter each lab folder and run:

```
make
```

### Test the Project

To do unit test on the project in `test_unit` folder, enter each lab folder and run:

```
make unittest
```

To do end-to-end test on the project in `test_end2end` folder, enter each lab folder and run:

```
make end2endtest
```

### Clean the Project

To remove the compiled files and the executable in `build/`, run:

```
make clean
```
