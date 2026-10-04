// credit_risk_omp.cpp - OpenMP version.
// Usage: ./credit_risk_omp <num_records> <threads> [seed=42]
//
// Strategy: records are independent, so the analysis loop is data-parallel.
//  - each thread keeps PRIVATE counters, sums and a PRIVATE top-K heap
//    (no sharing inside the loop => no races, no false sharing on hot data)
//  - private results are merged once per thread in a critical section
//    (only T merges in total, so the serial part is tiny)
#include <omp.h>
#include <cstdio>
#include <cstdlib>
#include "risk.h"

int main(int argc, char **argv) {
    if (argc < 3) { fprintf(stderr, "usage: %s N threads [seed]\n", argv[0]); return 1; }
    const long long N    = strtoll(argv[1], nullptr, 10);
    const int threads    = atoi(argv[2]);
    const uint64_t seed  = argc > 3 ? strtoull(argv[3], nullptr, 10) : 42;
    const size_t K       = 10;
    omp_set_num_threads(threads);

    // Phase 1: generation. Customer i depends only on (seed, i) => parallel-safe.
    double t0 = omp_get_wtime();
    std::vector<Customer> data(N);
    #pragma omp parallel for schedule(static)
    for (long long i = 0; i < N; i++) data[i] = make_customer(seed, (uint64_t)i);
    double t_gen = omp_get_wtime() - t0;

    // Phase 2: analysis (timed)
    t0 = omp_get_wtime();
    Result r;
    TopK top(K);
    #pragma omp parallel
    {
        Result lr;            // thread-private partial result
        TopK   ltop(K);       // thread-private top-K
        #pragma omp for schedule(static) nowait
        for (long long i = 0; i < N; i++) {
            double s = risk_score(data[i]);
            lr.count[risk_bucket(s)]++;
            lr.sum_score += s;
            lr.sum_loss  += expected_loss(s, data[i]);
            ltop.push({s, (uint64_t)i});
        }
        #pragma omp critical
        {
            for (int b = 0; b < 3; b++) r.count[b] += lr.count[b];
            r.sum_score += lr.sum_score;
            r.sum_loss  += lr.sum_loss;
            top.merge(ltop);
        }
    }
    double t_run = omp_get_wtime() - t0;

    printf("RESULT version=omp N=%lld low=%llu medium=%llu high=%llu\n", N,
           (unsigned long long)r.count[0], (unsigned long long)r.count[1],
           (unsigned long long)r.count[2]);
    printf("RESULT mean_score=%.9f total_expected_loss=%.4f\n", r.sum_score / N, r.sum_loss);
    int rank = 1;
    for (auto &e : top.sorted())
        printf("RESULT top%d idx=%llu score=%.6f\n", rank++, (unsigned long long)e.idx, e.score);
    printf("TIME version=omp threads=%d N=%lld gen_s=%.6f analysis_s=%.6f\n",
           threads, N, t_gen, t_run);
    return 0;
}
