#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>

enum class FeedType {
    BID,
    ASK,
    EXECUTION,
    UNKNOWN
};

struct FeedEvent {
    FeedType type = FeedType::UNKNOWN;
    double price = 0.0;
    int quantity = 0;
    int order_id = -1; // used for EXECUTION only

    // Debug print
    void print() const {
        switch (type) {
            case FeedType::BID:
                std::cout << "[BID] " << price << " x " << quantity << "\n";
                break;
            case FeedType::ASK:
                std::cout << "[ASK] " << price << " x " << quantity << "\n";
                break;
            case FeedType::EXECUTION:
                std::cout << "[EXECUTION] Order " << order_id << " filled: " << quantity << "\n";
                break;
            default:
                std::cout << "[UNKNOWN]\n";
        }
    }
};

std::vector<FeedEvent> load_feed(const std::string& filename) {
    std::ifstream file(filename);
    std::vector<FeedEvent> events;

    if (!file.is_open()) {
        std::cerr << "Error: could not open file " << filename << "\n";
        return events;
    }

    std::string line;
    while (std::getline(file, line)) {
        // Ignore comments and empty lines
        if (line.empty() || line[0] == '#') continue;

        std::istringstream iss(line);
        std::string type;
        iss >> type;

        if (type == "BID") {
            double price;
            int qty;
            if (iss >> price >> qty) {
                events.push_back({FeedType::BID, price, qty});
            }
        } else if (type == "ASK") {
            double price;
            int qty;
            if (iss >> price >> qty) {
                events.push_back({FeedType::ASK, price, qty});
            }
        } else if (type == "EXECUTION") {
            int order_id;
            int filled;
            if (iss >> order_id >> filled) {
                events.push_back({FeedType::EXECUTION, 0.0, filled, order_id});
            }
        } else {
            std::cerr << "Unknown event type: " << line << "\n";
        }
    }

    return events;
}