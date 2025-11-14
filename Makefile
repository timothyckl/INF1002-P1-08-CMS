# Compiler and flags
CC := gcc
CFLAGS := -Iinclude -Wall -Wextra -g
LDFLAGS :=              # e.g. -lm if you need libm

# Directories
SRC_DIR := src
BUILD_DIR := build

# Sources and header files
SRCS := $(wildcard $(SRC_DIR)/*.c)
HDRS := $(wildcard $(SRC_DIR)/*.h)

# Executable
TARGET := $(BUILD_DIR)/main

.PHONY: all run clean

# Default target
all: $(TARGET)

# Single-step build: compile & link directly to build/main
$(TARGET): $(SRCS) $(HDRS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SRCS) -o $@ $(LDFLAGS)

# Ensure build directory exists
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Run the executable
run: $(TARGET)
	./$(TARGET) 

# Clean
clean:
	rm -rf $(BUILD_DIR)
