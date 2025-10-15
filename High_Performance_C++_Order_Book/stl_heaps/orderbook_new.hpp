#pragma once
#include <cstdint>
#include <vector>
#include <map>
#include <queue>
#include <unordered_map>
#include <stdexcept>
#include <algorithm>


using id_t = uint64_t;
using price_t = uint32_t;
using qty_t = uint32_t;

enum class Side : uint8_t { 
    Buy = 0, 
    Sell = 1 
};

struct Order {
    id_t id;
    price_t price;
    qty_t qty;
    Side side;
    bool active;
    Order() : id(0), price(0), qty(0), side(Side::Buy), active(false) {}
};

struct PriceLevel {
    qty_t totalQty = 0;
    size_t orderCount = 0;
};

class OrderBookNew {
public:
    explicit OrderBookNew(size_t maxOrders = 1'000'000) {
        // id2price.reserve(maxOrders); // avoid rehash
        id2meta.reserve(maxOrders); // avoid rehash
    }

    bool newOrder(const Order& o) {
        if (o.qty == 0) {
            return false;
        }
        // if (id2price.count(o.id)) {
        //     return false;
        // }
        if (id2meta.count(o.id)) {
            return false;
        }

        auto& lvl = levels[o.price];
        lvl.totalQty += o.qty;
        lvl.orderCount += 1;
        // id2price[o.id] = o.price;
        id2meta.emplace(o.id, std::make_pair(o.price, o.qty));

        if (o.side == Side::Buy) {
            bidHeap.push(o.price);
        }
        else {
            askHeap.push(o.price);
        }

        return true;
    }

    bool amendOrder(id_t id, qty_t newQty) {
        // auto it = id2price.find(id);
        auto it = id2meta.find(id);
        // if (it == id2price.end()){
        //     return false;
        // }
        if (it == id2meta.end()){
            return false;
        }

        price_t price = it->second.first;
        qty_t oldQ = it->second.second;

        auto lvlIt = levels.find(price);

        if (lvlIt == levels.end()) {
            return false;
        }
        if (newQty > oldQ) {
            lvlIt->second.totalQty += (newQty - oldQ);
        } 
        else {
            lvlIt->second.totalQty -= (oldQ - newQty);
        }
        if (newQty == 0) {
            return deleteOrder(id);
        }
        return true;
    }

    bool deleteOrder(id_t id) {
        // auto it = id2price.find(id);
        auto it = id2meta.find(id);
        // if (it == id2price.end()) {
        //     return false;
        // }
        if (it == id2meta.end()) {
            return false;
        }

        price_t p = it->second.first;
        qty_t q = it->second.second;

        auto lvlIt = levels.find(p);
        if (lvlIt != levels.end()) {
            if (lvlIt->second.totalQty > 0) {
                lvlIt->second.totalQty -= q;
            }
            else {
                lvlIt->second.totalQty = 0;
            }
            if (lvlIt->second.orderCount > 0) {
                lvlIt->second.orderCount--;
            }
        }

        // id2price.erase(it);
        id2meta.erase(it);
        return true;
    }

    price_t bestBid() {
        while (!bidHeap.empty()) {
            price_t p = bidHeap.top();
            auto it = levels.find(p);
            if (it != levels.end() && it->second.orderCount > 0) {
                return p;
            }
            bidHeap.pop();
        }
        return 0;
    }

    price_t bestAsk() {
        while (!askHeap.empty()) {
            price_t p = askHeap.top();
            auto it = levels.find(p);
            if (it != levels.end() && it->second.orderCount > 0) {
                return p;
            }
            askHeap.pop();
        }
        return UINT32_MAX;
    }

    size_t totalOrders() const { 
        // return id2price.size();
        return id2meta.size();
    }

    void clear() {
        // id2price.clear();
        id2meta.clear();
        levels.clear();
        bidHeap = {};
        askHeap = {};
    }

private:
    // std::unordered_map<id_t, price_t> id2price;
    std::unordered_map<id_t, std::pair<price_t, qty_t>> id2meta; // 
    std::map<price_t, PriceLevel> levels;        // O(log M)
    std::priority_queue<price_t> bidHeap;        // max-heap
    std::priority_queue<
        price_t, std::vector<price_t>, std::greater<price_t>> askHeap; // min-heap
};
