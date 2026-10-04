#!/usr/bin/env python3
"""Benchmark seq vs OpenMP; writes results.csv and graphs (PNG).
Usage: python3 benchmark.py [--sizes 1000000 10000000 ...] [--threads 1 2 4 8] [--reps 3]
Metrics: speedup S = T_seq / T_par,  efficiency E = S / threads."""
import argparse, csv, re, subprocess
import matplotlib; matplotlib.use("Agg")
import matplotlib.pyplot as plt

ap = argparse.ArgumentParser()
ap.add_argument("--sizes", type=int, nargs="+", default=[10**5, 10**6, 10**7, 5*10**7])
ap.add_argument("--threads", type=int, nargs="+", default=[1, 2, 4, 8])
ap.add_argument("--reps", type=int, default=3)
a = ap.parse_args()

def analysis_time(cmd):
    out = subprocess.run(cmd, capture_output=True, text=True, check=True).stdout
    return float(re.search(r"analysis_s=([\d.]+)", out).group(1))

def best(cmd):  # best-of-reps reduces noise from other processes
    return min(analysis_time(cmd) for _ in range(a.reps))

rows = []
for n in a.sizes:
    t_seq = best(["./credit_risk_seq", str(n)])
    rows.append(dict(N=n, threads=1, version="seq", time_s=t_seq, speedup=1.0, efficiency=1.0))
    print(f"N={n:>10} seq {t_seq:.4f}s")
    for t in a.threads:
        t_par = best(["./credit_risk_omp", str(n), str(t)])
        s = t_seq / t_par
        rows.append(dict(N=n, threads=t, version="omp", time_s=t_par, speedup=s, efficiency=s / t))
        print(f"N={n:>10} omp t={t:<2} {t_par:.4f}s  speedup={s:.2f}  eff={s/t:.2f}")

with open("results.csv", "w", newline="") as f:
    w = csv.DictWriter(f, fieldnames=rows[0].keys()); w.writeheader(); w.writerows(rows)

omp = [r for r in rows if r["version"] == "omp"]
def plot(ykey, ylabel, fname, ideal=False):
    plt.figure(figsize=(6, 4))
    for n in a.sizes:
        pts = [r for r in omp if r["N"] == n]
        plt.plot([r["threads"] for r in pts], [r[ykey] for r in pts], marker="o", label=f"N={n:,}")
    if ideal: plt.plot(a.threads, a.threads, "k--", label="ideal")
    plt.xlabel("Threads"); plt.ylabel(ylabel); plt.grid(alpha=.3); plt.legend()
    plt.tight_layout(); plt.savefig(fname, dpi=150); plt.close()
plot("speedup", "Speedup", "speedup_vs_threads.png", ideal=True)
plot("efficiency", "Parallel efficiency", "efficiency_vs_threads.png")

plt.figure(figsize=(6, 4))   # time vs input size (log-log)
seq = [r for r in rows if r["version"] == "seq"]
plt.loglog([r["N"] for r in seq], [r["time_s"] for r in seq], "k-o", label="sequential")
for t in a.threads:
    pts = [r for r in omp if r["threads"] == t]
    plt.loglog([r["N"] for r in pts], [r["time_s"] for r in pts], marker="o", label=f"OpenMP {t}T")
plt.xlabel("Records (N)"); plt.ylabel("Analysis time (s)"); plt.grid(alpha=.3, which="both"); plt.legend()
plt.tight_layout(); plt.savefig("time_vs_size.png", dpi=150)
print("wrote results.csv + 3 PNG graphs")
