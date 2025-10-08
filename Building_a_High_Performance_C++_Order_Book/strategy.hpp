// strategy.hpp
#pragma once
#include <cstdio>
#include "order_book.hpp"

struct PrintStrategy {
  void onL1(const L1& l1) {
    // 简单打印示例；真实策略里做alpha/下单等
    std::printf("L1: bid=%u(%u) / ask=%u(%u)\n",
      l1.bestBid, l1.bidQty, l1.bestAsk, l1.askQty);
  }
};
