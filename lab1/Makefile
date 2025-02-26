# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Iinclude

# Directories
SRC_DIR = src
OBJ_DIR = build
BIN = main

# Find all source files in the src directory
SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
# Map source files to object files in the build directory
OBJECTS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SOURCES))


# Link object files to create the binary
$(BIN): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(BIN)

# Compile each source file into an object file
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Create the build directory if it doesn't exist
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Default target builds the binary executable
all: $(BIN)

# Run target to execute the binary
run: all
	./$(BIN)

# Clean target to remove built files
clean:
	rm -rf $(OBJ_DIR) $(BIN)

.PHONY: all clean run
