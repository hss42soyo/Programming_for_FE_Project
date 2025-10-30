# Phase 5 - HFT OrderBook Optimization

## Step 2: Performance Analysis & Bottleneck Identification

Run on Apple M1 CPU.

Baseline

```
======== Benchmark Result ========
Insert: 0.530991 s
Modify: 0.459515 s
Delete: 0.140561 s
Total:  1.17571 s
Lookup Test:
  Total lookups: 500000
  Found: 500000
  Total lookup time: 0.412324 s
  Avg lookup latency: 824.649 ns per lookup
```

## Step 3: Advanced Optimization Techniques