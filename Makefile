# C Path Tracer Makefile
CC = gcc
CFLAGS = -O3 -march=native -mtune=native -fopenmp -Wall -Wextra -Wunused -Wunused-function -Wunused-variable -std=c11 -Iinclude
CFLAGS_DEBUG = -g -O0 -fopenmp -Wall -Wextra -std=c11 -fsanitize=address -Iinclude
LDFLAGS = -lm -fopenmp

# GTK flags
GTK_CFLAGS = `pkg-config --cflags gtk+-3.0` -pthread
GTK_LIBS = `pkg-config --libs gtk+-3.0` -pthread

# Directories
SRC_DIR = src
INCLUDE_DIR = include
OUTPUT_DIR = output

# Common source files
COMMON_SRCS = $(SRC_DIR)/pathtracer.c $(SRC_DIR)/primitive.c $(SRC_DIR)/material.c $(SRC_DIR)/bvh.c $(SRC_DIR)/scenes.c
COMMON_OBJS = $(COMMON_SRCS:.c=.o)

# GUI source files
GUI_SRCS = $(SRC_DIR)/main_gui.c $(SRC_DIR)/gui.c
GUI_OBJS = $(GUI_SRCS:.c=.o)
TARGET = pathtracer_gui

# Default target - build GUI application (keep .o files for incremental compilation)
all: $(TARGET)

# Release build - compile and auto-cleanup object files
release: $(TARGET)
	@echo "Cleaning up object files..."
	@rm -f $(COMMON_OBJS) $(GUI_OBJS)
	@echo "Build complete! Object files cleaned."

# Build GUI executable
$(TARGET): $(COMMON_OBJS) $(GUI_OBJS)
	$(CC) $(CFLAGS) $(GTK_CFLAGS) -o $@ $^ $(LDFLAGS) $(GTK_LIBS)

# Compile common source files
$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Compile GUI source files with GTK flags
$(SRC_DIR)/gui.o: $(SRC_DIR)/gui.c
	$(CC) $(CFLAGS) $(GTK_CFLAGS) -c $< -o $@

$(SRC_DIR)/main_gui.o: $(SRC_DIR)/main_gui.c
	$(CC) $(CFLAGS) $(GTK_CFLAGS) -c $< -o $@

# Debug build
debug: CFLAGS = $(CFLAGS_DEBUG)
debug: clean all

# Clean build artifacts
clean:
	rm -f $(COMMON_OBJS) $(GUI_OBJS) $(TARGET)
	rm -f $(OUTPUT_DIR)/*.bmp

# Run GUI
run: $(TARGET)
	./$(TARGET)

# Static Analysis - Check for unused code
analyze:
	@echo "================================================"
	@echo "Static Code Analysis"
	@echo "================================================"
	@echo ""
	@echo "1. Checking for unused functions/variables with GCC..."
	@$(MAKE) clean > /dev/null 2>&1
	@$(MAKE) all 2>&1 | grep -i "unused" || echo "    No unused code detected by GCC"
	@echo ""
	@echo "2. Checking for defined symbols in binary..."
	@nm -g $(TARGET) 2>/dev/null | grep -E " [TtWw] " | grep -v "@@" | grep -v "^[[:space:]]*w" | wc -l | xargs -I {} echo "   {} functions compiled into binary"
	@echo ""
	@echo "3. Additional tools (install if needed):"
	@which cppcheck > /dev/null 2>&1 && echo "    cppcheck available - run 'make cppcheck'" || echo "    cppcheck not installed - install: sudo apt install cppcheck"
	@which clang-tidy > /dev/null 2>&1 && echo "    clang-tidy available - run 'make lint'" || echo "    clang-tidy not installed - install: sudo apt install clang-tidy"
	@echo ""

# Deep static analysis with cppcheck (if installed)
cppcheck:
	@which cppcheck > /dev/null || (echo "Error: cppcheck not installed. Run: sudo apt install cppcheck" && exit 1)
	@echo "Running cppcheck static analysis..."
	cppcheck --enable=all --inconclusive --suppress=missingIncludeSystem \
		-I$(INCLUDE_DIR) $(SRC_DIR)/*.c 2>&1 | grep -v "^Checking"

# Lint with clang-tidy (if installed)
lint:
	@which clang-tidy > /dev/null || (echo "Error: clang-tidy not installed. Run: sudo apt install clang-tidy" && exit 1)
	@echo "Running clang-tidy..."
	clang-tidy $(SRC_DIR)/*.c -- $(CFLAGS) $(GTK_CFLAGS)

# Check GTK dependencies
check_deps:
	@echo "Checking dependencies..."
	@pkg-config --exists gtk+-3.0 && echo " GTK3 found" || echo " GTK3 not found - install with: sudo apt-get install libgtk-3-dev"
	@which gcc > /dev/null && echo " GCC found" || echo " GCC not found"
	@echo ""
	@echo "GTK3 version: `pkg-config --modversion gtk+-3.0 2>/dev/null || echo 'Not installed'`"
	@echo "GCC version: `gcc --version | head -n1`"

# Installation
PREFIX = /usr/local
install: $(TARGET)
	install -d $(PREFIX)/bin
	install -m 755 $(TARGET) $(PREFIX)/bin

uninstall:
	rm -f $(PREFIX)/bin/$(TARGET)

.PHONY: all release clean run debug analyze cppcheck lint check_deps install uninstall