#pragma once
#include <vector>
#include <chrono>

struct alignas(64) MarketData {
    int instrument_id;
    double price;
    std::chrono::high_resolution_clock::time_point timestamp;
};

class MarketDataFeed {
public:
    explicit MarketDataFeed(std::vector<MarketData>& ref);
    void generateData(int num_ticks);
private:
    std::vector<MarketData>& data;
};

#endif