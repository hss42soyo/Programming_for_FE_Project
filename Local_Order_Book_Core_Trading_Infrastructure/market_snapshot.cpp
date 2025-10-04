#include "market_snapshot.h"
#include <iostream>

void MarketSnapshot::update_bid(double price, int qty) {
    if (qty < 0) {
        bids.erase(price);
    } 
    else {
        auto it = bids.find(price);
        if(it != bids.end()) {
            it->second->quantity = qty; // Update existing price level
            if (qty == 0  && it == bids.begin()) {
                bids.erase(it);
                std::cout << "[Market] Best Bid: " << price << " removed\n";
            }
        } 
        else {
            bids.insert_or_assign(price, std::make_unique<PriceLevel>(price, qty));
            std::cout << "[Market] New Bid: " << price << " x " << qty << "\n";
        }
    }
    // Sort bids in descending order
    print(true);
    return;
}

void MarketSnapshot::update_ask(double price, int qty) {
    if (qty < 0) {
        asks.erase(price);
    }
    else {
        auto it = asks.find(price);
        if(it != asks.end()) {
            it->second->quantity = qty; // Update existing price level
            if (qty == 0  && it == asks.begin()) {
                asks.erase(it);
                std::cout << "[Market] Best Ask: " << price << " removed\n";
            }
        } 
        else {
            asks.insert_or_assign(price, std::make_unique<PriceLevel>(price, qty));
            std::cout << "[Market] New Ask: " << price << " x " << qty << "\n";
        }
    }
    // Sort asks in ascending order
    print(false);
    return;
}

const PriceLevel* MarketSnapshot::get_best_bid() const {
    if (bids.empty()) return nullptr;
    return bids.begin()->second.get();
}

const PriceLevel* MarketSnapshot::get_best_ask() const {
    if (asks.empty()) return nullptr;
    return asks.begin()->second.get();
}

void MarketSnapshot::print(bool isBuy) const {
    std::cout << "[Market] ";
    if (isBuy) {
        std ::cout << "Best Bid: " << get_best_bid()->price << " x " << get_best_bid()->quantity << "\n";
    } else {
        std ::cout << "Best Ask: " << get_best_ask()->price << " x " << get_best_ask()->quantity << "\n";
    }
}
