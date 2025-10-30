#pragma once
#include <unordered_map>
#include <string>

enum class OrderState { New, PartiallyFilled, Filled, Canceled };

// OMS keeps states only (no owning pointers). OrderBook owns orders.
template <typename Price, typename Oid>
class OrderManager {
public:
   void on_new(Oid id) { states_[id] = OrderState::New; }

   void on_partial(Oid id) { states_[id] = OrderState::PartiallyFilled; }
   void on_filled(Oid id) { states_[id] = OrderState::Filled; }
   void cancel(Oid id) { states_[id] = OrderState::Canceled; }

   bool has(Oid id) const { return states_.find(id) != states_.end(); }
   OrderState state(Oid id) const { return states_.at(id); }

private:
   std::unordered_map<Oid, OrderState> states_;
};
