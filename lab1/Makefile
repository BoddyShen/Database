# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Iinclude

# Directories
SRC_DIR = src
TEST_UNIT_DIR = test_unit
TEST_END2END_DIR = test_end2end
OBJ_DIR = build

# Production binary
BIN = main
# Get production source files and corresponding object files
PROD_SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
PROD_OBJECTS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(PROD_SOURCES))
# Filter out main.o (from src/main.cpp) for test linking
PROD_OBJECTS_FOR_TEST = $(filter-out $(OBJ_DIR)/main.o, $(PROD_OBJECTS))

#####################
# Unit test binaries
#####################
TEST_UNIT_SOURCES = $(wildcard $(TEST_UNIT_DIR)/*.cpp)
# Prepend "unit_" to avoid name collision
TEST_UNIT_BINS = $(patsubst $(TEST_UNIT_DIR)/%.cpp, $(OBJ_DIR)/unit_%, $(TEST_UNIT_SOURCES))

# For each unit test source file, build an independent binary (excluding main.o)
$(OBJ_DIR)/unit_%: $(TEST_UNIT_DIR)/%.cpp $(PROD_OBJECTS_FOR_TEST) | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $< $(PROD_OBJECTS_FOR_TEST) -o $@

###########################
# End-to-end test binaries
###########################
TEST_END2END_SOURCES = $(wildcard $(TEST_END2END_DIR)/*.cpp)
# Prepend "e2e_" to avoid name collision
TEST_END2END_BINS = $(patsubst $(TEST_END2END_DIR)/%.cpp, $(OBJ_DIR)/e2e_%, $(TEST_END2END_SOURCES))

# For each end-to-end test source file, build an independent binary (excluding main.o)
$(OBJ_DIR)/e2e_%: $(TEST_END2END_DIR)/%.cpp $(PROD_OBJECTS_FOR_TEST) | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $< $(PROD_OBJECTS_FOR_TEST) -o $@

##################################
# Production binary build section
##################################
# Link production object files to create the production binary
$(BIN): $(PROD_OBJECTS)
	$(CXX) $(PROD_OBJECTS) -o $(BIN)

# Compile production source files into object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

##################################
# Build directory
##################################
# Create the build directory if it does not exist
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

##################################
# Targets
##################################
# Default target: build the production binary
all: $(BIN)

# Run target: execute the production binary
run: all
	./$(BIN)

# Unit test target: build and run all unit test binaries
unittest: $(TEST_UNIT_BINS)
	@for test_exe in $(TEST_UNIT_BINS); do \
		echo "Running $$test_exe..."; \
		./$$test_exe; \
	done

# End-to-end test target: build and run all end-to-end test binaries
end2endtest: $(TEST_END2END_BINS)
	@for test_exe in $(TEST_END2END_BINS); do \
		echo "Running $$test_exe..."; \
		./$$test_exe; \
	done

# Clean target: remove all object files and binaries
clean:
	rm -rf $(OBJ_DIR) $(BIN)

.PHONY: all run unittest end2endtest clean