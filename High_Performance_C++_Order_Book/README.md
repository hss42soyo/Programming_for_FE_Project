# High Performance C++ Order Book
In this project, we tried three methods to implement an order book in C++: vector without tunning, vector with map, and heaps. Their performance is compared in the following table:

| Method | Insert Rate (Mops/s) | Amend/Delete Rate (Mops/s) | Top-of-Book Latency (ns) |
| --- | --- | --- | --- |
| Baseline Vector |  |  | |
| HashMap + std::map |  |  | |
| STL + Heaps |  |  | |

To compare the performance of different methods, we drew the histogram of three method respectively and is shown as below:

Optimzing methods:

Environment: