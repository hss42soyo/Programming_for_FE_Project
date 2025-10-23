#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <chrono>
#include <fstream>
#include <atomic>
#include <cmath>

using namespace std;
using namespace std::chrono;

// ---------- Parameters ----------
struct Config {
    int market_rate = 1000; 
    double threshold = 2.0; 
    int duration_sec = 5; 
    bool verbose = false; 
};
Config cfg;

// ---------- Data Structures ----------
struct PriceUpdate {
    double price;
    steady_clock::time_point timestamp;
};

struct Order {
    string side; // "BUY" or "SELL"
    double price;
    steady_clock::time_point timestamp;
};

// ---------- Shared Queues ----------
queue<PriceUpdate> priceQueue;
queue<Order> orderQueue;
mutex priceMutex, orderMutex;
condition_variable priceCV, orderCV;
atomic<bool> running{true};

// ---------- Market Data Feed ----------
void marketDataFeed() {
    double base = 100.0;
    int interval_us = 1'000'000 / cfg.market_rate; // Simulate feed rate

    while (running) {
        this_thread::sleep_for(microseconds(interval_us));
        PriceUpdate update{base + (rand() % 10), steady_clock::now()};
        {
            lock_guard<mutex> lock(priceMutex);
            priceQueue.push(update);
        }
        priceCV.notify_one();
    }
}

// ---------- Strategy Engine ----------
void strategyEngine(double threshold) {
    // double lastPrice = 100.0;

    while (running) {
        unique_lock<mutex> lock(priceMutex);
        priceCV.wait(lock, [] { return !priceQueue.empty() || !running; });
        if (!running && priceQueue.empty()) break;

        PriceUpdate update = priceQueue.front();
        priceQueue.pop();
        lock.unlock();

        static double lastPrice = update.price;
        double delta = update.price - lastPrice;

        if (fabs(delta) > threshold) {
            Order order{
                delta < 0 ? "BUY" : "SELL",
                update.price,
                steady_clock::now()
            };
            {
                lock_guard<mutex> olock(orderMutex);
                orderQueue.push(order);
            }
            orderCV.notify_one();
        }
        lastPrice = update.price;
    }
}

// ---------- Order Router ----------
void orderRouter(ofstream& logFile) {
    size_t orderCount = 0;
    long long totalLatency = 0; // microseconds
    while (running) {
        unique_lock<mutex> lock(orderMutex);
        orderCV.wait(lock, [] { return !orderQueue.empty() || !running; });
        if (!running && orderQueue.empty()) break;

        Order order = orderQueue.front();
        orderQueue.pop();
        lock.unlock();

        auto now = steady_clock::now();
        auto latency = duration_cast<microseconds>(now - order.timestamp).count();
        totalLatency += latency;
        orderCount++;
        logFile << order.side << "," << order.price << "," << latency << "\n";
        if (cfg.verbose)
            cout << order.side << " @" << order.price << " latency=" << latency << "us\n";
    }

    if (orderCount > 0) {
        double avgLatency = (double)totalLatency / orderCount;
        double throughput = orderCount * 1.0 / cfg.duration_sec;
        cout << "\n========== PERFORMANCE ==========\n";
        cout << "Orders/sec   : " << throughput << endl;
        cout << "Avg Latency  : " << avgLatency << " us\n";
        cout << "=================================\n";
    } else {
        cout << "\nNo orders generated.\n";
    }
}

// ---------- Main ----------
int main() {
    srand(time(nullptr));
    // cout << "Starting HFT simulation...\n";
    ofstream logFile(to_string(cfg.market_rate) + "_orders.csv");
    // ofstream logFile("orders.csv");
    logFile << "Side,Price,Latency\n";

    thread feedThread(marketDataFeed);
    thread strategyThread(strategyEngine, cfg.threshold);
    thread routerThread(orderRouter, ref(logFile));

    this_thread::sleep_for(seconds(cfg.duration_sec));
    running = false;

    priceCV.notify_all();
    orderCV.notify_all();

    feedThread.join();
    strategyThread.join();
    routerThread.join();

    logFile.close();
    cout << "Simulation complete. Results saved to " << cfg.market_rate << "_" << "orders.csv\n";
    return 0;
}
