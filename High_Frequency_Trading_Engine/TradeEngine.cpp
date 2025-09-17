#include <unordered_map>
#include <iostream>
#include "MarketDataFeed.h"
#include "TradeEngine.h"

// class TradeEngine
// {
// public:
//     TradeEngine(const std::vector<MarketData> &feed)
//         : market_data(feed) {}

//     void process()
//     {
//         for (const auto &tick : market_data)
//         {
//             // Update history
//             updateHistory(tick);

//             // Apply signals
//             bool buy = false, sell = false;

//             if (signal1(tick))
//                 buy = true;
//             if (signal2(tick))
//             {
//                 if (tick.price < getAvg(tick.instrument_id))
//                     buy = true;
//                 else
//                     sell = true;
//             }
//             if (signal3(tick))
//                 buy = true;

//             if (buy || sell)
//             {
//                 auto now = std::chrono::high_resolution_clock::now();
//                 Order o{tick.instrument_id, tick.price + (buy ? 0.01 : -0.01), buy, now};
//                 orders.push_back(o);

//                 auto latency = std::chrono::duration_cast<std::chrono::nanoseconds>(now - tick.timestamp).count();
//                 latencies.push_back(latency);
//             }
//         }
//     }

//     void reportStats()
//     {
//         long long sum = 0, max_latency = 0;
//         for (auto l : latencies)
//         {
//             sum += l;
//             if (l > max_latency)
//                 max_latency = l;
//         }

//         std::cout << \"\\n--- Performance Report ---\\n\";
//             std::cout
//                   << \"Total Market Ticks Processed: \" << market_data.size() << \"\\n\";
//             std::cout
//                   << \"Total Orders Placed: \" << orders.size() << \"\\n\";
//             std::cout
//                   << \"Average Tick-to-Trade Latency (ns): \" << (latencies.empty() ? 0 : sum / latencies.size()) << \"\\n\";
//             std::cout
//                   << \"Maximum Tick-to-Trade Latency (ns): \" << max_latency << \"\\n\";
//     };

// private:
//     const std::vector<MarketData> &market_data;
//     std::vector<Order> orders;
//     std::vector<long long> latencies;
//     std::unordered_map<int, std::vector<double>> price_history;

//     void updateHistory(const MarketData &tick)
//     {
//         auto &hist = price_history[tick.instrument_id];
//         hist.push_back(tick.price);
//         if (hist.size() > 10)
//             hist.erase(hist.begin());
//     }

//     double getAvg(int id)
//     {
//         auto &hist = price_history[id];
//         double sum = 0;
//         for (double p : hist)
//             sum += p;
//         return hist.empty() ? 0 : sum / hist.size();
//     }

//     // Signal 1: Price thresholds
//     bool signal1(const MarketData &tick)
//     {
//         return tick.price < 105.0 || tick.price > 195.0;
//     }

//     // Signal 2: Deviation from average
//     bool signal2(const MarketData &tick)
//     {
//         if (price_history[tick.instrument_id].size() < 5)
//             return false;
//         double avg = getAvg(tick.instrument_id);
//         return tick.price < avg * 0.98 || tick.price > avg * 1.02;
//     }

//     // Signal 3: Simple momentum
//     bool signal3(const MarketData &tick)
//     {
//         auto &hist = price_history[tick.instrument_id];
//         if (hist.size() < 3)
//             return false;
//         double diff1 = hist[hist.size() - 2] - hist[hist.size() - 3];
//         double diff2 = hist[hist.size() - 1] - hist[hist.size() - 2];
//         return diff1 > 0 && diff2 > 0;
//     }
// };

TradeEngine::TradeEngine(const std::vector<MarketData> &feed)
    : market_data(feed) {}

void TradeEngine::process()
{
    for (const auto &tick : market_data)
    {
        // Update history
        updateHistory(tick);

        // Apply signals
        bool buy = false, sell = false;

        if (signal1(tick))
            buy = true;
        if (signal2(tick))
        {
            if (tick.price < getAvg(tick.instrument_id))
                buy = true;
            else
                sell = true;
        }
        if (signal3(tick))
            buy = true;
        if (signal4(tick) == 1){
            buy = true;
        }else if(signal4(tick) == -1){
            sell = true;
        }

        if (buy || sell)
        {
            auto now = std::chrono::high_resolution_clock::now();
            Order o{tick.instrument_id, tick.price + (buy ? 0.01 : -0.01), buy, now};
            orders.push_back(o);

            auto latency = std::chrono::duration_cast<std::chrono::nanoseconds>(now - tick.timestamp).count();
            latencies.push_back(latency);
        }
    }
}

void TradeEngine::reportStats()
{
    long long sum = 0, max_latency = 0;
    for (auto l : latencies)
    {
        sum += l;
        if (l > max_latency)
            max_latency = l;
    }

    std::cout << "\n--- Performance Report ---\n";
    std::cout << "Total Market Ticks Processed: " << market_data.size() << "\n";
    std::cout << "Total Orders Placed: " << orders.size() << "\n";
    std::cout << "Average Tick-to-Trade Latency (ns): "
              << (latencies.empty() ? 0 : sum / latencies.size()) << "\n";
    std::cout << "Maximum Tick-to-Trade Latency (ns): " << max_latency << "\n";
}

void TradeEngine::updateHistory(const MarketData &tick)
{
    auto &hist = price_history[tick.instrument_id];
    hist.push_back(tick.price);
    if (hist.size() > 10)
        hist.erase(hist.begin());
}

double TradeEngine::getAvg(int id)
{
    auto &hist = price_history[id];
    double sum = 0;
    for (double p : hist)
        sum += p;
    return hist.empty() ? 0 : sum / hist.size();
}

double TradeEngine::getStd(int id)
{
    auto &hist = price_history[id];
    double sum = 0, avg = getAvg(id);
    for (double p : hist)
        sum += (p - avg) * (p - avg);
    return hist.empty() ? 0 : std::sqrt(sum / hist.size());
}

bool TradeEngine::signal1(const MarketData &tick)
{
    return tick.price < 105.0 || tick.price > 195.0;
}

bool TradeEngine::signal2(const MarketData &tick)
{
    if (price_history[tick.instrument_id].size() < 5)
        return false;
    double avg = getAvg(tick.instrument_id);
    return tick.price < avg * 0.98 || tick.price > avg * 1.02;
}

bool TradeEngine::signal3(const MarketData &tick)
{
    auto &hist = price_history[tick.instrument_id];
    if (hist.size() < 3)
        return false;
    double diff1 = hist[hist.size() - 2] - hist[hist.size() - 3];
    double diff2 = hist[hist.size() - 1] - hist[hist.size() - 2];
    return diff1 > 0 && diff2 > 0;
}

bool TradeEngine::signal4(const MarketData &tick)
{
    double avg = getAvg(tick.instrument_id);
    double std = getStd(tick.instrument_id);
    double dir = (tick.price - avg)/std;
    if (dir > 2){
        return -1;
    }  
    else if (dir < -2){
        return 1;
    }
    else{
        return 0;
    }
}