// order_book.hpp
#pragma once
#include <cstdint>
#include <map>
#include <unordered_map>
#include <vector>
#include <functional>
#include <limits>
#include <cassert>

using id_t  = uint64_t;
using px_t  = uint32_t;        // 价格按tick编码
using qty_t = uint32_t;

enum class Side : uint8_t { Buy=0, Sell=1 };

struct alignas(64) Order {     // 尽量cache line对齐
  id_t  id;
  px_t  px;
  qty_t qty;
  Side  side;
};

struct PriceLevel {
  qty_t totalQty{0};
  uint32_t orderCount{0};
};

struct L1 {                    // L1快照
  px_t bestBid{0};
  px_t bestAsk{std::numeric_limits<px_t>::max()};
  qty_t bidQty{0};
  qty_t askQty{0};
};

// 订阅者：收到L1更新回调
using L1Callback = std::function<void(const L1&)>;

class OrderBook {
public:
  explicit OrderBook(L1Callback cb = {}) : onL1_(std::move(cb)) {
    id2idx_.reserve(1u<<20);   // 预留容量避免rehash
  }

  // 新单：O(log M) 更新价格层，O(1) 记录id索引
  inline void newOrder(const Order& o) {
    assert(o.qty>0);
    auto &lvl = (o.side==Side::Buy) ? bids_[o.px] : asks_[o.px];
    lvl.totalQty   += o.qty;
    lvl.orderCount += 1;
    id2idx_[o.id] = {o.px,o.qty,o.side};
    // 只有触及顶簿才触发回调
    maybeEmitL1(o.side, o.px);
  }

  // 改量：仅支持改数量（正/负变动均可；减到0等价删单）
  inline void amendOrder(id_t id, int32_t delta) {
    auto it = id2idx_.find(id); if (it==id2idx_.end()) return; // 忽略不存在
    auto &meta = it->second;
    if (delta==0) return;
    // 定位价格层
    auto &tree = (meta.side==Side::Buy) ? bids_ : asks_;
    auto lit = tree.find(meta.px); if (lit==tree.end()) return;
    // 应用变化
    int64_t newQty = int64_t(meta.qty) + delta;
    if (newQty <= 0) { deleteOrder(id); return; }
    qty_t d = (delta>0)? qty_t(delta) : qty_t(-delta);
    if (delta>0) lit->second.totalQty += d;
    else         lit->second.totalQty -= d;
    meta.qty = qty_t(newQty);
    maybeEmitL1(meta.side, meta.px);
  }

  // 删单：O(1)+O(log M)；价格层清空则擦除该层
  inline void deleteOrder(id_t id) {
    auto it = id2idx_.find(id); if (it==id2idx_.end()) return;
    auto meta = it->second;
    auto &tree = (meta.side==Side::Buy) ? bids_ : asks_;
    auto lit = tree.find(meta.px); if (lit!=tree.end()) {
      auto &lvl = lit->second;
      lvl.totalQty   -= meta.qty;
      lvl.orderCount -= 1;
      if (lvl.orderCount==0) tree.erase(lit);
      id2idx_.erase(it);
      maybeEmitL1(meta.side, meta.px);
    } else {
      id2idx_.erase(it);
    }
  }

  // 顶簿：O(1) 取map首/尾（空则返回缺省）
  inline PriceLevel topOfBook(Side s, px_t* outPx=nullptr) const {
    if (s==Side::Buy) {
      if (bids_.empty()) return {};
      const auto &kv = *bids_.rbegin();
      if (outPx) *outPx = kv.first;
      return kv.second;
    } else {
      if (asks_.empty()) return {};
      const auto &kv = *asks_.begin();
      if (outPx) *outPx = kv.first;
      return kv.second;
    }
  }

  // 同价位订单数量/总量：O(log M)
  inline uint32_t orderCount(px_t px, Side s) const {
    const auto &tree = (s==Side::Buy)? bids_ : asks_;
    auto it = tree.find(px); if (it==tree.end()) return 0;
    return it->second.orderCount;
  }
  inline qty_t totalVolume(px_t px, Side s) const {
    const auto &tree = (s==Side::Buy)? bids_ : asks_;
    auto it = tree.find(px); if (it==tree.end()) return 0;
    return it->second.totalQty;
  }

  // 当前L1（无副作用）
  inline L1 snapshotL1() const {
    L1 l1;
    auto bid = topOfBook(Side::Buy, &l1.bestBid);
    auto ask = topOfBook(Side::Sell, &l1.bestAsk);
    l1.bidQty = bid.totalQty;
    l1.askQty = ask.totalQty;
    return l1;
  }

private:
  struct Meta { px_t px; qty_t qty; Side side; };
  std::unordered_map<id_t, Meta> id2idx_; // id -> 元数据
  std::map<px_t, PriceLevel> bids_;       // 活跃买价层
  std::map<px_t, PriceLevel> asks_;       // 活跃卖价层
  L1Callback onL1_;

  inline void maybeEmitL1(Side changedSide, px_t changedPx) {
    if (!onL1_) return;
    // 仅在影响bestBid/bestAsk时回调（O(1)取端点）
    if (changedSide==Side::Buy) {
      if (bids_.empty() || asks_.empty()) { onL1_(snapshotL1()); return; }
      if (changedPx==bids_.rbegin()->first) onL1_(snapshotL1());
    } else {
      if (bids_.empty() || asks_.empty()) { onL1_(snapshotL1()); return; }
      if (changedPx==asks_.begin()->first) onL1_(snapshotL1());
    }
  }
};
