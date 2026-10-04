// risk.h - shared by the sequential and OpenMP versions.
// Keeping the data model and scoring formula in ONE place guarantees both
// versions compute exactly the same thing.
#pragma once
#include <algorithm>
#include <cstdint>
#include <queue>
#include <vector>

// ---------------------------------------------------------------- data model
struct Customer {
    float income;          // annual income
    float debt;            // total outstanding debt
    float utilization;     // credit utilization ratio, 0..1
    float loan_amount;     // requested / outstanding loan
    float account_age;     // months
    int   late_payments;   // late payments in last 24 months
};

// ------------------------------------------------- deterministic generation
// Customer i depends ONLY on (seed, i), so generation can be parallelised
// and still give identical data to the sequential version.
static inline uint64_t splitmix64(uint64_t &s) {
    uint64_t z = (s += 0x9E3779B97F4A7C15ULL);
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
    return z ^ (z >> 31);
}
static inline double u01(uint64_t &s) {
    return (splitmix64(s) >> 11) * (1.0 / 9007199254740992.0);
}

static inline Customer make_customer(uint64_t seed, uint64_t i) {
    uint64_t s = seed ^ (i * 0xD1B54A32D192ED03ULL);
    Customer c;
    double inc = 15000.0 + u01(s) * 185000.0;
    double r   = u01(s);
    c.income        = (float)inc;
    c.debt          = (float)(inc * r * r * 1.2);        // skewed: most have low debt
    c.utilization   = (float)u01(s);
    c.loan_amount   = (float)(inc * (0.1 + u01(s) * 2.0));
    c.account_age   = (float)(u01(s) * 240.0);
    double l        = u01(s);
    c.late_payments = (int)(l * l * l * 10.0);           // most have few late payments
    return c;
}

// --------------------------------------------------------------- risk model
// Risk score in [0,100]; higher = riskier. Criteria and weights:
//   debt-to-income 30% | utilization 25% | late payments 20%
//   short credit history 10% | loan-to-income 15%
static inline double risk_score(const Customer &c) {
    double dti  = std::min((double)c.debt / (double)c.income, 1.0);
    double lti  = std::min((double)c.loan_amount / (double)c.income / 2.5, 1.0);
    double late = std::min(c.late_payments / 6.0, 1.0);
    double hist = 1.0 - std::min((double)c.account_age / 120.0, 1.0);
    return 100.0 * (0.30 * dti + 0.25 * c.utilization + 0.20 * late +
                    0.10 * hist + 0.15 * lti);
}
static inline int risk_bucket(double score) {   // 0 = Low, 1 = Medium, 2 = High
    return score < 35.0 ? 0 : (score < 60.0 ? 1 : 2);
}
// Expected loss: PD proxy = (score/100)^2, loss-given-default = 45%
static inline double expected_loss(double score, const Customer &c) {
    double pd = (score / 100.0) * (score / 100.0);
    return pd * (double)c.loan_amount * 0.45;
}

// -------------------------------------------------------------- top-K riskiest
struct Entry { double score; uint64_t idx; };
// total order (score desc, index asc) => top-K is unique, so results are
// comparable exactly between sequential and parallel runs
static inline bool better(const Entry &a, const Entry &b) {
    return a.score > b.score || (a.score == b.score && a.idx < b.idx);
}
struct TopK {
    size_t k;
    // heap top = the WORST entry currently kept
    std::priority_queue<Entry, std::vector<Entry>, bool (*)(const Entry &, const Entry &)> h;
    explicit TopK(size_t k_) : k(k_), h(better) {}
    void push(const Entry &e) {
        if (h.size() < k) h.push(e);
        else if (better(e, h.top())) { h.pop(); h.push(e); }
    }
    void merge(const TopK &o) {
        auto copy = o.h;
        while (!copy.empty()) { push(copy.top()); copy.pop(); }
    }
    std::vector<Entry> sorted() const {
        std::vector<Entry> v;
        auto copy = h;
        while (!copy.empty()) { v.push_back(copy.top()); copy.pop(); }
        std::sort(v.begin(), v.end(), better);
        return v;
    }
};

// ------------------------------------------------------------ analysis result
struct Result {
    uint64_t count[3] = {0, 0, 0};
    double   sum_score = 0.0;
    double   sum_loss  = 0.0;
};
