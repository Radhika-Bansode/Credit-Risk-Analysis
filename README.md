# Credit Risk Analysis (Sequential vs OpenMP)

A high-performance computational framework in C++ comparing sequential and OpenMP parallel implementations for evaluating credit risk on large-scale synthetic customer records.

The system calculates individual risk scores, categorizes customers into risk buckets (Low, Medium, High), computes expected credit losses, and tracks the top-$K$ riskiest customers using thread-local min-heaps and lock-free processing.

---

## Repository Structure

```text
Credit_Risk_Analysis/
├── bin/                          # Compiled executable binaries
│   ├── credit_risk_seq           # Sequential executable
│   └── credit_risk_omp           # OpenMP parallel executable
├── data/                         # Sample and test datasets
│   └── sample_data_1000.csv      # Sample synthetic customer records (1,000 rows)
├── docs/                         # Project documentation and reports
│   ├── Project_Report.md         # Comprehensive project report & performance analysis
│   ├── LLM_Usage_Log.md          # LLM usage log and critical design evaluations
│   └── LLM_Usage_Log_Preview.html# Formatted HTML report preview
├── output/                       # Generated benchmark results and visualizations
│   ├── results.csv               # Benchmark timing, speedup, and efficiency data
│   ├── speedup_vs_threads.png    # Speedup vs thread count plot
│   ├── efficiency_vs_threads.png # Parallel efficiency vs thread count plot
│   └── time_vs_size.png          # Execution time vs dataset size (log-log)
├── scripts/                      # Verification and automation scripts
│   ├── benchmark.py              # Automated benchmarking and plot generation suite
│   └── compare.py                # Numerical correctness validation script
├── src/                          # C++ source code and headers
│   ├── credit_risk_seq.cpp       # Sequential baseline implementation
│   ├── credit_risk_omp.cpp       # Multi-threaded OpenMP implementation
│   └── risk.h                    # Shared risk model, data generator, and heap algorithms
├── Makefile                      # Cross-platform build and automation script
├── requirements.txt              # Python dependencies for plotting (matplotlib)
├── .gitignore                    # Git ignore file for OS, python, and build artifacts
└── README.md                     # Project overview and documentation
```

---

## Directory Details

| Directory | Purpose |
|---|---|
| **`src/`** | Core C++ implementations (`credit_risk_seq.cpp`, `credit_risk_omp.cpp`) and header definitions (`risk.h`). |
| **`bin/`** | Compiled binaries output by the build process. |
| **`scripts/`** | Python scripts for correctness verification (`compare.py`) and performance benchmarks (`benchmark.py`). |
| **`output/`** | Benchmark output metrics (`results.csv`) and generated graphs (`*.png`). |
| **`data/`** | Exported sample CSV dataset generated via deterministic PRNG. |
| **`docs/`** | Complete academic report (`Project_Report.md`) and development log (`LLM_Usage_Log.md`). |

---

## Prerequisites

- **C++ Compiler**: `g++` (Linux) or `clang++` with OpenMP (macOS with Homebrew `libomp`).
- **Standard**: C++17 or newer.
- **Python**: Python 3.8+ (with `matplotlib` for graph generation).

```bash
# Optional: install Python dependencies for benchmarking graphs
pip install -r requirements.txt
```

---

## Build Instructions

### Quick Build with Makefile (Recommended)

The provided `Makefile` automatically detects your operating system (macOS or Linux) and configures the appropriate OpenMP flags:

```bash
# Build both sequential and OpenMP executables
make

# Or build individual targets
make seq   # Builds bin/credit_risk_seq
make omp   # Builds bin/credit_risk_omp
```

### Manual Compilation

**On Linux (GCC):**
```bash
mkdir -p bin
g++ -O3 -std=c++17 -Isrc src/credit_risk_seq.cpp -o bin/credit_risk_seq
g++ -O3 -std=c++17 -fopenmp -Isrc src/credit_risk_omp.cpp -o bin/credit_risk_omp
```

**On macOS (Apple Clang + Homebrew libomp):**
```bash
mkdir -p bin
clang++ -O3 -std=c++17 -Isrc src/credit_risk_seq.cpp -o bin/credit_risk_seq
clang++ -O3 -std=c++17 -Xpreprocessor -fopenmp \
    -I$(brew --prefix libomp)/include -Isrc src/credit_risk_omp.cpp \
    -L$(brew --prefix libomp)/lib -lomp -o bin/credit_risk_omp
```

---

## Usage

### 1. Run Sequential Baseline
```bash
./bin/credit_risk_seq <num_records> [seed=42] [dump_path.csv]

# Example: Run on 10,000,000 records
./bin/credit_risk_seq 10000000

# Example: Generate and save 1,000 records to CSV
./bin/credit_risk_seq 1000 42 data/sample_data_1000.csv
```

### 2. Run OpenMP Parallel Implementation
```bash
./bin/credit_risk_omp <num_records> <num_threads> [seed=42]

# Example: Run on 10,000,000 records with 8 threads
./bin/credit_risk_omp 10000000 8
```

---

## Verification & Benchmarks

### Correctness Verification
Validates that parallel thread execution matches the sequential reference (exact match on bucket counts and top-$K$, with floating-point tolerance on sums):

```bash
make verify
# or:
python3 scripts/compare.py 1000000 1 2 4 8
```

### Automated Benchmarking
Runs timing trials across multiple input sizes and thread configurations, saving metrics to `output/results.csv` and rendering plots in `output/`:

```bash
make benchmark
# or:
python3 scripts/benchmark.py --sizes 100000 1000000 10000000 50000000 --threads 1 2 4 8 --reps 3
```

---

## Performance Summary (50M Records on Apple M4)

| Configuration | Time (s) | Speedup | Parallel Efficiency |
|---|---|---|---|
| Sequential Baseline | 0.2015 s | 1.00× | 100% |
| OpenMP (2 Threads) | 0.1062 s | 1.90× | 95.0% |
| OpenMP (4 Threads) | 0.0716 s | 2.81× | 70.3% |
| OpenMP (8 Threads) | 0.0477 s | 4.22× | 52.8% |

Detailed charts and analysis are located in the [`output/`](output/) and [`docs/`](docs/) directories.
