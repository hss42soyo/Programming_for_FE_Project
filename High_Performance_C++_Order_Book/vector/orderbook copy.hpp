#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <stdexcept>
#include <algorithm>
#include <limits>
#include <iostream>


using id_t = uint64_t;
using price_t = int32_t;
using qty_t = uint32_t;
using idx_t = uint32_t;

enum class Side : uint8_t { 
    Buy = 0, 
    Sell = 1 
};

struct Order {
    alignas(64) id_t id;
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
    alignas(64) std::vector<Order> orders;
    std::vector<idx_t> free_list;
    qty_t totalVolume;
    size_t activeCount;
    PriceLevel(): totalVolume(0), activeCount(0) {}

    void reserve(size_t n) { orders.reserve(n); }

    idx_t allocSlot() {
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

    void freeSlot(idx_t i) {
        if (i < orders.size()) {
            orders[i].active = false;
            free_list.push_back(i);
        }
    }
};


class OrderBook {
public:
    struct Meta {
        price_t price;
        idx_t idx;
        Side side;
    };

    OrderBook(price_t min_tick, price_t max_tick, size_t reserve_per_level = 8)
        : minTick(min_tick), maxTick(max_tick)
    {
        if (maxTick < minTick) {
            throw std::runtime_error("invalid tick range");
        }
        nLevels = static_cast<size_t>(maxTick - minTick + 1);
        levels.resize(nLevels);
        for (auto &pl : levels) {
            pl.reserve(reserve_per_level);
        }
        idMap.reserve(1<<20);
    }

    bool newOrder(const Order& o) {
        if (o.qty == 0 || !inRange(o.price)) {
            return false;
        }
        if (idMap.count(o.id)) {
            return false;
        }
        bool findLevel = false;
        idx_t levelIdx = 0;
        for (size_t i = 0; i < nLevels; ++i) {
            if (levels[i].orders[0].price == o.price) {
                findLevel = true;
                break;
            }
            ++levelIdx;
        }

        idx_t slotid;
        if (!findLevel) {
            PriceLevel newLevel;
            idx_t id = newLevel.allocSlot();
            newLevel.orders[id] = o;
            newLevel.orders[id].active = true;
            newLevel.totalVolume += o.qty;
            newLevel.activeCount++;
            levels.push_back(newLevel);
            slotid = id;
        }
        else {
            auto &pl = levels[levelIdx];
            idx_t id = pl.allocSlot();
            pl.orders[id] = o;
            pl.orders[id].active = true;
            pl.totalVolume += o.qty;
            pl.activeCount++;
            slotid = id;
        }

        idMap.emplace(o.id, Meta{ o.price, slotid, o.side });
        return true;
    }

    bool amendOrder(id_t id, qty_t newQty) {
        auto it = idMap.find(id);
        if (it == idMap.end()) {
            return false;
        }

        Meta e = it->second;
        bool findLevel = false;
        size_t levelIdx = 0;
        for (size_t i = 0; i < nLevels; ++i) {
            if (levels[i].orders[0].price == e.price) {
                findLevel = true;
                break;
            }
            ++levelIdx;
        }
        if (!findLevel || e.idx >= levels[levelIdx].orders.size() || !levels[levelIdx].orders[e.idx].active) {
            return false;
        }

        auto &ord = levels[levelIdx].orders[e.idx];
        if (newQty == 0) {
            return deleteOrder(id);
        }
        if (newQty > ord.qty) {
            levels[levelIdx].totalVolume += (newQty - ord.qty);
        } 
        else {
            levels[levelIdx].totalVolume -= (ord.qty - newQty);
        }
        ord.qty = newQty;
        return true;
    }

