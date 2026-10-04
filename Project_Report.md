# Project Report: Credit Risk Analysis

## 1. Problem Statement
The objective of this project is to develop a computational approach for analyzing a large collection of synthetic customer/financial records to estimate credit risk according to specified criteria. The risk is evaluated based on several factors, including:
- Debt-to-income ratio (30% weight)
- Credit utilization ratio (25% weight)
- Late payments (20% weight)
- Credit history length (10% weight)
- Loan-to-income ratio (15% weight)

Each customer is assigned a risk score from 0 to 100, which categorizes them into risk buckets: Low (< 35), Medium (35-59), and High (>= 60). The application also computes the total expected loss based on the probability of default and loan amounts, and identifies the top-10 riskiest customers. 
The analysis is applied to varying data volumes to evaluate performance and scalability.

## 2. Algorithm and Implementation
The problem is naturally data-parallel. Each customer's risk score and expected loss can be calculated independently of other customers. 

### Sequential Version (`credit_risk_seq.cpp`)
The sequential implementation iterates through the array of generated `Customer` records one by one. 
For each customer, it:
1. Calculates the risk score.
2. Increments the corresponding risk bucket counter.
3. Adds to the global sums for mean score and expected loss.
4. Pushes the record to a top-K min-heap to keep track of the riskiest customers.

### Parallel Version (`credit_risk_omp.cpp`)
The parallel implementation utilizes OpenMP to distribute the analysis across multiple CPU threads.
**Parallelization Strategy:**
- We use `#pragma omp parallel` to spawn a team of threads.
- Each thread maintains its own **private/local variables**: bucket counters (`lr.count`), sums (`lr.sum_score`, `lr.sum_loss`), and a private top-K heap (`ltop`). This avoids race conditions and false sharing during the hot loop.
- The `for` loop is parallelized using `#pragma omp for schedule(static) nowait`, meaning the dataset is divided into equal chunks and distributed to threads.
- After the parallel loop finishes, all threads enter a `#pragma omp critical` section to merge their private results into the global `Result` structure and the global `TopK` heap. Since this merging happens only once per thread, the serial overhead is minimal.

## 3. Correctness Verification
The OpenMP parallel version is verified against the sequential baseline using `compare.py`. Since floating-point arithmetic is not associative, parallel sums might yield slightly different results due to round-off errors. We account for this by comparing sums with a small tolerance (1e-8 relative), while integer bucket counts and top-K exact matches are required.

## 4. Performance Evaluation
The performance of the sequential and parallel implementations was evaluated by measuring the analysis time across varying dataset sizes (up to 50 million records) and different OpenMP thread counts (1, 2, 4, 8). 

**Key Results (50 Million Records):**
* **Sequential Baseline:** ~0.2015 seconds
* **OpenMP (2 Threads):** ~0.1062 seconds (Speedup: ~1.90x, Efficiency: ~0.95)
* **OpenMP (4 Threads):** ~0.0716 seconds (Speedup: ~2.81x, Efficiency: ~0.70)
* **OpenMP (8 Threads):** ~0.0477 seconds (Speedup: ~4.22x, Efficiency: ~0.53)

**Analysis:**
The OpenMP implementation demonstrates excellent speedup. For 50 million records, utilizing 8 threads results in a ~4.2x speedup compared to the sequential version. The efficiency drops as thread count increases, which is typical for memory-bound or highly synchronized operations, but our strategy of using thread-local variables minimized contention and avoided false sharing, leading to a very scalable solution.

*Note: Visual graphs (`speedup_vs_threads.png`, `efficiency_vs_threads.png`, and `time_vs_size.png`) and the raw data (`results.csv`) are available in the project directory.*

## 5. Team Contribution
- **Student 1**: Understanding problem statement, implementing sequential baseline.
- **Student 2**: Developing OpenMP parallel version, designing local-variables approach for race condition avoidance.
- **Student 3**: Writing `compare.py` and `benchmark.py`, setting up evaluation framework.
- **Student 4**: Conducting performance evaluation, generating graphs, compiling the final report and documentation.
