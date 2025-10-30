#include <iostream>
#include "OrderBook.hpp"
#include <vector>
#include <random>
#include <chrono>
const int NUM_ORDERS = 200000;   // 测试规模，可改
const int NUM_LOOKUPS = 100000;  // 测试规模，可改

int main() {
    auto startLookup = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < NUM_LOOKUPS; i++) {
        auto& id = orderIds[rng() % NUM_ORDERS];
        auto it = ob.orderLookup.find(id);
    }
    auto endLookup = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> lookupTime = endLookup - startLookup;
    std::cout << "Lookup: " << lookupTime.count() << " s\n";
    return 0;
}