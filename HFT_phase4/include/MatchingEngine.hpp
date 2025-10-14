#pragma once
#include <vector>
#include <chrono>
#include "OrderBook.hpp"

struct Trade {
   std::string symbol;
   double price{};
   int quantity{};
   std::chrono::high_resolution_clock::time_point ts{};
};

template <typename Price, typename Oid>
class MatchingEngine {
public:
   using OB = OrderBook<Price, Oid>;
   using Ord = Order<Price, Oid>;

   explicit MatchingEngine(OB& ob) : ob_(ob) {}

   std::vector<Trade> match() {
      std::vector<Trade> trades;
      auto bid = ob_.best_bid();
      auto ask = ob_.best_ask();
      while (bid && ask && bid->price >= ask->price && bid->quantity > 0 && ask->quantity > 0) {
         const int qty = std::min(bid->quantity, ask->quantity);
         const double px = static_cast<double>(ask->price); 

         bid->quantity -= qty;
         ask->quantity -= qty;

         trades.push_back(Trade{ bid->symbol, px, qty, std::chrono::high_resolution_clock::now() });

         if (bid->quantity == 0) ob_.pop_best_bid_if_empty();
         if (ask->quantity == 0) ob_.pop_best_ask_if_empty();

         bid = ob_.best_bid();
         ask = ob_.best_ask();
      }
      return trades;
   }

private:
   OB& ob_;
};
