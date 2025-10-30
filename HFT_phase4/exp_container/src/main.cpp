#include <iostream>
#include <vector>
#include <numeric>
#include <algorithm>
#include <random>
#include <fstream>

#include "../include/MarketData.hpp"     // your original simple feed
#include "../include/Order.hpp"
#include "../include/OrderBook.hpp"
#include "../include/OrderManager.hpp"
#include "../include/MatchingEngine.hpp"
#include "../include/TradeLogger.hpp"
#include "../include/Timer.hpp"

using PriceT = double;
using OidT = int;
using OB = OrderBook<PriceT, OidT>;
using Ord = Order<PriceT, OidT>;

struct Stats {
   long long min{};
   long long max{};
   double mean{};
   double stddev{};
   long long p95{};
   long long p99{};
};

// compute stats without changing the original vector
static Stats computeStats(const std::vector<long long>& lat) {
   Stats s{};
   if (lat.empty()) return s;

   std::vector<long long> v = lat;              // copy to sort
   std::sort(v.begin(), v.end());

   s.min = v.front();
   s.max = v.back();

   double sum = std::accumulate(v.begin(), v.end(), 0.0);
   s.mean = sum / v.size();

   double var = 0.0;
   for (auto x : v) { double d = x - s.mean; var += d * d; }
   s.stddev = std::sqrt(var / v.size());

   s.p95 = v[static_cast<size_t>(v.size() * 0.95)];
   s.p99 = v[static_cast<size_t>(v.size() * 0.99)];
   return s;
}

static void appendCsv(const std::string& path,
   const std::string& metric,
   int ticks,
   const std::string& container_type,
   const Stats& s) {
   std::ifstream fin(path);
   bool has_header = fin.good() && fin.peek() != std::ifstream::traits_type::eof();
   fin.close();

   std::ofstream fout(path, std::ios::app);
   if (!has_header) {
      fout << "container_type,metric,ticks,min_ns,max_ns,mean_ns,stddev_ns,p95_ns,p99_ns\n";
   }
   fout << container_type << ',' << metric << ',' << ticks << ','
      << s.min << ',' << s.max << ',' << s.mean << ',' << s.stddev << ','
      << s.p95 << ',' << s.p99 << '\n';
}

static void analyzeLatencies(std::vector<long long>& lat) {
   if (lat.empty()) return;
   std::sort(lat.begin(), lat.end());
   auto min = lat.front();
   auto max = lat.back();
   double mean = std::accumulate(lat.begin(), lat.end(), 0.0) / lat.size();
   double var = 0.0;
   for (auto x : lat) { double d = x - mean; var += d * d; }
   double stddev = std::sqrt(var / lat.size());
   auto p95 = lat[static_cast<size_t>(lat.size() * 0.95)];
   auto p99 = lat[static_cast<size_t>(lat.size() * 0.99)];

   std::cout << "Tick-to-Trade (ns)\n";
   std::cout << "Min: " << min << "  Max: " << max
      << "  Mean: " << mean << "  StdDev: " << stddev
      << "  P95: " << p95 << "  P99: " << p99 << "\n";
}

int main() {
   const int num_ticks = 10000;

#if USE_FLAT_CONTAINER
   const std::string container_type = "flat";
#else
   const std::string container_type = "map";
#endif
   std::cout << "[container] " << container_type << "\n";

#if USE_RAW_PTR
   const std::string ptr_type = "raw";
#else
   const std::string ptr_type = "smart";
#endif
   std::cout << "[pointer] " << ptr_type << "\n";

   std::vector<long long> tick_latencies;
   std::vector<long long> trade_latencies_ns;
   tick_latencies.reserve(num_ticks);
   trade_latencies_ns.reserve(num_ticks * 2);

   MarketDataConfig cfg{ /* ... */ };
   MarketDataFeed feed(cfg);

   OB ob;
   OrderManager<PriceT, OidT> oms;
   MatchingEngine<PriceT, OidT> me(ob, oms);
   TradeLogger logger(100000);

   std::mt19937 rng(42);
   std::uniform_int_distribution<int> qty_dist(10, 200);

   for (int i = 0; i < num_ticks; ++i) {
      Timer t; t.start();
      auto tick_start = std::chrono::high_resolution_clock::now();

      auto md = feed.next_tick(i);

      const bool is_buy = (i % 2 == 0);
      const double px = is_buy ? md.bid_price : md.ask_price;
      const int qty = qty_dist(rng);

      oms.on_new(i);

#if USE_RAW_PTR
      Ord* p = new Ord(i, md.symbol, px, qty, is_buy);
      ob.add(p);
#else
      auto up = std::make_unique<Ord>(i, md.symbol, px, qty, is_buy);
      ob.add(std::move(up));
#endif

      auto trades = me.match(tick_start, trade_latencies_ns);
      if (!trades.empty()) logger.on_trades(trades);

      tick_latencies.push_back(t.stop_ns());
   }

   #ifdef CSV_DIR
      const std::string csv_path = std::string(CSV_DIR) + "/results_container.csv";
   #else
      const std::string csv_path = "results_container.csv";
   #endif

   const Stats st_tick = computeStats(tick_latencies);
   const Stats st_trade = computeStats(trade_latencies_ns);
   appendCsv(csv_path, "Per-Tick", num_ticks, container_type, st_tick);
   appendCsv(csv_path, "Per-Trade", num_ticks, container_type, st_trade);


   return 0;
}

