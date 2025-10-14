#pragma once
#include <map>
#include <memory>
#include <optional>
#include "Order.hpp"

template <typename Price, typename Oid>
class OrderBook {
public:
   using Ord = Order<Price, Oid>;

   std::shared_ptr<Ord> add(std::shared_ptr<Ord> o) {
      if (o->is_buy) {
         bids_.emplace(o->price, std::move(o));
      }
      else {
         asks_.emplace(o->price, std::move(o));
      }
      return o;
   }

   std::shared_ptr<Ord> best_bid() const {
      if (bids_.empty()) return nullptr;
      return bids_.rbegin()->second;
   }
   std::shared_ptr<Ord> best_ask() const {
      if (asks_.empty()) return nullptr;
      return asks_.begin()->second;
   }

   void pop_best_bid_if_empty() {
      if (!bids_.empty() && bids_.rbegin()->second->quantity == 0) {
         auto it = bids_.end(); --it;
         bids_.erase(it);
      }
   }
   void pop_best_ask_if_empty() {
      if (!asks_.empty() && asks_.begin()->second->quantity == 0) {
         asks_.erase(asks_.begin());
      }
   }

private:

   std::multimap<Price, std::shared_ptr<Ord>> asks_;
   std::multimap<Price, std::shared_ptr<Ord>> bids_;
};
