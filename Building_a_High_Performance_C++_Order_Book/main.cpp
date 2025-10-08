// bench_main.cpp
#include <chrono>
#include <random>
#include <cstdio>
#include <algorithm>
#include "order_book.hpp"
#include "strategy.hpp"

using clk = std::chrono::steady_clock;

struct Event { uint8_t type; id_t id; px_t px; qty_t qty; Side side; };
// type: 0=new, 1=amend, 2=del

int main() {
  const size_t N = 10'000'000;           // 1e7事件
  const px_t pxMin=9900, pxMax=10100;    // 模拟价位区间
  const qty_t qMin=1, qMax=50;

  PrintStrategy strat;
  OrderBook ob([&](const L1& l1){ strat.onL1(l1); }); // 注册回调
  std::vector<Event> evts; evts.reserve(N);

  // 生成事件流（先插入一批，后续混合amend/delete）
  std::mt19937_64 rng(42);
  std::uniform_int_distribution<px_t>  dpx(pxMin, pxMax);
  std::uniform_int_distribution<qty_t> dqty(qMin, qMax);
  std::uniform_int_distribution<int>   dside(0,1);
  std::uniform_int_distribution<int>   dtype(0,99);   // 控制new:amend:del比例

  id_t nextId=1;
  size_t live = 0;
  evts.resize(N);
  for (size_t i=0;i<N;i++){
    int t = dtype(rng);
    if (live<1000 || t<60) {              // ~60% new
      evts[i] = {0,nextId++, dpx(rng), dqty(rng), dside(rng)?Side::Sell:Side::Buy};
      live++;
    } else if (t<85) {                    // ~25% amend
      id_t id = 1 + (rng()%(nextId-1));
      int32_t delta = (rng()&1)? int32_t(dqty(rng)) : -int32_t(dqty(rng));
      evts[i] = {1,id,0,(qty_t)delta,Side::Buy};
    } else {                              // ~15% delete
      id_t id = 1 + (rng()%(nextId-1));
      evts[i] = {2,id,0,0,Side::Buy};
      if (live>0) live--;
    }
  }

  // 延迟直方图（纳秒），简易分桶：0-1000ns，>1000归最后桶
  const int BINS=101;
  std::vector<uint64_t> hist(BINS,0);
  auto t0 = clk::now();

  // 批处理计时：按块减少时钟读取开销
  const size_t BATCH = 8192;
  uint64_t total_ns=0;
  size_t processed=0;

  for (size_t i=0;i<N;i+=BATCH){
    auto t1 = clk::now();
    size_t end = std::min(N, i+BATCH);
    for (size_t j=i;j<end;++j){
      auto &e = evts[j];
      if (e.type==0) ob.newOrder(Order{e.id,e.px,e.qty,e.side});
      else if (e.type==1) ob.amendOrder(e.id, (int32_t)e.qty);
      else ob.deleteOrder(e.id);
    }
    auto t2 = clk::now();
    uint64_t ns = std::chrono::duration_cast<std::chrono::nanoseconds>(t2-t1).count();
    total_ns += ns;
    processed += (end-i);
    // 将批均延迟粗分桶
    uint64_t per = ns/ (end-i);
    int b = (per<1000)? (int)per : 1000;
    b = std::min(b, BINS-1);
    hist[b]++;
  }
  auto tAll = clk::now();
  auto wall_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(tAll-t0).count();

  double avg_ns = double(total_ns)/double(processed);
  double mops   = (double(processed)/ (double(wall_ns)*1e-9))/1e6;
  std::printf("Processed: %zu, wall=%.3f s, avg=%.2f ns/op, rate=%.2f Mops/s\n",
              processed, wall_ns*1e-9, avg_ns, mops);

  // 顶簿查询基准（示例：100万次）
  volatile px_t sink=0; volatile qty_t sinkq=0;
  auto q1 = clk::now();
  for (int i=0;i<1'000'000;i++){
    px_t p; auto pl = ob.topOfBook((i&1)?Side::Buy:Side::Sell, &p);
    sink ^= p; sinkq ^= pl.totalQty;
  }
  auto q2 = clk::now();
  auto qns = std::chrono::duration_cast<std::chrono::nanoseconds>(q2-q1).count();
  std::printf("TopOfBook avg=%.2f ns\n", double(qns)/1'000'000.0);

  // 打印直方图关键分位（中位、P90、P99）
  auto cum = 0ull; auto total_batches = 0ull;
  for (auto h:hist) total_batches += h;
  auto p50=total_batches/2, p90=total_batches*9/10, p99=total_batches*99/100;
  int m50=0,m90=0,m99=0;
  for (int i=0;i<BINS;i++){
    cum += hist[i];
    if (!m50 && cum>=p50) m50=i;
    if (!m90 && cum>=p90) m90=i;
    if (!m99 && cum>=p99) m99=i;
  }
  std::printf("Latency per-op (batch-avg) ~ P50=%dns, P90=%dns, P99=%dns\n", m50, m90, m99);
  return int(sink^sinkq); // 防止优化器移除
}
