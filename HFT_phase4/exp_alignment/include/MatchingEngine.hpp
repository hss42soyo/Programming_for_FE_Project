#pragma once
#include <vector>
#include <chrono>
#include "OrderBook.hpp"
#include "OrderManager.hpp"

struct Trade {
   std::string symbol;
   double price{};
   int quantity{};
   std::chrono::high_resolution_clock::time_point ts{};
};

// MatchingEngine updates OMS states and records per-trade latency.
template <typename Price, typename Oid>
class MatchingEngine {
public:
   using OB = OrderBook<Price, Oid>;
   using Ord = Order<Price, Oid>;

   explicit MatchingEngine(OB& ob, OrderManager<Price, Oid>& oms)
      : ob_(ob), oms_(oms) {
   }

   // Match until crossed. For each trade, append latency (ns) since tick_start.
   std::vector<Trade> match(std::chrono::high_resolution_clock::time_point tick_start,
      std::vector<long long>& per_trade_lat_ns)
   {
      std::vector<Trade> trades;
      Ord* bid = ob_.best_bid();
      Ord* ask = ob_.best_ask();

      while (bid && ask && bid->price >= ask->price && bid->quantity > 0 && ask->quantity > 0) {
         const int qty = std::min(bid->quantity, ask->quantity);
         const double px = static_cast<double>(ask->price);

         bid->quantity -= qty;
         ask->quantity -= qty;

         // record trade + per-trade latency
         auto now = std::chrono::high_resolution_clock::now();
         trades.push_back(Trade{ bid->symbol, px, qty, now });
         per_trade_lat_ns.push_back(
            std::chrono::duration_cast<std::chrono::nanoseconds>(now - tick_start).count()
         );

         // update OMS states (no ownership here)
         if (bid->quantity == 0)  oms_.on_filled(bid->id);
         else                     oms_.on_partial(bid->id);
         if (ask->quantity == 0)  oms_.on_filled(ask->id);
         else                     oms_.on_partial(ask->id);

         if (bid->quantity == 0) ob_.pop_best_bid_if_empty();
         if (ask->quantity == 0) ob_.pop_best_ask_if_empty();

         bid = ob_.best_bid();
         ask = ob_.best_ask();
      }
      return trades;
   }

private:
   OB& ob_;
   OrderManager<Price, Oid>& oms_;
};
