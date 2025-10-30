#pragma once
#include <string>
#include <type_traits>

#if USE_POOL_ALLOC
#include "MemoryPool.hpp"
#endif

template <typename PriceType, typename OrderIdType>
struct Order {
   static_assert(std::is_integral<OrderIdType>::value, "Order ID must be an integer");

   OrderIdType id;
   std::string symbol;
   PriceType price;
   int quantity;
   bool is_buy;

   Order(OrderIdType id_, std::string sym, PriceType pr, int qty, bool buy)
      : id(id_), symbol(std::move(sym)), price(pr), quantity(qty), is_buy(buy) {
   }

#if USE_POOL_ALLOC
   // Per-type singleton pool (lazily constructed)
   static ObjectPool<Order<PriceType, OrderIdType>, 4096>& pool() {
      static ObjectPool<Order<PriceType, OrderIdType>, 4096> p;
      return p;
   }
   static void* operator new(std::size_t) {
      return pool().allocate();
   }
   static void operator delete(void* p) noexcept {
      pool().deallocate(p);
   }
#endif
};
