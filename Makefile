# Compiler and flags
CC := gcc
CFLAGS := -Iinclude -Wall -Wextra -g
LDFLAGS :=              # e.g. -lm if you need libm

# Directories
SRC_DIR := src
BUILD_DIR := build

# Sources and header files
SRCS := $(wildcard $(SRC_DIR)/*.c)
HDRS := $(wildcard include/*.h)
LIB_SRCS := $(filter-out $(SRC_DIR)/main.c,$(SRCS))

# Tests
TEST_DIR := tests
TEST_SRCS := $(wildcard $(TEST_DIR)/*.c)
TEST_BINS := $(patsubst $(TEST_DIR)/%.c,$(BUILD_DIR)/%,$(TEST_SRCS))

# Executable
TARGET := $(BUILD_DIR)/main

.PHONY: all run tests clean

# Default target
all: $(TARGET)

# Single-step build: compile & link directly to build/main
$(TARGET): $(SRCS) $(HDRS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SRCS) -o $@ $(LDFLAGS)

# Build all test harnesses
tests: $(TEST_BINS)

$(BUILD_DIR)/%: $(TEST_DIR)/%.c $(LIB_SRCS) $(HDRS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(LIB_SRCS) $< -o $@ $(LDFLAGS)

# Ensure build directory exists
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Run the executable
run: $(TARGET)
	./$(TARGET) 

# Clean
clean:
	rm -rf $(BUILD_DIR)
