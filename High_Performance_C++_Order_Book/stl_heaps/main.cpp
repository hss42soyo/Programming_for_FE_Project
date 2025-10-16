#include "orderbook_new.hpp"
#include <chrono>
#include <random>
#include <iostream>
#include <fstream>

using clk = std::chrono::high_resolution_clock;

static std::mt19937_64 rng(123456789);

static void write_csv(const std::string& path, const std::vector<double>& v){
    std::ofstream ofs(path, std::ios::binary);
    for (double x : v) ofs << x << '\n';
}

void unit_tests() {
    std::cout << "==== UNIT TEST ====" << std::endl;
    OrderBookNew ob;
    const int N = 1000;

    // Insert random orders
    for (int i = 0; i < N; ++i) {
        Order o;
        o.id = i + 1;
        o.price = rng() % 1001;
        o.qty = (rng() % 100) + 1;
        o.side = (rng() % 2) ? Side::Buy : Side::Sell;
        ob.newOrder(o);
    }
    std::cout << "Inserted: " << ob.totalOrders() << " orders\n";

    // Amend + Delete
    for (int i = 1; i <= 100; ++i) ob.amendOrder(i, (rng() % 200));
    for (int i = 1; i <= 100; ++i) ob.deleteOrder(i);

    std::cout << "Remaining after delete: " << ob.totalOrders() << "\n";
    auto tb = ob.bestBid();
    auto ta = ob.bestAsk();
    std::cout << "Top Bid = " << tb << " | Top Ask = " << ta << "\n";
}

