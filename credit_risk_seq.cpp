// credit_risk_seq.cpp - sequential baseline.
// Usage: ./credit_risk_seq <num_records> [seed=42] [dump.csv]
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include "risk.h"

static double now() {
    using namespace std::chrono;
    return duration<double>(steady_clock::now().time_since_epoch()).count();
}

int main(int argc, char **argv) {
    if (argc < 2) { fprintf(stderr, "usage: %s N [seed] [dump.csv]\n", argv[0]); return 1; }
    const uint64_t N    = strtoull(argv[1], nullptr, 10);
    const uint64_t seed = argc > 2 ? strtoull(argv[2], nullptr, 10) : 42;
    const size_t   K    = 10;

    // Phase 1: generate data (not part of the timed analysis)
    double t0 = now();
    std::vector<Customer> data(N);
    for (uint64_t i = 0; i < N; i++) data[i] = make_customer(seed, i);
    double t_gen = now() - t0;

    // Phase 2: analysis (timed)
    t0 = now();
    Result r;
    TopK top(K);
    for (uint64_t i = 0; i < N; i++) {
        double s = risk_score(data[i]);
        r.count[risk_bucket(s)]++;
        r.sum_score += s;
        r.sum_loss  += expected_loss(s, data[i]);
        top.push({s, i});
    }
    double t_run = now() - t0;

    // Optional: dump (a small) dataset as CSV for the "input data" deliverable
    if (argc > 3) {
        FILE *f = fopen(argv[3], "w");
        fprintf(f, "id,income,debt,utilization,loan_amount,account_age_months,late_payments\n");
        for (uint64_t i = 0; i < N; i++) {
            const Customer &c = data[i];
            fprintf(f, "%llu,%.2f,%.2f,%.4f,%.2f,%.1f,%d\n", (unsigned long long)i,
                    c.income, c.debt, c.utilization, c.loan_amount, c.account_age, c.late_payments);
        }
        fclose(f);
    }

    // Parseable output (compare.py checks lines starting with RESULT)
    printf("RESULT version=seq N=%llu low=%llu medium=%llu high=%llu\n",
           (unsigned long long)N, (unsigned long long)r.count[0],
           (unsigned long long)r.count[1], (unsigned long long)r.count[2]);
    printf("RESULT mean_score=%.9f total_expected_loss=%.4f\n", r.sum_score / N, r.sum_loss);
    int rank = 1;
    for (auto &e : top.sorted())
        printf("RESULT top%d idx=%llu score=%.6f\n", rank++, (unsigned long long)e.idx, e.score);
    printf("TIME version=seq threads=1 N=%llu gen_s=%.6f analysis_s=%.6f\n",
           (unsigned long long)N, t_gen, t_run);
    return 0;
}
