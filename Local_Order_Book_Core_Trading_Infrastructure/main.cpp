#include "order_manager.h"
#include "market_snapshot.h"
#include "feed_parser.h"
#include <iostream>
#include <vector>
#include <fstream>

bool ShouldBuy(MarketSnapshot& snapshot) {
    if (snapshot.asks.empty()) {
        return false;
    }
    if (snapshot.get_best_ask()->price <= 100.20) {
        return true;
    }
    return false;
}

bool ShouldSell(MarketSnapshot& snapshot) {
    if (snapshot.bids.empty()) {
        return false;
    }
    if (snapshot.get_best_bid()->price >= 100.15) {
        return true;
    }
    return false;
}

int main() {
    std::ofstream logfile("output.log");
    std::streambuf* orig = std::cout.rdbuf();
    std::cout.rdbuf(logfile.rdbuf());

    auto feed = load_feed("sample_feed.txt");
    OrderManager order_manager;
    MarketSnapshot snapshot;

    std::cout << "Trading session started\n" << std::endl;

    for (const auto& event : feed) {
        event.print();

        // Integrate with your components:
        if (event.type == FeedType::BID) {
            snapshot.update_bid(event.price, event.quantity);
            if (ShouldSell(snapshot)) {
                order_manager.place_order(Side::Sell, snapshot.get_best_bid()->price, 100);
            }
            
        } 
        else if (event.type == FeedType::ASK) {
            snapshot.update_ask(event.price, event.quantity);
            if (ShouldSell(snapshot)) {
                order_manager.place_order(Side::Buy, snapshot.get_best_ask()->price, 100);
            }
        } 
        else if (event.type == FeedType::EXECUTION) {
            order_manager.handle_fill(event.order_id, event.quantity);
        }
    }
    std::cout << "Trading session ended\n" << std::endl;

    std::cout.rdbuf(orig);
    logfile.close();

    std::cout << "Log saved to output.log" << std::endl;


    return 0;
}