    bool deleteOrder(id_t id) {
        auto it = idMap.find(id);
        if (it == idMap.end()) {
            return false;
        }

        Meta e = it->second;
        bool findLevel = false;
        size_t levelIdx = 0;
        for (size_t i = 0; i < nLevels; ++i) {
            if (levels[i].orders[0].price == e.price) {
                findLevel = true;
                break;
            }
            ++levelIdx;
        }
        if (e.idx >= levels[levelIdx].orders.size()|| !findLevel) {
            return false;
        }

        auto &ord = levels[levelIdx].orders[e.idx];
        if (!ord.active) {
            return false;
        }

        levels[levelIdx].totalVolume -= ord.qty;
        levels[levelIdx].activeCount--;
        levels[levelIdx].freeSlot(e.idx);
        idMap.erase(it);
        return true;
    }

    PriceLevelSummary topOfBook(Side s) const {
        PriceLevelSummary ret;
        if (s == Side::Buy) {
            size_t bestBidIdx = SIZE_MAX;
            price_t bestBidPrice = INT32_MIN;
            for (auto &pl : levels) {
                if (pl.activeCount && !pl.orders.empty()) {
                    for (const auto& ord : pl.orders) {
                        if (ord.active && ord.price > bestBidPrice) {
                            bestBidPrice = ord.price;
                            bestBidIdx = &pl - &levels[0];
                        }
                    }
                }
            }
            if (bestBidIdx != SIZE_MAX) {
                ret.price = levels[bestBidIdx].orders[0].price;
                ret.totalQty = levels[bestBidIdx].totalVolume;
                ret.orderCount = levels[bestBidIdx].activeCount;
            }
            else{
                ret.price = 0;
                ret.totalQty = 0;
                ret.orderCount = 0;
            }
        } 
        else {
            size_t bestAskIdx = SIZE_MAX;
            price_t bestAskPrice = INT32_MAX;
            for (auto &pl : levels) {
                if (pl.activeCount && !pl.orders.empty()) {
                    for (const auto& ord : pl.orders) {
                        if (ord.active && ord.price < bestAskPrice) {
                            bestAskPrice = ord.price;
                            bestAskIdx = &pl - &levels[0];
                        }
                    }
                }
            }
            if (bestAskIdx != SIZE_MAX) {
                ret.price = levels[bestAskIdx].orders[0].price;
                ret.totalQty = levels[bestAskIdx].totalVolume;
                ret.orderCount = levels[bestAskIdx].activeCount;
            }
            else {
                ret.price = 0;
                ret.totalQty = 0;
                ret.orderCount = 0;
            }
        }
        return ret;
    }

    size_t orderCount(price_t p) const {
        idx_t levelIdx = 0;
        if(!inRange(p)) {
            return 0;
        }
        for (size_t i = 0; i < nLevels; ++i) {
            if (levels[i].orders[0].price == p) {
                levelIdx = i;
                return levels[levelIdx].activeCount;
            }
        }
    }

    qty_t totalVolume(price_t p) const {
        idx_t levelIdx = 0;
        if(!inRange(p)) {
            return 0;
        }
        for (size_t i = 0; i < nLevels; ++i) {
            if (levels[i].orders[0].price == p) {
                levelIdx = i;
                return levels[levelIdx].totalVolume;
            }
        }
    }

    size_t totalOrders() const { 
        return idMap.size(); 
    }

    void clear() {
        for (auto &pl : levels) {
            pl.orders.clear(); 
            pl.free_list.clear();
            pl.totalVolume = 0; 
            pl.activeCount = 0;
        }
        idMap.clear();

    }

    void printPriceLevels() const {
        std::cout << "Price Levels:\n";
        for (const auto& pl : levels) {
            if (pl.activeCount > 0 && !pl.orders.empty()) {
                std::cout << "Price: " << pl.orders[0].price 
                          << ", Total Volume: " << pl.totalVolume 
                          << ", Active Orders: " << pl.activeCount << "\n";
            }
        }
    }

private:
    price_t minTick, maxTick;
    size_t nLevels;
    std::vector<PriceLevel> levels;
    std::unordered_map<id_t, Meta> idMap;

    inline bool inRange(price_t p) const { 
        bool result = p >= minTick && p <= maxTick; 
        return result;
    }
};

