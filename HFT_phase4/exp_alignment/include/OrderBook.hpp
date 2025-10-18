#pragma once
#include <map>
#include <memory>
#include "Order.hpp"

template <typename PriceT, typename IdT>
class OrderBook {
public:
   using Ord = Order<PriceT, IdT>;
#if USE_RAW_PTR
   using OrderPtr = Ord*;                      // raw pointer
   const char* pointer_type = "raw";
#else
   using OrderPtr = std::unique_ptr<Ord>;      // unique_ptr
   const char* pointer_type = "smart";
#endif

   void add(OrderPtr ord) {
#if USE_RAW_PTR
      if (ord->is_buy) bids_.insert({ ord->price, ord });
      else             asks_.insert({ ord->price, ord });
#else
      if (ord->is_buy) bids_.insert({ ord->price, std::move(ord) });
      else             asks_.insert({ ord->price, std::move(ord) });
#endif
   }


   Ord* best_bid() const {
      if (bids_.empty()) return nullptr;
      auto it = bids_.end(); --it;          
#if USE_RAW_PTR
      return it->second;
#else
      return it->second.get();
#endif
   }
   Ord* best_ask() const {
      if (asks_.empty()) return nullptr;
      auto it = asks_.begin();              
#if USE_RAW_PTR
      return it->second;
#else
      return it->second.get();
#endif
   }

   void pop_best_bid_if_empty() {
      Ord* b = best_bid();
      if (!b || b->quantity != 0) return;
      auto it = bids_.end(); --it;
#if USE_RAW_PTR
      delete it->second;
#endif
      bids_.erase(it);
   }
   void pop_best_ask_if_empty() {
      Ord* a = best_ask();
      if (!a || a->quantity != 0) return;
      auto it = asks_.begin();
#if USE_RAW_PTR
      delete it->second;
#endif
      asks_.erase(it);
   }

#if USE_RAW_PTR
   ~OrderBook() {
      for (auto& kv : bids_) delete kv.second;
      for (auto& kv : asks_) delete kv.second;
   }
#endif

private:
   std::multimap<PriceT, OrderPtr> asks_;
   std::multimap<PriceT, OrderPtr> bids_;
};
