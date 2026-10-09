# LLM Usage Log

This page logs how we used AI tools while developing our parallel Credit Risk Analysis project. Note that our benchmark programs generate synthetic customer records in memory to avoid disk I/O bottlenecks. A sample CSV exists, but the main benchmark computation does not read it.

<div style="overflow-x: auto; margin-bottom: 24px;">
  <table style="width: 100%; min-width: 800px; border-collapse: collapse; text-align: left; line-height: 1.5;">
    <thead>
      <tr style="border-bottom: 2px solid #555;">
        <th style="padding: 10px; width: 6%;">Sr. No.</th>
        <th style="padding: 10px; width: 14%;">Purpose</th>
        <th style="padding: 10px; width: 30%;">Prompt Used</th>
        <th style="padding: 10px; width: 25%;">How the Response Was Used</th>
        <th style="padding: 10px; width: 25%;">Modification / Verification</th>
      </tr>
    </thead>
    <tbody>
      <tr style="border-bottom: 1px solid #444;">
        <td style="padding: 10px;">1</td>
        <td style="padding: 10px;">Understanding the Risk Calculation</td>
        <td style="padding: 10px;">"How should we calculate a simple credit risk score for each customer using income, existing debt, loan amount, credit utilization, late payments, and previous default?"</td>
        <td style="padding: 10px;">We used the suggestions to decide how the financial factors could contribute to the customer's risk score.<br><br><em>Note: The implemented formula uses income, debt, utilization, loan amount, account age, and late payments (previous default is not used). Thresholds are Low Risk (&lt; 35), Medium (35 to &lt; 60), High (&gt;= 60).</em></td>
        <td style="padding: 10px;"><strong>[Needs Confirmation]</strong> We still need to record the manual check of the scoring rules against sample customer records.</td>
      </tr>
      <tr style="border-bottom: 1px solid #444;">
        <td style="padding: 10px;">2</td>
        <td style="padding: 10px;">Preparing the Dataset</td>
        <td style="padding: 10px;">"What columns should we include in a synthetic CSV dataset for our Credit Risk Analysis project, and what values should each column contain?"</td>
        <td style="padding: 10px;">We used the suggestions to plan the customer fields needed for the project.</td>
        <td style="padding: 10px;">We checked the CSV columns against the customer data structure used in the code. The actual CSV columns are: <code>id</code>, <code>income</code>, <code>debt</code>, <code>utilization</code>, <code>loan_amount</code>, <code>account_age_months</code>, and <code>late_payments</code>.</td>
      </tr>
      <tr style="border-bottom: 1px solid #444;">
        <td style="padding: 10px;">3</td>
        <td style="padding: 10px;">Sequential Implementation</td>
        <td style="padding: 10px;">"Explain how to implement a sequential C++ program that generates customer records, calculates risk scores, counts risk categories, and identifies the riskiest customers."</td>
        <td style="padding: 10px;">We implemented the sequential version in <code>credit_risk_seq.cpp</code>. It generates records in memory, calculates risk scores, counts categories, and maintains a Top-K list.</td>
        <td style="padding: 10px;">Execution is confirmed. The program successfully compiles and prints execution time, category counts, mean score, total expected loss, and the Top-K riskiest customers.</td>
      </tr>
      <tr style="border-bottom: 1px solid #444;">
        <td style="padding: 10px;">4</td>
        <td style="padding: 10px;">OpenMP Parallelization</td>
        <td style="padding: 10px;">"Explain how to parallelize customer risk calculations with OpenMP and safely combine per-thread results."</td>
        <td style="padding: 10px;">We implemented the parallel version in <code>credit_risk_omp.cpp</code>. Each thread maintains local counters and a local Top-K heap, and the program merges these results after processing.</td>
        <td style="padding: 10px;">The script <code>compare.py</code> verifies that bucket counts, Top-K lists, mean scores, and total expected loss match between the sequential and OpenMP versions within a floating-point tolerance.</td>
      </tr>
      <tr style="border-bottom: 1px solid #444;">
        <td style="padding: 10px;">5</td>
        <td style="padding: 10px;">Performance Evaluation</td>
        <td style="padding: 10px;">"How can we measure execution time, speedup, and parallel efficiency for sequential and OpenMP implementations across different input sizes and thread counts?"</td>
        <td style="padding: 10px;">We used <code>benchmark.py</code> to run performance experiments and record results in <code>results.csv</code>.</td>
        <td style="padding: 10px;">Recorded benchmark results include 50 million records, with a sequential time of 0.201543 seconds and an eight-thread time of 0.047716 seconds. This corresponds to approximately 4.22x speedup.</td>
      </tr>
    </tbody>
  </table>
</div>

## Critical Evaluation of LLM Output

**Implementation decision**

The OpenMP program keeps separate result counters and a separate Top-K heap for each thread. Each thread updates its local results while processing customers. The local results are merged after the processing loop.

**Why this design is useful**

Keeping counters and heap updates local avoids repeatedly synchronizing access to the same shared result structures during customer processing.

**What we checked**

The implementation in `credit_risk_omp.cpp` and the shared structures in `risk.h` were inspected to confirm how the local results are maintained and merged.

**What still needs confirmation**

The available development history does not establish that the original critical-section suggestion came from the LLM or that the two approaches were directly benchmarked against each other.
