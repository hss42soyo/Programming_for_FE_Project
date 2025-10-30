#pragma once
#include <map>
#include <vector>
#include <memory>
#include <algorithm>
#include "Order.hpp"

template <typename Price, typename OrderId>
class OrderBook {
public:
   using Ord = Order<Price, OrderId>;

#if USE_FLAT_CONTAINER
   // flat: contiguous storage, better locality; we scan for best
   std::vector<std::unique_ptr<Ord>> bids_;
   std::vector<std::unique_ptr<Ord>> asks_;
#else
   // map: always ordered by price
   std::multimap<Price, std::unique_ptr<Ord>> bids_;
   std::multimap<Price, std::unique_ptr<Ord>> asks_;
#endif

   void add(std::unique_ptr<Ord> ord) {
      if (ord->is_buy) {
#if USE_FLAT_CONTAINER
         bids_.push_back(std::move(ord));
#else
         bids_.emplace(ord->price, std::move(ord));
#endif
      }
      else {
#if USE_FLAT_CONTAINER
         asks_.push_back(std::move(ord));
#else
         asks_.emplace(ord->price, std::move(ord));
#endif
      }
   }

   // ----- best bid / best ask -----
   Ord* best_bid() {
#if USE_FLAT_CONTAINER
      if (bids_.empty()) return nullptr;
      auto it = std::max_element(bids_.begin(), bids_.end(),
         [](const auto& a, const auto& b) { return a->price < b->price; });
      return it->get();
#else
      if (bids_.empty()) return nullptr;
      return std::prev(bids_.end())->second.get(); // highest price
#endif
   }

   Ord* best_ask() {
#if USE_FLAT_CONTAINER
      if (asks_.empty()) return nullptr;
      auto it = std::min_element(asks_.begin(), asks_.end(),
         [](const auto& a, const auto& b) { return a->price < b->price; });
      return it->get();
#else
      if (asks_.empty()) return nullptr;
      return asks_.begin()->second.get(); // lowest price
#endif
   }

   
   const Ord* best_bid() const {
      return const_cast<OrderBook*>(this)->best_bid();
   }
   const Ord* best_ask() const {
      return const_cast<OrderBook*>(this)->best_ask();
   }

   // ----- erase best if quantity == 0 -----
   void pop_best_bid_if_empty() {
      Ord* b = best_bid();
      if (!b || b->quantity != 0) return;
#if USE_FLAT_CONTAINER
      auto it = std::max_element(bids_.begin(), bids_.end(),
         [](const auto& a, const auto& b) { return a->price < b->price; });
      if (it != bids_.end()) {
         bids_.erase(it); 
      }
#else
      auto it = bids_.end();
      if (it == bids_.begin()) return;
      --it;                 

      bids_.erase(it);
#endif
   }

   void pop_best_ask_if_empty() {
      Ord* a = best_ask();
      if (!a || a->quantity != 0) return;
#if USE_FLAT_CONTAINER
      auto it = std::min_element(asks_.begin(), asks_.end(),
         [](const auto& x, const auto& y) { return x->price < y->price; });
      if (it != asks_.end()) {
         asks_.erase(it);
      }
#else
      if (!asks_.empty()) {
         auto it = asks_.begin();
         asks_.erase(it);
      }
#endif
   }

   std::size_t bid_count() const {
#if USE_FLAT_CONTAINER
      return bids_.size();
#else
      return bids_.size();
#endif
   }
   std::size_t ask_count() const {
#if USE_FLAT_CONTAINER
      return asks_.size();
#else
      return asks_.size();
#endif
   }
};
