#pragma once
#include <string>
#include <type_traits>

template <typename PriceType, typename OrderIdType>
struct Order {
   static_assert(std::is_integral<OrderIdType>::value, "Order ID must be an integer");

   OrderIdType id{};
   std::string symbol;
   PriceType price{};
   int quantity{};
   bool is_buy{};

   Order(OrderIdType id_, std::string sym_, PriceType pr_, int qty_, bool buy_)
      : id(id_), symbol(std::move(sym_)), price(pr_), quantity(qty_), is_buy(buy_) {
   }
};
