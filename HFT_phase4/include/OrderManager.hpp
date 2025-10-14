#pragma once
#include <unordered_map>
#include <memory>
#include "Order.hpp"

enum class OrderState { New, PartiallyFilled, Filled, Canceled };

template <typename Price, typename Oid>
class OrderManager {
public:
   using Ord = Order<Price, Oid>;

   std::shared_ptr<Ord> create(Oid id, const std::string& sym, Price px, int qty, bool is_buy) {
      auto sp = std::make_shared<Ord>(id, sym, px, qty, is_buy);
      states_[id] = OrderState::New;
      orders_[id] = sp;
      return sp;
   }

   void set_state(Oid id, OrderState st) { states_[id] = st; }
   OrderState state(Oid id) const { return states_.at(id); }
   std::shared_ptr<Ord> get(Oid id) const {
      auto it = orders_.find(id);
      return it == orders_.end() ? nullptr : it->second;
   }

private:
   std::unordered_map<Oid, std::shared_ptr<Ord>> orders_;
   std::unordered_map<Oid, OrderState> states_;
};
