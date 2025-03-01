# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Iinclude

# Directories
SRC_DIR = src
TEST_DIR = test
OBJ_DIR = build

# Production binary
BIN = main
# Get production source files and corresponding object files
PROD_SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
PROD_OBJECTS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(PROD_SOURCES))
# Filter out main.o (from src/main.cpp) for test linking
PROD_OBJECTS_FOR_TEST = $(filter-out $(OBJ_DIR)/main.o, $(PROD_OBJECTS))

# Test source files and corresponding test binaries
# (Assuming each .cpp file in the test directory has its own main())
TEST_SOURCES = $(wildcard $(TEST_DIR)/*.cpp)
TEST_BINS = $(patsubst $(TEST_DIR)/%.cpp, $(OBJ_DIR)/%, $(TEST_SOURCES))

# Link production object files to create the production binary
$(BIN): $(PROD_OBJECTS)
	$(CXX) $(PROD_OBJECTS) -o $(BIN)

# Compile production source files into object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# For each test source file, build an independent binary (excluding main.o)
$(OBJ_DIR)/%: $(TEST_DIR)/%.cpp $(PROD_OBJECTS_FOR_TEST) | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $< $(PROD_OBJECTS_FOR_TEST) -o $@

# Create the build directory if it does not exist
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Default target: build the production binary
all: $(BIN)

# Run target: execute the production binary
run: all
	./$(BIN)

# Test target: build all test binaries and run them one by one
test: $(TEST_BINS)
	@for test_exe in $(TEST_BINS); do \
		echo "Running $$test_exe..."; \
		./$$test_exe; \
	done

# Clean target: remove all object files and binaries
clean:
	rm -rf $(OBJ_DIR) $(BIN)

.PHONY: all run test clean