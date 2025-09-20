#include "MarketDataFeed.h"
#include <unordered_map>

struct alignas(64) Order {
    int instrument_id;
    double price;
    bool is_buy;
    std::chrono::high_resolution_clock::time_point timestamp;
};

class TradeEngine
{
public:
    TradeEngine(std::vector<MarketData> &feed);

    void process();
    void process_simultaneous(int num_ticks);
    void reportStats();
    void getOrdersToCsv();

private:
    std::vector<MarketData> &market_data;
    std::vector<Order> orders;
    std::vector<long long> latencies;
    std::unordered_map<int, std::vector<double>> price_history;
    
    int signal1_count = 0, signal2_count = 0, signal3_count = 0, signal4_count = 0;
    void updateHistory(const MarketData &tick);
    double getAvg(int id);
    double getStd(int id);
    bool signal1(const MarketData &tick);
    bool signal2(const MarketData &tick);
    bool signal3(const MarketData &tick);
    int signal4(const MarketData &tick);
};
