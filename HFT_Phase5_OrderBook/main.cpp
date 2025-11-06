#include "OrderBook.hpp"
#include <vector>
#include <random>
#include <algorithm>
#include <iostream>
#include <cassert>
#include <fstream>

struct BenchmarkResult {
    int numOrders;
    double insertTime;
    double modifyTime;
    double deleteTime;
    double totalTime;
};

void writeCSVHeader(const std::string& filename) {
    std::ofstream file(filename, std::ios::app);
    if (file.tellp() == 0) {
        file << "TestType,NumOrders,NumLookups,Insert,Modify,Delete,Total,LookupTime\n";
    }
}

void appendToCSV(const std::string& filename, const std::string& testType, const BenchmarkResult& result, int numLookups = 0, double lookupTime = 0.0) {
    std::ofstream file(filename, std::ios::app);
    file << testType << "," << result.numOrders << "," << numLookups << ","
         << result.insertTime << "," << result.modifyTime << ","
         << result.deleteTime << "," << result.totalTime << "," << lookupTime << "\n";
}

BenchmarkResult benchmark_baseline(const int NUM_ORDERS) {
    OrderBook ob;
    std::vector<std::string> orderIds;
    orderIds.reserve(NUM_ORDERS);

    std::mt19937 rng(42);
    std::uniform_real_distribution<double> priceDist(50.0, 150.0);
    std::uniform_int_distribution<int> qtyDist(1, 1000);

    auto tic = std::chrono::high_resolution_clock::now();

    auto startInsert = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < NUM_ORDERS; i++) {
        std::string id = "ORD" + std::to_string(i);
        double price = priceDist(rng);
        int qty = qtyDist(rng);
        bool isBuy = (i % 2 == 0);
        ob.addOrder(id, price, qty, isBuy);
        orderIds.push_back(id);
    }
    auto endInsert = std::chrono::high_resolution_clock::now();

    std::shuffle(orderIds.begin(), orderIds.end(), rng);

    auto startModify = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < NUM_ORDERS / 2; i++) {
        double newPrice = priceDist(rng);
        int newQty = qtyDist(rng);
        ob.modifyOrder(orderIds[i], newPrice, newQty);
    }
    auto endModify = std::chrono::high_resolution_clock::now();

    auto startDelete = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < NUM_ORDERS / 4; i++) {
        ob.deleteOrder(orderIds[i]);
    }
    auto endDelete = std::chrono::high_resolution_clock::now();

    auto toc = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> insertTime = endInsert - startInsert;
    std::chrono::duration<double> modifyTime = endModify - startModify;
    std::chrono::duration<double> deleteTime = endDelete - startDelete;
    std::chrono::duration<double> totalTime = toc - tic;

    std::cout << "======== Benchmark Result ========\n";
    std::cout << "Insert: " << insertTime.count() << " s\n";
    std::cout << "Modify: " << modifyTime.count() << " s\n";
    std::cout << "Delete: " << deleteTime.count() << " s\n";
    std::cout << "Total:  " << totalTime.count() << " s\n";

    BenchmarkResult result{NUM_ORDERS, insertTime.count(), modifyTime.count(), deleteTime.count(), totalTime.count()};
    return result;
}

// ================= OptimizedOrderBook Benchmark =================
BenchmarkResult benchmark_optimized(const int NUM_ORDERS) {
    OptimizedOrderBook optOb(NUM_ORDERS);
    std::vector<std::string> orderIds;
    orderIds.reserve(NUM_ORDERS);

    std::mt19937 rng(42);
    std::uniform_real_distribution<double> priceDist(50.0, 150.0);
    std::uniform_int_distribution<int> qtyDist(1, 1000);

    auto startInsert = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < NUM_ORDERS; i++) {
        std::string id = "OID" + std::to_string(i);
        optOb.addOrder(id, priceDist(rng), qtyDist(rng), i % 2);
        orderIds.push_back(id);
    }
    auto endInsert = std::chrono::high_resolution_clock::now();


    std::shuffle(orderIds.begin(), orderIds.end(), rng);
    auto startModify = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < NUM_ORDERS / 2; i++) {
        double newPrice = priceDist(rng);
        int newQty = qtyDist(rng);
        optOb.modifyOrder(orderIds[i], newPrice, newQty);
    }
    auto endModify =  std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> modifyTime = endModify - startModify;

    auto startDelete = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < NUM_ORDERS / 4; i++) {
        optOb.deleteOrder(orderIds[i]);
    }
    auto endDelete = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> insertTime = endInsert - startInsert;
    std::chrono::duration<double> deleteTime = endDelete - startDelete;
    BenchmarkResult result{NUM_ORDERS, insertTime.count(), modifyTime.count(), deleteTime.count(), insertTime.count() + modifyTime.count() + deleteTime.count()};
    std::cout << "Optimized Insert: " << insertTime.count() << " s\n";
    std::cout << "Optimized Modify: " << modifyTime.count() << " s\n";
    std::cout << "Optimized Delete: " << deleteTime.count() << " s\n";

    return result;
}