void benchmark_run(size_t N = 1'000'000) {
    std::cout << "\n==== BENCHMARK (" << N << " events) ====\n";
    OrderBookNew ob;
    std::vector<id_t> live;
    live.reserve(1 << 20);

    std::uniform_real_distribution<double> ud(0.0, 1.0);
    std::uniform_int_distribution<int> qty(1, 500);
    std::uniform_int_distribution<int> price(0, 5000);
    std::uniform_int_distribution<int> side(0, 1);

    id_t nextId = 1;

    double insertRate = 0.45;
    double amendRate = 0.35;
    double deleteRate = 0.20;

    //auto t0 = clk::now();

    long long time_insert = 0;
    for (size_t i = 0; i < (int)(insertRate * N); ++i) {
        // new
        Order o;
        o.id = nextId++;
        o.price = (price_t)price(rng);
        o.qty = (qty_t)qty(rng);
        o.side = side(rng) ? Side::Buy : Side::Sell;
        o.active = true;
        auto t0_insert = clk::now();
        if (ob.newOrder(o)) {
            auto t1_insert = clk::now();
            time_insert += std::chrono::duration_cast<std::chrono::nanoseconds>(t1_insert - t0_insert).count();
            live.push_back(o.id);
        }
    }
    std::cout << "Insert Rate: " << (1e3 * (insertRate * N) / time_insert) << " Mops/s\n";
    //auto t_new = clk::now();

    long long time_amend = 0;
    for (size_t i = 0; i < (int)(amendRate * N); ++i) {
        // amend
        if (!live.empty()) {
            auto t0_amend = clk::now();
            ob.amendOrder(live[rng()%live.size()], qty(rng));
            auto t1_amend = clk::now();
            time_amend += std::chrono::duration_cast<std::chrono::nanoseconds>(t1_amend - t0_amend).count();
        }
    }
    std::cout << "Amend Rate: " << (1e3 * (amendRate * N) / time_amend) << " Mops/s\n";
    //auto t_amend = clk::now();

    long long time_delete = 0;
    for (size_t i = 0; i < (int)(deleteRate * N); ++i) {
        // delete
        if (!live.empty()) {
            size_t pos = rng()%live.size();
            auto t0_delete = clk::now();
            ob.deleteOrder(live[pos]);
            auto t1_delete = clk::now();
            time_delete += std::chrono::duration_cast<std::chrono::nanoseconds>(t1_delete - t0_delete).count();
            std::swap(live[pos], live.back());
            live.pop_back();
        }
    }
    std::cout << "Delete Rate: " << (1e3 * (deleteRate * N) / time_delete) << " Mops/s\n";
    //auto t_delete = clk::now();

    auto t0_top = clk::now();
    auto tb = ob.bestBid();
    auto ta = ob.bestAsk();
    auto t1_top = clk::now();
    std::cout << "Top Bid: " << tb << " | Top Ask: " << ta << "\n";
    std::cout << "Top of Book latency: " << std::chrono::duration<double, std::nano>(t1_top - t0_top).count() << " ns\n";

    // auto t1 = clk::now();
    // double sec = std::chrono::duration<double>(t1 - t0).count();
    // std::cout << "Total time: " << sec << " s\n";
    // std::cout << "Avg time per event: " << (1e9 * sec / N) << " ns\n";
    // std::cout << "Final orders: " << ob.totalOrders() << "\n";
    // sec = std::chrono::duration<double>(t_new - t0).count();
    // std::cout << "Insert Rate: " << (int)(0.6 * N / sec) << " ops/s\n";
    // sec = std::chrono::duration<double>(t_amend - t_new).count();
    // std::cout << "Amend Rate: " << (int)(0.2 * N / sec) << " ops/s\n";
    // sec = std::chrono::duration<double>(t_delete - t_amend).count();
    // std::cout << "Delete Rate: " << (int)(0.2 * N / sec) << " ops/s\n";
    // sec = std::chrono::duration<double>(t_top - t_delete).count();
    // std::cout << "Top of Book latency: " << (1e9 * sec) << " ns\n";

    std::cout << "Total time: " << (time_insert + time_amend + time_delete) << " ns\n";
    std::cout << "Avg time per event: " << (double)(time_insert + time_amend + time_delete) / N << " ns\n";
    std::cout << "Operation Rate: " << (1e3 * N / (time_insert + time_amend + time_delete)) << " Mops/s\n";
    std::cout << "Final orders: " << ob.totalOrders() << "\n";

    ob.clear();
    std::cout << "After clear, orders: " << ob.totalOrders() << "\n";

    std::cout << "\n==== BENCHMARK LATENCY/OPS (Median Min Max 90th 99th) ====\n";
    std::vector<double> latencies;
    latencies.reserve(N);
    nextId = 1;
    for (size_t i = 0; i < N; ++i) {
        Order o;
        o.id = nextId++;
        o.price = (price_t)price(rng);
        o.qty = (qty_t)qty(rng);
        o.side = side(rng) ? Side::Buy : Side::Sell;
        o.active = true;

        auto t0 = clk::now();
        ob.newOrder(o);
        auto t1 = clk::now();
        latencies.push_back(std::chrono::duration<double, std::nano>(t1 - t0).count());
    }
    write_csv("latency_insert_heaps.csv", latencies);

    std::cout << "Insert: ";
    std::sort(latencies.begin(), latencies.end());
    std::cout << "Median: " << latencies[latencies.size()/2] << " " 
         << "Min: " << latencies[0] << " " 
         << "Max: " << latencies.back() << " "
         << "90th: " << latencies[(int)(0.9 * latencies.size())] << " "
         << "99th: " << latencies[(int)(0.99 * latencies.size())] << " ns\n";

    latencies.clear();
    for (size_t i = 0; i < N; ++i) {
        id_t id = (rng() % (nextId - 1)) + 1;
        qty_t newQty = (qty_t)qty(rng);

        auto t0 = clk::now();
        ob.amendOrder(id, newQty);
        auto t1 = clk::now();
        latencies.push_back(std::chrono::duration<double, std::nano>(t1 - t0).count());
    }
    write_csv("latency_amend_heaps.csv", latencies);

    std::cout << "Amend: ";
    std::sort(latencies.begin(), latencies.end());
    std::cout << "Median: " << latencies[latencies.size()/2] << " " 
         << "Min: " << latencies[0] << " " 
         << "Max: " << latencies.back() << " "
         << "90th: " << latencies[(int)(0.9 * latencies.size())] << " "
         << "99th: " << latencies[(int)(0.99 * latencies.size())] << " ns\n";
    
    latencies.clear();
    for (size_t i = 0; i < N; ++i) {
        id_t id = (rng() % (nextId - 1)) + 1;

        auto t0 = clk::now();
        ob.deleteOrder(id);
        auto t1 = clk::now();
        latencies.push_back(std::chrono::duration<double, std::nano>(t1 - t0).count());
    }
    write_csv("latency_delete_heaps.csv", latencies);

    std::cout << "Delete: ";
    std::sort(latencies.begin(), latencies.end());
    std::cout << "Median: " << latencies[latencies.size()/2] << " " 
         << "Min: " << latencies[0] << " " 
         << "Max: " << latencies.back() << " "
         << "90th: " << latencies[(int)(0.9 * latencies.size())] << " "
         << "99th: " << latencies[(int)(0.99 * latencies.size())] << " ns\n";


}

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    //unit_tests();
    benchmark_run(1'000'000);
    return 0;
}
