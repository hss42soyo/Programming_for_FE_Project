#include <iostream>
#include <vector>
#include <chrono>
#include "MarketDataFeed.h"
#include "TradeEngine.h"


int main() {
    std::vector<MarketData> feed;
    MarketDataFeed generator(feed);

    auto start = std::chrono::high_resolution_clock::now();

    TradeEngine engine(feed);
//    generator.generateData(1000000);
//    engine.process();
    engine.process_simultaneous(100000);

    auto end = std::chrono::high_resolution_clock::now();
    auto runtime = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    engine.reportStats();
    std::cout << "Total Runtime (ms): " << runtime << std::endl;

    return 0;
}