// ================= Lookup Benchmark =================
double lookupBenchmark_OrderBook(int numOrders, int numLookups) {
    OrderBook ob;
    std::vector<std::string> orderIds;
    orderIds.reserve(numOrders);

    std::mt19937 rng(42);
    std::uniform_real_distribution<double> priceDist(50.0, 150.0);
    std::uniform_int_distribution<int> qtyDist(1, 1000);
    for (int i = 0; i < numOrders; i++) {
        std::string id = "ID" + std::to_string(i);
        ob.addOrder(id, priceDist(rng), qtyDist(rng), i % 2);
        orderIds.push_back(id);
    }

    std::mt19937 rng2(666);
    std::uniform_int_distribution<int> idDist(0, numOrders - 1);

    auto startLookup = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < numLookups; ++i) {
        const std::string& id = orderIds[idDist(rng2)];
        auto found = ob.findOrder(id);
        (void)found;
    }
    auto endLookup = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> lookupTime = endLookup - startLookup;

    std::cout << "[OrderBook] Lookup " << numLookups << ": " << lookupTime.count() << " s\n";
    return lookupTime.count();
}

double lookupBenchmark_OptimizedOrderBook(int numOrders, int numLookups) {
    OptimizedOrderBook optOb(numOrders);
    std::mt19937 rng(42);
    std::uniform_real_distribution<double> priceDist(50.0, 150.0);
    std::uniform_int_distribution<int> qtyDist(1, 1000);
    for (int i = 0; i < numOrders; i++) {
        std::string id = "OID" + std::to_string(i);
        optOb.addOrder(id, priceDist(rng), qtyDist(rng), i % 2);
    }

    std::mt19937 rng2(777);
    std::uniform_int_distribution<int> idDist(0, numOrders - 1);

    auto startLookup = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < numLookups; ++i) {
        std::string id = "OID" + std::to_string(idDist(rng2));
        bool found = optOb.findOrder(id);
        (void)found;
    }
    auto endLookup = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> lookupTime = endLookup - startLookup;

    std::cout << "[OptimizedOrderBook] Lookup " << numLookups << ": " << lookupTime.count() << " s\n";
    return lookupTime.count();
}

// ================= main =================
int main() {
    const std::string csvFile = "benchmark_results.csv";
    writeCSVHeader(csvFile);

    std::vector<int> orderSizes = {10000, 50000, 100000, 200000, 500000};
    std::vector<int> lookupSizes = {1000, 10000, 100000, 500000};

    for (int numOrders : orderSizes) {
        std::cout << "\n=== Benchmark for " << numOrders << " Orders ===\n";

        auto baseResult = benchmark_baseline(numOrders);
        appendToCSV(csvFile, "OrderBook_Benchmark", baseResult);

        auto optResult = benchmark_optimized(numOrders);
        appendToCSV(csvFile, "OptimizedOrderBook_Benchmark", optResult);

        for (int numLookups : lookupSizes) {
            double t1 = lookupBenchmark_OrderBook(numOrders, numLookups);
            appendToCSV(csvFile, "OrderBook_Lookup", {numOrders,0,0,0,0}, numLookups, t1);

            double t2 = lookupBenchmark_OptimizedOrderBook(numOrders, numLookups);
            appendToCSV(csvFile, "OptimizedOrderBook_Lookup", {numOrders,0,0,0,0}, numLookups, t2);
        }
    }
    return 0;
}