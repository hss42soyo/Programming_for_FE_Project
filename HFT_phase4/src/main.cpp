#include <iostream>
#include <vector>
#include <numeric>
#include <algorithm>
#include <random>

#include "../include/MarketData.hpp"
#include "../include/Order.hpp"
#include "../include/OrderBook.hpp"
#include "../include/OrderManager.hpp"
#include "../include/MatchingEngine.hpp"
#include "../include/TradeLogger.hpp"
#include "../include/Timer.hpp"

using PriceT = double;
using OidT = int;
using OB = OrderBook<PriceT, OidT>;

static void analyzeLatencies(std::vector<long long>& lat) {
   if (lat.empty()) return;
   std::sort(lat.begin(), lat.end());
   auto min = lat.front();
   auto max = lat.back();
   double mean = std::accumulate(lat.begin(), lat.end(), 0.0) / lat.size();
   double var = 0.0;
   for (auto x : lat) { double d = x - mean; var += d * d; }
   double stddev = std::sqrt(var / lat.size());
   auto p99 = lat[static_cast<size_t>(lat.size() * 0.99)];

   std::cout << "Tick-to-Trade (ns)\n";
   std::cout << "Min: " << min << "  Max: " << max
      << "  Mean: " << mean << "  StdDev: " << stddev
      << "  P99: " << p99 << "\n";
}

int main() {
   const int num_ticks = 10000;
   std::vector<long long> latencies; latencies.reserve(num_ticks);

   MarketDataConfig cfg;
   cfg.symbol = "AAPL";
   cfg.mid = 150.0;
   cfg.spread = 0.02;
   cfg.tick_mod = 7;

   MarketDataFeed feed(cfg);
   OB ob;
   OrderManager<PriceT, OidT> oms;
   MatchingEngine<PriceT, OidT> me(ob);
   TradeLogger logger(1'000'00);

   std::mt19937 rng(42);
   std::uniform_int_distribution<int> qty_dist(10, 200);

   for (int i = 0; i < num_ticks; ++i) {
      Timer t; t.start();

      auto md = feed.next_tick(i);

      const bool is_buy = (i % 2 == 0);
      const double px = is_buy ? md.bid_price : md.ask_price;
      const int qty = qty_dist(rng);

      auto sp = oms.create(i, md.symbol, px, qty, is_buy);
      ob.add(sp);

      auto trades = me.match();
      if (!trades.empty()) logger.on_trades(trades);

      latencies.push_back(t.stop_ns());
   }

   analyzeLatencies(latencies);
   // logger.flush_to_file("trades.csv"); 
   return 0;
}
