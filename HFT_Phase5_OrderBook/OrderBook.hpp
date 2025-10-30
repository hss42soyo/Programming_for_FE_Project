#include <iostream>
#include <map>
#include <unordered_map>
#include <string>
#include <chrono>
#include <vector>
#include <atomic>


struct Order {
    std::string id;  // String-based order ID
    double price;
    int quantity;
    bool isBuy;
};

class OrderBook {
private:
    std::map<double, std::unordered_map<std::string, Order>> orderLevels;
    std::unordered_map<std::string, Order> orderLookup;

public:

    // for testing purpose
    bool findOrder(const std::string& id) const {
        return orderLookup.find(id) != orderLookup.end();
    }

    void addOrder(const std::string& id, double price, int quantity, bool isBuy) {
        Order order = {id, price, quantity, isBuy};
        orderLevels[price][id] = order;
        orderLookup[id] = order;
    }
    void modifyOrder(const std::string& id, double newPrice, int newQuantity) {
        if (orderLookup.find(id) != orderLookup.end()) {
            Order oldOrder = orderLookup[id];
            orderLevels[oldOrder.price].erase(id);  
            addOrder(id, newPrice, newQuantity, oldOrder.isBuy);
        }
        else {
            std::cerr << "Order ID not found: " << id << std::endl;
        }
    }

    void deleteOrder(const std::string& id) {
        if (orderLookup.find(id) != orderLookup.end()) {
            Order order = orderLookup[id];
            orderLevels[order.price].erase(id);  
            orderLookup.erase(id);
        }
        else {
            std::cerr << "Order ID not found: " << id << std::endl;
        }
    }

    void benchmark() {
        auto start = std::chrono::high_resolution_clock::now();
        
        addOrder("ORD001", 50.10, 100, true);
        
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;
        
        std::cout << "Execution time: " << elapsed.count() << " seconds" << std::endl;
    }
};


// ================================
class OptimizedOrderBook {
private:
    std::vector<Order> orderPool;  // 预分配订单内存池
    std::unordered_map<std::string, size_t> orderMap;  // 映射ID到orderPool索引
    std::atomic<int> orderCount{0};  // 无锁计数器，统计总订单数

public:
    OptimizedOrderBook(size_t reserveSize = 100000) {
        orderPool.reserve(reserveSize);
        orderMap.reserve(reserveSize);
    }

    void addOrder(const std::string& id, double price, int quantity, bool isBuy) {
        Order order = {id, price, quantity, isBuy};
        orderPool.push_back(order);
        orderMap[id] = orderPool.size() - 1;
        orderCount.fetch_add(1, std::memory_order_relaxed);
    }

    void modifyOrder(const std::string& id, double newPrice, int newQuantity) {
        if (orderMap.find(id) != orderMap.end()) {
            size_t index = orderMap[id];
            orderPool[index].price = newPrice;
            orderPool[index].quantity = newQuantity;
        }
    }

    void deleteOrder(const std::string& id) {
        if (orderMap.find(id) != orderMap.end()) {
            size_t index = orderMap[id];
            orderPool[index] = orderPool.back();
            orderMap[orderPool[index].id] = index;
            orderPool.pop_back();
            orderMap.erase(id);
            orderCount.fetch_sub(1, std::memory_order_relaxed);
        }
    }


    bool findOrder(const std::string& id) const {
        return orderMap.find(id) != orderMap.end();
    }

    void processOrders() {
        size_t n = orderPool.size();
        // 循环展开优化
        for (size_t i = 0; i < n; i += 2) {
            handleOrder(orderPool[i]);
            if (i + 1 < n) handleOrder(orderPool[i + 1]);
        }
    }

private:
    void handleOrder(const Order& order) {
        // 模拟订单处理逻辑（此处为占位）
        volatile double dummy = order.price * order.quantity;
        (void)dummy;
    }
};