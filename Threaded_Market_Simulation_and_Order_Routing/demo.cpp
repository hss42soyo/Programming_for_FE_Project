#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <chrono>
#include <fstream>
#include <atomic>

using namespace std;
using namespace std::chrono;

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
    while (running) {
        this_thread::sleep_for(milliseconds(1)); // Simulate feed rate

        PriceUpdate update{100.0 + rand() % 10, steady_clock::now()};
        {
            lock_guard<mutex> lock(priceMutex);
            priceQueue.push(update);
        }
        priceCV.notify_one();
    }
}

// ---------- Strategy Engine ----------
void strategyEngine(double threshold = 2.0) {
    while (running) {
        unique_lock<mutex> lock(priceMutex);
        priceCV.wait(lock, [] { return !priceQueue.empty() || !running; });

        if (!running) break;

        PriceUpdate update = priceQueue.front();
        priceQueue.pop();
        lock.unlock();

        static double lastPrice = update.price;
        double delta = update.price - lastPrice;

        if (abs(delta) > threshold) {
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
    while (running) {
        unique_lock<mutex> lock(orderMutex);
        orderCV.wait(lock, [] { return !orderQueue.empty() || !running; });

        if (!running) break;

        Order order = orderQueue.front();
        orderQueue.pop();
        lock.unlock();

        auto latency = duration_cast<microseconds>(order.timestamp.time_since_epoch()).count();
        logFile << order.side << "," << order.price << "," << latency << "\n";
    }
}

// ---------- Main ----------
int main() {
    ofstream logFile("orders.csv");
    logFile << "Side,Price,Latency\n";

    thread feedThread(marketDataFeed);
    thread strategyThread(strategyEngine, 2.0);
    thread routerThread(orderRouter, ref(logFile));

    this_thread::sleep_for(seconds(5)); // Run simulation
    running = false;

    priceCV.notify_all();
    orderCV.notify_all();

    feedThread.join();
    strategyThread.join();
    routerThread.join();

    logFile.close();
    cout << "Simulation complete. Results saved to orders.csv\n";
    return 0;
}