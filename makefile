# AOI Editor Makefile
# Usage:
#   make        - Compiles the editor
#   make run    - Compiles and runs with default file
#   make clean  - Removes compiled binary

# Compiler and flags
CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2
LDFLAGS := -lncurses
TARGET := aoi
SRC := aoi.cpp
DEFAULT_FILE := example.cpp

# Default target
all: $(TARGET)

# Build the editor
$(TARGET): $(SRC)
	@echo "🛠️  Compiling AOI Editor..."
	@$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)
	@echo "✅ Successfully built $(TARGET)"

# Run the editor (compiles if needed)
run: $(TARGET)
	@echo "🚀 Launching AOI Editor with $(DEFAULT_FILE)..."
	@./$(TARGET) $(DEFAULT_FILE)

# Clean up
clean:
	@echo "🧹 Cleaning up..."
	@rm -f $(TARGET)
	@echo "✅ Clean complete"

# Help information
help:
	@echo "AOI Editor Makefile Help:"
	@echo "  make        - Compile the editor"
	@echo "  make run    - Compile and run with $(DEFAULT_FILE)"
	@echo "  make clean  - Remove compiled binary"
	@echo "  make help   - Show this help message"

.PHONY: all run clean help
