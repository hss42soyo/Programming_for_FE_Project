#pragma once
#include <vector>
#include <fstream>
#include <string>
#include "MatchingEngine.hpp"

class TradeLogger {
public:
   explicit TradeLogger(size_t reserve_n = 1'000'000) { trades_.reserve(reserve_n); }

   void on_trades(const std::vector<Trade>& ts) {
      for (const auto& t : ts) trades_.push_back(t);
   }

   void flush_to_file(const std::string& path) {
      std::ofstream ofs(path, std::ios::app);
      for (const auto& t : trades_) {
         ofs << t.symbol << ',' << t.price << ',' << t.quantity << '\n';
      }
      trades_.clear();
   }

private:
   std::vector<Trade> trades_;
};
