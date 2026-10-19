#!/usr/bin/env python3
"""Correctness check: parallel output must match sequential output.
Usage: python3 compare.py [N=1000000] [threads: 1 2 4 8]
Exact match required: bucket counts, top-K indices & scores.
Tolerance (1e-8 relative): mean score and total expected loss, because a
parallel sum adds floating-point numbers in a different order."""
import os, subprocess, sys

def find_binary(name):
    env_dir = os.environ.get("RISK_BIN_DIR")
    if env_dir:
        p = os.path.join(env_dir, name)
        if os.path.exists(p):
            return p

    script_dir = os.path.dirname(os.path.abspath(__file__))
    repo_root = os.path.abspath(os.path.join(script_dir, ".."))

    candidates = [
        os.path.join(repo_root, "bin", name),
        os.path.join(os.getcwd(), "bin", name),
        os.path.join(os.getcwd(), name),
        os.path.join(repo_root, name),
        name
    ]
    for c in candidates:
        if os.path.isfile(c) and os.access(c, os.X_OK):
            return c
    return os.path.join(repo_root, "bin", name)

def run(cmd):
    try:
        out = subprocess.run(cmd, capture_output=True, text=True, check=True).stdout
        return [l for l in out.splitlines() if l.startswith("RESULT")]
    except subprocess.CalledProcessError as e:
        if e.stderr:
            sys.stderr.write(e.stderr)
        else:
            sys.stderr.write(f"Error executing command: {' '.join(cmd)}\n")
        sys.exit(e.returncode)

def kv(line):
    return dict(p.split("=") for p in line.split()[1:] if "=" in p)

def main():
    N = sys.argv[1] if len(sys.argv) > 1 else "1000000"
    threads = sys.argv[2:] or ["1", "2", "4", "8"]

    try:
        if int(N) <= 0:
            sys.stderr.write(f"Error: invalid number of records N '{N}'. N must be a positive integer (> 0).\n")
            sys.exit(1)
    except ValueError:
        sys.stderr.write(f"Error: invalid number of records N '{N}'. N must be a positive integer (> 0).\n")
        sys.exit(1)

    for t in threads:
        try:
            if int(t) <= 0:
                sys.stderr.write(f"Error: invalid number of threads '{t}'. Threads must be a positive integer (> 0).\n")
                sys.exit(1)
        except ValueError:
            sys.stderr.write(f"Error: invalid number of threads '{t}'. Threads must be a positive integer (> 0).\n")
            sys.exit(1)

    seq_bin = find_binary("credit_risk_seq")
    omp_bin = find_binary("credit_risk_omp")

    ref = run([seq_bin, N])
    ok_all = True
    for t in threads:
        par = run([omp_bin, N, t])
        ok = True
        a, b = kv(ref[0]), kv(par[0])
        for key in ("N", "low", "medium", "high"):
            if a[key] != b[key]:
                ok = False; print(f"  MISMATCH {key}: seq={a[key]} omp={b[key]}")
        a, b = kv(ref[1]), kv(par[1])
        for key in ("mean_score", "total_expected_loss"):
            x, y = float(a[key]), float(b[key])
            if abs(x - y) > 1e-8 * max(1.0, abs(x)):
                ok = False; print(f"  MISMATCH {key}: seq={x} omp={y}")
        if ref[2:] != par[2:]:
            ok = False; print("  MISMATCH in top-K list")
        print(f"N={N} threads={t}: {'PASS' if ok else 'FAIL'}")
        ok_all &= ok
    sys.exit(0 if ok_all else 1)

main()
