# =============================================================================
# Makefile for Complaint Handling System (C11)
# Works on Windows (MinGW) and Linux/macOS
# =============================================================================

CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -Iinclude -D_POSIX_C_SOURCE=200809L
LDFLAGS = 

# Directories
SRC_DIR = src
INC_DIR = include
BIN_DIR = bin
DATA_DIR = data
TEST_DIR = tests

# Target binary
ifeq ($(OS),Windows_NT)
    TARGET = $(BIN_DIR)/complaint_system.exe
    TEST_TARGET = $(BIN_DIR)/test_runner.exe
    MKDIR = if not exist "$(subst /,\,$(1))" mkdir "$(subst /,\,$(1))"
    RM = del /Q /F
    RMDIR = rmdir /S /Q
    FIXPATH = $(subst /,\,$(1))
else
    TARGET = $(BIN_DIR)/complaint_system
    TEST_TARGET = $(BIN_DIR)/test_runner
    MKDIR = mkdir -p $(1)
    RM = rm -f
    RMDIR = rm -rf
    FIXPATH = $(1)
endif

# Source and Object files
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(BIN_DIR)/%.o, $(SRCS))

# Default rule
all: dirs $(TARGET)

# Ensure required directories exist
dirs:
	@$(call MKDIR, $(BIN_DIR))
	@$(call MKDIR, $(DATA_DIR))

# Link the main executable
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)
	@echo Build completed successfully: $(TARGET)

# Compile C source files to object files
$(BIN_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Run the system
run: all
	$(TARGET)

# Clean build artifacts
clean:
ifeq ($(OS),Windows_NT)
	@if exist $(BIN_DIR)\*.o del /Q $(BIN_DIR)\*.o
	@if exist $(TARGET) del /Q $(TARGET)
	@if exist $(TEST_TARGET) del /Q $(TEST_TARGET)
else
	$(RM) $(BIN_DIR)/*.o $(TARGET) $(TEST_TARGET)
endif
	@echo Clean completed.

# Test rule
TEST_SRCS = $(filter-out $(SRC_DIR)/main.c, $(SRCS)) $(TEST_DIR)/test_runner.c

test: dirs $(TEST_TARGET)
	$(TEST_TARGET)

$(TEST_TARGET): $(TEST_SRCS)
	$(CC) $(CFLAGS) $(TEST_SRCS) -o $(TEST_TARGET) $(LDFLAGS)
	@echo Test suite built successfully: $(TEST_TARGET)

.PHONY: all dirs run test clean
