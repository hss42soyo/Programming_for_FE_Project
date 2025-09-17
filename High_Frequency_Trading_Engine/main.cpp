#include <iostream>
#include <vector>
#include <chrono>
#include "MarketDataFeed.h"
#include "TradeEngine.h"


int main() {
    std::vector<MarketData> feed;
    MarketDataFeed generator(feed);

    auto start = std::chrono::high_resolution_clock::now();
    generator.generateData(100000);

    TradeEngine engine(feed);
    engine.process();

    auto end = std::chrono::high_resolution_clock::now();
    auto runtime = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    engine.reportStats();
    std::cout << \"Total Runtime (ms): \" << runtime << std::endl;

    return 0;
}