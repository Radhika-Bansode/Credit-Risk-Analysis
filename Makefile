# ==============================================================================
# Makefile - Credit Risk Analysis (Sequential & OpenMP)
# ==============================================================================

CXX ?= clang++
CXXFLAGS = -O3 -std=c++17 -Wall -Wextra

SRC_DIR     = src
BIN_DIR     = bin
SCRIPTS_DIR = scripts
DATA_DIR    = data
OUTPUT_DIR  = output

TARGET_SEQ = $(BIN_DIR)/credit_risk_seq
TARGET_OMP = $(BIN_DIR)/credit_risk_omp

# OS-specific OpenMP configuration
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    # macOS with Homebrew libomp
    BREW_LIBOMP := $(shell brew --prefix libomp 2>/dev/null || echo /opt/homebrew/opt/libomp)
    OMP_FLAGS   := -Xpreprocessor -fopenmp -I$(BREW_LIBOMP)/include
    OMP_LIBS    := -L$(BREW_LIBOMP)/lib -lomp
else
    # Linux / GCC
    CXX        := g++
    OMP_FLAGS  := -fopenmp
    OMP_LIBS   := -fopenmp
endif

.PHONY: all seq omp clean run-seq run-omp verify benchmark help

all: $(TARGET_SEQ) $(TARGET_OMP)

seq: $(TARGET_SEQ)

omp: $(TARGET_OMP)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(TARGET_SEQ): $(SRC_DIR)/credit_risk_seq.cpp $(SRC_DIR)/risk.h | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -I$(SRC_DIR) $< -o $@

$(TARGET_OMP): $(SRC_DIR)/credit_risk_omp.cpp $(SRC_DIR)/risk.h | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(OMP_FLAGS) -I$(SRC_DIR) $< $(OMP_LIBS) -o $@

# Run sequential baseline (default 1,000,000 records)
run-seq: $(TARGET_SEQ)
	./$(TARGET_SEQ) 1000000

# Run OpenMP parallel version (default 1,000,000 records, 4 threads)
run-omp: $(TARGET_OMP)
	./$(TARGET_OMP) 1000000 4

# Run correctness comparison script
verify: $(TARGET_SEQ) $(TARGET_OMP)
	python3 $(SCRIPTS_DIR)/compare.py 1000000 1 2 4 8

# Run performance benchmark and generate plots
benchmark: $(TARGET_SEQ) $(TARGET_OMP)
	python3 $(SCRIPTS_DIR)/benchmark.py

clean:
	rm -f $(TARGET_SEQ) $(TARGET_OMP)

help:
	@echo "Available targets:"
	@echo "  all        - Build both sequential and OpenMP binaries (default)"
	@echo "  seq        - Build sequential binary ($(TARGET_SEQ))"
	@echo "  omp        - Build OpenMP binary ($(TARGET_OMP))"
	@echo "  run-seq    - Run sequential version with 1,000,000 records"
	@echo "  run-omp    - Run OpenMP version with 1,000,000 records and 4 threads"
	@echo "  verify     - Verify numerical correctness against sequential baseline"
	@echo "  benchmark  - Run benchmark suite and generate output plots"
	@echo "  clean      - Remove compiled binaries from $(BIN_DIR)/"
