#pragma once
#include <string>
#include <chrono>
#include "Config.hpp"  // 确保 HFT_ALIGN 定义可见

struct HFT_ALIGN MarketData {
   std::string symbol;
   double bid_price;
   double ask_price;
   std::chrono::high_resolution_clock::time_point timestamp;
};

struct MarketDataConfig {
   std::string symbol{ "AAPL" };
   double mid{ 150.0 };
   double spread{ 0.02 };
   int tick_mod{ 5 };
};

class MarketDataFeed {
public:
   explicit MarketDataFeed(MarketDataConfig cfg) : cfg_(std::move(cfg)) {}

   MarketData next_tick(int i) {
      MarketData md;
      md.symbol = cfg_.symbol;
      const double d = static_cast<double>(i % cfg_.tick_mod);
      const double mid = cfg_.mid + 0.01 * d;
      md.bid_price = mid - cfg_.spread * 0.5;
      md.ask_price = mid + cfg_.spread * 0.5;
      md.timestamp = std::chrono::high_resolution_clock::now();
      return md;
   }

private:
   MarketDataConfig cfg_;
};
