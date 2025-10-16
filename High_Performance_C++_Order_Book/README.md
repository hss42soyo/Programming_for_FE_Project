# High Performance C++ Order Book
In this project, we tried three methods to implement an order book in C++: vector without tunning, vector with map, and heaps. Their performance is compared in the following table:

| Method | Insert Rate (Mops/s) | Amend/Delete Rate (Mops/s) | Top-of-Book Latency (ns) |
| --- | --- | --- | --- |
| Baseline Vector |  |  | |
| HashMap + std::map |  |  | |
| STL + Heaps |  |  | |

To compare the performance of different methods, we drew the histogram of three method respectively and is shown as below:

Optimzing methods:
| Operation   | Original / with_map (fixed buckets) | with_heaps (heap + map)| Notes |
|--------|-----------------------|-----------|-------------------|
| newOrder    | `unordered_map` duplicate check O(1)* + **indexed write O(1)** | `unordered_map` O(1)* + **`std::map` insert/update O(log M)** + **heap push O(log M)** | For dense price grids, fixed buckets avoid map/heap and are more cache-friendly.               |
| amendOrder  | **O(1)** (direct locate via price→idx)                         | *Originally missing* totalQty update (should adjust by Δqty)      | Fixed buckets can precisely locate and update volume.                                          |
| deleteOrder | **O(1)** (free slot, update vol/count)                         | `unordered_map` O(1)* + **`std::map` O(log M)**; heap **lazy cleanup** | Heaps may keep stale prices; removed lazily during queries.                                    |
| topOfBook   | **O(1) ~ few steps** (maintain bestBid/Ask indices)            | **Amortized O(log M)** (keep `pop` until finding a valid level)   | Heap approach avoids maintaining best indices but shifts cleanup cost to query time.           |

Environment: