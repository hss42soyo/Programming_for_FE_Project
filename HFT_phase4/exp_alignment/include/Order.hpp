#pragma once
#include <string>
#include <type_traits>
#include "Config.hpp"

template <typename PriceType, typename OrderIdType>
struct HFT_ALIGN Order {
   static_assert(std::is_integral<OrderIdType>::value, "Order ID must be an integer");

   OrderIdType id;
   std::string symbol;
   PriceType price;
   int quantity;
   bool is_buy;

   Order(OrderIdType id_, std::string sym, PriceType pr, int qty, bool buy)
      : id(id_), symbol(std::move(sym)), price(pr), quantity(qty), is_buy(buy) {
   }
};
