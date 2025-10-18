#pragma once
#include <string>
#include <type_traits>
#include "MemoryPool.hpp"

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

   // Per-type static pool (single-threaded; adjust size if needed).
   static FixedBlockPool& pool() {
      static FixedBlockPool p(sizeof(Order), 1u << 20); // ~1M nodes max
      return p;
   }

   // Custom new/delete to hit the pool (allocator experiment).
   void* operator new(std::size_t sz) {
      if (sz == sizeof(Order)) return pool().alloc();
      return ::operator new(sz);
   }
   void operator delete(void* p, std::size_t sz) noexcept {
      if (p == nullptr) return;
      if (sz == sizeof(Order)) { pool().dealloc(p); return; }
      ::operator delete(p);
   }
};
