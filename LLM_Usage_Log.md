# LLM Usage Log

| Sr. No. | Purpose | Prompt Used | How the Response Was Used | Modification / Verification |
| --- | --- | --- | --- | --- |
| 1 | Algorithm understanding | "How can we calculate credit risk score from synthetic user data (debt, income, utilization, late payments)?" | Provided the formulation for risk scoring, weights, and expected loss calculation. | Verified logic and weights according to standard financial definitions. |
| 2 | Parallelization | "How can I parallelize a loop in C++ that counts items in buckets and finds top K elements without race conditions?" | Suggested OpenMP thread-private variables and reduction, and a local min-heap per thread followed by a global merge in a critical section. | Implemented the local heap and tested it against a global critical section (which was too slow). Modified approach to merge thread-local results at the end. |
| 3 | Debugging | "My OpenMP code is producing slightly different sum_score than sequential version. Why?" | LLM explained floating point non-associativity and recommended a relative tolerance check. | Added `compare.py` with 1e-8 relative tolerance for floating-point sums. |
| 4 | Optimization | "The OpenMP version is slow when threads write to a shared array. How to fix false sharing?" | Suggested using local `Result` struct per thread instead of `#pragma omp atomic` for every iteration. | Achieved near-linear speedup after switching to thread-local accumulators. |

## Critical Evaluation of LLM Output
During the development of the parallel Top-K search, the LLM initially suggested protecting a single shared priority queue with `#pragma omp critical` for every iteration. 
- **Consideration:** We considered this because it was simple to write.
- **Modification/Rejection:** We quickly rejected this approach because benchmarking showed severe performance degradation (worse than sequential). The critical section caused massive thread contention.
- **Verification:** We modified the code to use thread-local priority queues instead, and only merge them once at the end. Benchmarks then confirmed high parallel efficiency.
