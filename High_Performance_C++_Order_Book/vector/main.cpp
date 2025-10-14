#include "orderbook.hpp"
#include <chrono>

using clk = chrono::high_resolution_clock;

static std::mt19937_64 rng(123456789);

void unit_tests() {
    cout << "==== UNIT TEST ====" << endl;
    OrderBook ob(0, 1000);
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
    cout << "Inserted: " << ob.totalOrders() << " orders\n";

    // Amend + Delete
    for (int i = 1; i <= 100; ++i) ob.amendOrder(i, (rng() % 200));
    for (int i = 1; i <= 100; ++i) ob.deleteOrder(i);

    cout << "Remaining after delete: " << ob.totalOrders() << "\n";
    auto tb = ob.topOfBook(Side::Buy);
    auto ta = ob.topOfBook(Side::Sell);
    cout << "Top Bid = " << tb.price << " qty=" << tb.totalQty << " | Top Ask = " 
         << ta.price << " qty=" << ta.totalQty << "\n";
}

void benchmark_run(size_t N = 1'000'000) {
    cout << "\n==== BENCHMARK (" << N << " events) ====\n";
    OrderBook ob(0, 5000, 8);
    vector<id_t> live;
    live.reserve(1 << 20);

    uniform_real_distribution<double> ud(0.0, 1.0);
    uniform_int_distribution<int> qty(1, 500);
    uniform_int_distribution<int> price(0, 5000);
    uniform_int_distribution<int> side(0, 1);

    id_t nextId = 1;
    auto t0 = clk::now();

    for (size_t i = 0; i < (int)(0.6 * N); ++i) {
        // new
        Order o;
        o.id = nextId++;
        o.price = (price_t)price(rng);
        o.qty = (qty_t)qty(rng);
        o.side = side(rng) ? Side::Buy : Side::Sell;
        o.active = true;
        if (ob.newOrder(o)){
            live.push_back(o.id);
        }
    }
    auto t_new = clk::now();

    for (size_t i = 0; i < (int)(0.2 * N); ++i) {
        // amend
        if (!live.empty()) {
            ob.amendOrder(live[rng()%live.size()], qty(rng));
        }
    }
    auto t_amend = clk::now();

    for (size_t i = 0; i < (int)(0.2 * N); ++i) {
        // delete
        if (!live.empty()) {
            size_t pos = rng()%live.size();
            ob.deleteOrder(live[pos]);
            swap(live[pos], live.back());
            live.pop_back();
        }
    }
    auto t_delete = clk::now();

    auto tb = ob.topOfBook(Side::Buy);
    auto ta = ob.topOfBook(Side::Sell);
    auto t_top = clk::now();
    cout << "Top Bid: " << tb.price << " qty=" << tb.totalQty << " | Top Ask: " 
         << ta.price << " qty=" << ta.totalQty << "\n";

    auto t1 = clk::now();
    double sec = chrono::duration<double>(t1 - t0).count();
    cout << "Total time: " << sec << " s\n";
    cout << "Avg time per event: " << (1e9 * sec / N) << " ns\n";
    cout << "Final orders: " << ob.totalOrders() << "\n";
    sec = chrono::duration<double>(t_new - t0).count();
    cout << "Insert Rate: " << (int)(0.6 * N / sec) << " ops/s\n";
    sec = chrono::duration<double>(t_amend - t_new).count();
    cout << "Amend Rate: " << (int)(0.2 * N / sec) << " ops/s\n";
    sec = chrono::duration<double>(t_delete - t_amend).count();
    cout << "Delete Rate: " << (int)(0.2 * N / sec) << " ops/s\n";
    sec = chrono::duration<double>(t_top - t_delete).count();
    cout << "Top of Book latency: " << (1e9 * sec) << " ns\n";
    ob.clear();
    cout << "After clear, orders: " << ob.totalOrders() << "\n";

    cout << "\n==== BENCHMARK LATENCY/OPS (Median Min Max 90th 99th) ====\n";
    vector<double> latencies;
    latencies.reserve(N);
    latencies.clear();
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
        latencies.push_back(chrono::duration<double, std::nano>(t1 - t0).count());
    }
    sort(latencies.begin(), latencies.end());
    cout << "Insert: ";
    cout << "Median: " << latencies[latencies.size()/2] << " " 
         << "Min: " << latencies[0] << " " 
         << "Max: " << latencies.back() << " "
         << "90th: " << latencies[(int)(0.9 * latencies.size())] << " "
         << "99th: " << latencies[(int)(0.99 * latencies.size())] << " ns\n";

    latencies.clear();
    for (size_t i = 0; i < N; ++i) {
        if (live.empty()) break;
        size_t pos = rng()%live.size();
        auto t0 = clk::now();
        ob.amendOrder(live[pos], qty(rng));
        auto t1 = clk::now();
        latencies.push_back(chrono::duration<double, std::nano>(t1 - t0).count());
    }
    sort(latencies.begin(), latencies.end());
    cout << "Amend: ";
    cout << "Median: " << latencies[latencies.size()/2] << " " 
         << "Min: " << latencies[0] << " " 
         << "Max: " << latencies.back() << " "
         << "90th: " << latencies[(int)(0.9 * latencies.size())] << " "
         << "99th: " << latencies[(int)(0.99 * latencies.size())] << " ns\n";

    latencies.clear();
    for (size_t i = 0; i < N; ++i) {
        if (live.empty()) break;
        size_t pos = rng()%live.size();
        auto t0 = clk::now();
        ob.deleteOrder(live[pos]);
        swap(live[pos], live.back());
        live.pop_back();
        auto t1 = clk::now();
        latencies.push_back(chrono::duration<double, std::nano>(t1 - t0).count());
    }
    sort(latencies.begin(), latencies.end());
    cout << "Delete: ";
    cout << "Median: " << latencies[latencies.size()/2] << " " 
         << "Min: " << latencies[0] << " " 
         << "Max: " << latencies.back() << " "
         << "90th: " << latencies[(int)(0.9 * latencies.size())] << " "
         << "99th: " << latencies[(int)(0.99 * latencies.size())] << " ns\n";
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    unit_tests();
    benchmark_run(2'000'000); // 可改小或改大
    return 0;
}
