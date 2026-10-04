# Q18 - Credit Risk Analysis (Sequential vs OpenMP)

Scores a large set of synthetic customer records for credit risk, buckets them
(Low < 35 <= Medium < 60 <= High), and reports bucket counts, mean score, total
expected loss and the top-10 riskiest customers.

## Files
| File | Purpose |
|---|---|
| `risk.h` | Data model, deterministic data generator, scoring formula, top-K (shared by both versions) |
| `credit_risk_seq.cpp` | Sequential baseline |
| `credit_risk_omp.cpp` | OpenMP version |
| `compare.py` | Correctness check (parallel output vs sequential) |
| `benchmark.py` | Timing runs -> `results.csv` and graphs |
| `sample_data_1000.csv` | Small sample dataset (same generator, seed 42) |

## Build (g++ with OpenMP; Linux/macOS/WSL)
    g++ -O2 -std=c++17 -o credit_risk_seq credit_risk_seq.cpp
    g++ -O2 -std=c++17 -fopenmp -o credit_risk_omp credit_risk_omp.cpp

## Run
    ./credit_risk_seq 10000000            # N records
    ./credit_risk_omp 10000000 8          # N records, 8 threads
    ./credit_risk_seq 1000 42 data.csv    # also dump the data as CSV

## Verify and benchmark
    python3 compare.py 1000000 1 2 4 8
    python3 benchmark.py --sizes 100000 1000000 10000000 50000000 --threads 1 2 4 8 --reps 3

Only the analysis phase is timed (`analysis_s`); data generation (`gen_s`) is reported separately.
Memory needed is about 24 bytes x N (50M records ~ 1.2 GB).
