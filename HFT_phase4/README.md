# High-Frequency Trading Latency Experiments

## Overview
This project benchmarks different design choices in a simplified high-frequency trading (HFT) system.  
The experiments focus on how software-level engineering decisions affect latency and performance.

## Experiment Variables
| Category | Variation | Objective |
|-----------|------------|------------|
| Smart vs Raw Pointers | `unique_ptr` vs raw pointers | Memory safety vs overhead |
| Memory Alignment | `alignas(64)` vs default | Cache line alignment and cache misses |
| Custom Allocator | Memory pool vs `new/delete` | Allocation/deallocation speed |
| Container Layout | `flat vector` vs `map/multimap` | Access pattern and locality |

## Directory Layout
```
HFT_Phase4/
 ├── exp_pointers/     → smart vs raw pointers
 ├── exp_alignment/    → alignas(64) cache experiments
 ├── exp_allocator/    → memory pool allocator
 └── exp_container/    → flat vs map container structure
```

Each subproject is self-contained and can be built independently.

## Build Instructions
1. Open the desired experiment folder (e.g., `exp_pointers`) in Visual Studio or use CMake:
   ```bash
   cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
   cmake --build build --config Release
   ```
2. Run the compiled executable:
   ```bash
   ./build/Release/hft_phase4_app.exe
   ```
3. The output CSV (`results.csv`) will be written to the working directory.

## System Architecture
The architecture simulates a typical low-latency trading stack:

```
┌──────────────────────────────┐
│        MarketDataFeed        │
│ Generates synthetic ticks    │
└────────────┬─────────────────┘
             │
┌────────────▼────────────┐
│       OrderManager       │
│ Tracks order lifecycle   │
└────────────┬─────────────┘
             │
┌────────────▼────────────┐
│        OrderBook         │
│ Stores bid/ask levels    │
│ (map / vector structure) │
└────────────┬─────────────┘
             │
┌────────────▼────────────┐
│     MatchingEngine       │
│ Matches buy/sell orders  │
└────────────┬─────────────┘
             │
┌────────────▼────────────┐
│       TradeLogger        │
│ Records matched trades   │
└──────────────────────────┘
```

## Results Summary
The results show measurable latency differences depending on design choice:

| Experiment | Observation |
|-------------|--------------|
| Smart vs Raw Pointers | Smart pointers have slightly higher mean latency but provide safe memory management. |
| Memory Alignment | `alignas(64)` marginally improves cache efficiency under high load. |
| Custom Allocator | Memory pool reduces allocation latency by about 50%. |
| Container Layout | Flat arrays are slower in insertion-heavy workloads but may benefit from cache locality in reads. |
