#pragma once
#include <vector>
#include <unordered_map>
#include <stdexcept>
#include <algorithm>


using id_t = int;
using price_t = int;
using qty_t = int;
using idx_t = int;

enum class Side : int {
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

struct PriceLevelSummary {
    price_t price;
    qty_t totalQty;
    size_t orderCount;
    PriceLevelSummary(): price(0), totalQty(0), orderCount(0) {}
};

struct PriceLevel {
    std::vector<Order> orders;
    std::vector<idx_t> free_list;
    qty_t totalVolume;
    size_t activeCount;
    PriceLevel(): totalVolume(0), activeCount(0) {}
    void reserve(size_t n);
    idx_t allocSlot();
    void freeSlot(idx_t i);
};

void PriceLevel::reserve(size_t n) { 
    orders.reserve(n); 
}

idx_t PriceLevel::allocSlot() {
    if (!free_list.empty()) {
        idx_t i = free_list.back();
        free_list.pop_back();
        return i;
    } 
    else {
        idx_t i = static_cast<idx_t>(orders.size());
        orders.emplace_back();
        return i;
    }
}

void PriceLevel::freeSlot(idx_t i) {
    if (i < orders.size()) {
        orders[i].active = false;
        free_list.push_back(i);
    }
}

class OrderBook {
public:
    struct Meta {
        price_t price;
        idx_t idx;
        Side side;
    };

    OrderBook(price_t min_tick, price_t max_tick)
        : minTick(min_tick), maxTick(max_tick)
    {
        if (maxTick < minTick) {
            throw std::runtime_error("invalid tick range");
        }
        nLevels = static_cast<size_t>(maxTick - minTick + 1);
        levels.resize(nLevels);
    }

    bool newOrder(const Order& o);

    bool amendOrder(id_t id, qty_t newQty);

    bool deleteOrder(id_t id);

    PriceLevelSummary topOfBook(Side s);

    size_t orderCount(price_t p);

    qty_t totalVolume(price_t p);

    size_t totalOrders();

    void clear();

private:
    price_t minTick, maxTick;
    size_t nLevels;
    std::vector<PriceLevel> levels;
    std::unordered_map<id_t, Meta> idMap;
    size_t bestBidIdx = 0;
    size_t bestAskIdx = 0;

    bool inRange(price_t p);

    size_t idxForPrice(price_t p);

    price_t idxToPrice(size_t i);

    void updateBestOnInsert(price_t p, Side s);

    void updateBestOnDelete(price_t p, Side s);
};

bool OrderBook::newOrder(const Order& o) {
    if (o.qty == 0 || !inRange(o.price)) {
        return false;
    }
    if (idMap.count(o.id)) {
        return false;
    }
    auto &pl = levels[idxForPrice(o.price)];
    idx_t id = pl.allocSlot();

    pl.orders[id] = o;
    pl.orders[id].active = true;
    pl.totalVolume += o.qty;
    pl.activeCount++;

    idMap.emplace(o.id, Meta{ o.price, id, o.side });
    updateBestOnInsert(o.price, o.side);
    return true;
}

bool OrderBook::amendOrder(id_t id, qty_t newQty) {
    auto it = idMap.find(id);
    if (it == idMap.end()) {
        return false;
    }

    Meta e = it->second;
    auto &pl = levels[idxForPrice(e.price)];
    if (e.idx >= pl.orders.size() || !pl.orders[e.idx].active) {
        return false;
    }

    auto &ord = pl.orders[e.idx];
    if (newQty == 0) {
        return deleteOrder(id);
    }
    if (newQty > ord.qty) {
        pl.totalVolume += (newQty - ord.qty);
    } 
    else {
        pl.totalVolume -= (ord.qty - newQty);
    }
    ord.qty = newQty;
    return true;
}

bool OrderBook::deleteOrder(id_t id) {
    auto it = idMap.find(id);
    if (it == idMap.end()) {
        return false;
    }

    Meta e = it->second;
    auto &pl = levels[idxForPrice(e.price)];
    if (e.idx >= pl.orders.size()) {
        return false;
    }

    auto &ord = pl.orders[e.idx];
    if (!ord.active) {
        return false;
    }

    pl.totalVolume -= ord.qty;
    pl.activeCount--;
    pl.freeSlot(e.idx);
    idMap.erase(it);
    updateBestOnDelete(e.price, e.side);
    return true;
}

PriceLevelSummary OrderBook::topOfBook(Side s){
    PriceLevelSummary ret;
    if (s == Side::Buy) {
        for (size_t i = bestBidIdx + 1; i-- > 0;) {
            if (levels[i].activeCount) {
                ret.price = idxToPrice(i);
                ret.totalQty = levels[i].totalVolume;
                ret.orderCount = levels[i].activeCount;
                return ret;
            }
            if (i == 0){
                break;
            }
        }
    } 
    else {
        for (size_t i = bestAskIdx; i < nLevels; ++i) {
            if (levels[i].activeCount) {
                ret.price = idxToPrice(i);
                ret.totalQty = levels[i].totalVolume;
                ret.orderCount = levels[i].activeCount;
                return ret;
            }
        }
    }
    return ret;
}

size_t OrderBook::orderCount(price_t p) {
        return inRange(p) ? levels[idxForPrice(p)].activeCount : 0;
}

qty_t OrderBook::totalVolume(price_t p) {
        return inRange(p) ? levels[idxForPrice(p)].totalVolume : 0;
}

size_t OrderBook::totalOrders() {
    return idMap.size();
}

void OrderBook::clear() {
    for (auto &pl : levels) {
        pl.orders.clear(); 
        pl.free_list.clear();
        pl.totalVolume = 0; 
        pl.activeCount = 0;
    }
    idMap.clear();
    bestBidIdx = 0; 
    bestAskIdx = 0;
}

bool OrderBook::inRange(price_t p) { 
    bool result = p >= minTick && p <= maxTick; 
    return result;
}

size_t OrderBook::idxForPrice(price_t p) { 
    return (size_t)(p - minTick); 
}

price_t OrderBook::idxToPrice(size_t i) {
    return (price_t)(i + minTick);
}

void OrderBook::updateBestOnInsert(price_t p, Side s) {
    size_t idx = idxForPrice(p);
    if (s == Side::Buy) {
        bestBidIdx = std::max(bestBidIdx, idx);
    } 
    else if (levels[bestAskIdx].activeCount == 0 || idx < bestAskIdx) {
        bestAskIdx = idx;
    }
}

void OrderBook::updateBestOnDelete(price_t p, Side s) {
    size_t idx = idxForPrice(p);
    if (s == Side::Buy && levels[idx].activeCount == 0 && idx == bestBidIdx){
        while (bestBidIdx > 0 && levels[bestBidIdx].activeCount == 0){
            --bestBidIdx;
        }
    }
    else if (s == Side::Sell && levels[idx].activeCount == 0 && idx == bestAskIdx){
        while (bestAskIdx + 1 < nLevels && levels[bestAskIdx].activeCount == 0){
            ++bestAskIdx;
        }
    }
}