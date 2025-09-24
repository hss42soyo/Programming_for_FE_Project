// hft_assignment.cpp
// 目的：测量虚函数派发 vs 非虚派发在类HFT内环中的吞吐差异
// 说明：单线程；三种流型；预热≥1e6；重复≥10；CSV到stdout或文件
#include <bits/stdc++.h>
using namespace std;

// ===== 订单结构（按题目精确布局） =====
struct Order {
    uint64_t id;
    int      side;   // 0 or 1
    int      qty;
    int      price;
    int      payload[2];
};

// ===== 小工具：计时/CSV/参数 =====
struct Args {
    uint64_t orders = 800000; // 默认N：~0.5-2s/次（视机器而定）
    int repeats = 12;
    uint64_t warmup = 1000000;
    string out_csv = ""; // 为空则stdout
};
static inline bool starts_with(const string& s, const char* p){ return s.rfind(p,0)==0; }

Args parse_args(int argc, char** argv){
    Args a;
    for(int i=1;i<argc;i++){
        string s=argv[i];
        auto need = [&](int i){ if(i+1>=argc) { fprintf(stderr,"Missing value for %s\n", s.c_str()); exit(1);} };
        if(starts_with(s,"--orders")){ need(i); a.orders = strtoull(argv[++i],nullptr,10); }
        else if(starts_with(s,"--repeats")){ need(i); a.repeats = atoi(argv[++i]); }
        else if(starts_with(s,"--warmup")){ need(i); a.warmup = strtoull(argv[++i],nullptr,10); }
        else if(starts_with(s,"--csv")){ need(i); a.out_csv = argv[++i]; }
        else { fprintf(stderr,"Unknown arg: %s\n", s.c_str()); exit(1); }
    }
    return a;
}

// ===== 订单生成（确定性随机，保证可复现）=====
static vector<Order> make_orders(uint64_t N){
    vector<Order> v; v.resize(N);
    std::mt19937 rng(12345u);
    std::uniform_int_distribution<int> uq(1,127), up(10000,20000), upay(0,1000);
    for(uint64_t i=0;i<N;i++){
        v[i].id = i+1;
        v[i].side = (int)(rng() & 1u);
        v[i].qty = uq(rng);
        v[i].price = up(rng);
        v[i].payload[0] = upay(rng);
        v[i].payload[1] = upay(rng);
    }
    return v;
}

// ===== 三种流型的策略指派序列（0=A, 1=B）=====
enum class Pattern { HomogA, Mixed50, Bursty64_16 };
static vector<uint8_t> make_assign(Pattern p, uint64_t N){
    vector<uint8_t> asg(N,0);
    if(p==Pattern::HomogA) {
        // 全A
        return asg;
    } else if(p==Pattern::Mixed50) {
        // 50/50 随机，固定种子
        std::mt19937 rng(424242u);
        std::bernoulli_distribution b(0.5);
        for(uint64_t i=0;i<N;i++) asg[i] = b(rng) ? 1u : 0u;
    } else {
        // 64个A，16个B循环
        const int A=64,B=16, P=A+B;
        for(uint64_t i=0;i<N;i++){
            int r = (int)(i % P);
            asg[i] = (r < A) ? 0u : 1u;
        }
    }
    return asg;
}

// ===== L1/L2模拟：小表写入（线程局部，固定小尺寸）=====
template<int SZ=64>
struct Book {
    // 使用两个小表，模拟订单簿更新的两处写
    static thread_local uint32_t t1[SZ];
    static thread_local uint32_t t2[SZ];
    static thread_local uint64_t ctr;
};
template<int SZ> thread_local uint32_t Book<SZ>::t1[SZ]={0};
template<int SZ> thread_local uint32_t Book<SZ>::t2[SZ]={0};
template<int SZ> thread_local uint64_t Book<SZ>::ctr=0;

// ===== 虚函数实现 =====
struct Processor {
    virtual ~Processor() = default;
    virtual uint64_t process(Order& o) = 0; // 纯虚
};

struct StrategyA_V : Processor {
    // 与NV版本保持**完全相同行为**
    uint64_t process(Order& o) override {
        auto& t1 = Book<64>::t1; auto& t2 = Book<64>::t2; auto& ctr = Book<64>::ctr;
        // 少量整数运算
        uint64_t x = o.id ^ (uint64_t)(o.price * (o.side ? 3 : 5));
        x += (uint64_t)o.qty;
        // 两次小表写（产生L1活动）
        uint32_t i1 = (uint32_t)(o.id) & 63;
        uint32_t i2 = ((uint32_t)o.id >> 6) & 63;
        t1[i1] = (uint32_t)(t1[i1] + (uint32_t)o.price) ^ (uint32_t)o.qty;
        t2[i2] ^= (uint32_t)(o.payload[0] + 3);
        // 小分支
        if ((o.qty & 1)==0) { ctr++; } else { t1[i1] += 1u; }
        // 汇总校验
        x = (x ^ t1[i1]) + t2[i2] + ctr;
        return x;
    }
};

struct StrategyB_V : Processor {
    uint64_t process(Order& o) override {
        auto& t1 = Book<64>::t1; auto& t2 = Book<64>::t2; auto& ctr = Book<64>::ctr;
        uint64_t x = (o.id * 1315423911ull) ^ (uint64_t)(o.price - o.qty);
        uint32_t i1 = (uint32_t)(o.price + o.qty) & 63;
        uint32_t i2 = (uint32_t)(o.id + (uint32_t)o.side) & 63;
        t1[i1] = (uint32_t)(t1[i1] * 1664525u + 1013904223u); // LCG一步
        t2[i2] += (uint32_t)(o.payload[1] ^ (uint32_t)o.side);
        if (o.side) { ctr++; } else { t2[i2] ^= t1[i1]; }
        x ^= (uint64_t)t1[i1] + t2[i2] + ctr;
        return x;
    }
};

// ===== 非虚实现（与虚版**完全相同行为**）=====
struct StrategyA_NV {
    uint64_t run(Order& o){
        auto& t1 = Book<64>::t1; auto& t2 = Book<64>::t2; auto& ctr = Book<64>::ctr;
        uint64_t x = o.id ^ (uint64_t)(o.price * (o.side ? 3 : 5));
        x += (uint64_t)o.qty;
        uint32_t i1 = (uint32_t)(o.id) & 63;
        uint32_t i2 = ((uint32_t)o.id >> 6) & 63;
        t1[i1] = (uint32_t)(t1[i1] + (uint32_t)o.price) ^ (uint32_t)o.qty;
        t2[i2] ^= (uint32_t)(o.payload[0] + 3);
        if ((o.qty & 1)==0) { ctr++; } else { t1[i1] += 1u; }
        x = (x ^ t1[i1]) + t2[i2] + ctr;
        return x;
    }
};
struct StrategyB_NV {
    uint64_t run(Order& o){
        auto& t1 = Book<64>::t1; auto& t2 = Book<64>::t2; auto& ctr = Book<64>::ctr;
        uint64_t x = (o.id * 1315423911ull) ^ (uint64_t)(o.price - o.qty);
        uint32_t i1 = (uint32_t)(o.price + o.qty) & 63;
        uint32_t i2 = (uint32_t)(o.id + (uint32_t)o.side) & 63;
        t1[i1] = (uint32_t)(t1[i1] * 1664525u + 1013904223u);
        t2[i2] += (uint32_t)(o.payload[1] ^ (uint32_t)o.side);
        if (o.side) { ctr++; } else { t2[i2] ^= t1[i1]; }
        x ^= (uint64_t)t1[i1] + t2[i2] + ctr;
        return x;
    }
};

// ===== 运行一次：返回<elapsed_ns, ops/sec, checksum> =====
struct RunResult { uint64_t ns; double ops_per_sec; uint64_t checksum; };
static volatile uint64_t g_sink = 0; // 防止优化去除

template<typename F>
RunResult time_loop(F&& f, vector<Order>& orders, const vector<uint8_t>& asg){
    using clk = std::chrono::high_resolution_clock;
    g_sink = 0;
    auto t0 = clk::now();
    for(size_t i=0;i<orders.size();i++){
        g_sink += f(orders[i], asg[i]);
    }
    auto t1 = clk::now();
    uint64_t ns = std::chrono::duration_cast<std::chrono::nanoseconds>(t1-t0).count();
    double ops = (double)orders.size() * 1e9 / (double)ns;
    return { ns, ops, (uint64_t)g_sink };
}

// ===== 每种实现的调用包装 =====
RunResult run_virtual(vector<Order>& orders, const vector<uint8_t>& asg){
    StrategyA_V a; StrategyB_V b;
    Processor* arr[2] = { &a, &b };
    auto fun = [&](Order& o, uint8_t which)->uint64_t{
        return arr[which]->process(o); // 虚调用
    };
    return time_loop(fun, orders, asg);
}

RunResult run_nonvirtual(vector<Order>& orders, const vector<uint8_t>& asg){
    StrategyA_NV a; StrategyB_NV b;
    auto fun = [&](Order& o, uint8_t which)->uint64_t{
        // 简单 if/switch 派发（禁止模板/variant等）
        return (which==0) ? a.run(o) : b.run(o);
    };
    return time_loop(fun, orders, asg);
}

// ===== 名称帮助 =====
static const char* pat_name(Pattern p){
    switch(p){
        case Pattern::HomogA: return "homog_A";
        case Pattern::Mixed50: return "mixed_50_50";
        case Pattern::Bursty64_16: return "bursty_64A16B";
    }
    return "unknown";
}

// ===== 预热（对每种实现/模式做足量操作）=====
void do_warmup(uint64_t warm_ops){
    auto ord = make_orders(warm_ops);
    auto asg = make_assign(Pattern::Mixed50, warm_ops);
    (void) run_virtual(ord, asg);
    (void) run_nonvirtual(ord, asg);
}

// ===== 主流程 =====
int main(int argc, char** argv){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    Args args = parse_args(argc, argv);
    do_warmup(args.warmup);

    // 输出CSV（含表头）
    unique_ptr<ofstream> fout;
    ostream* out = &cout;
    if(!args.out_csv.empty()){
        fout = make_unique<ofstream>(args.out_csv);
        if(!fout->is_open()){ cerr<<"Cannot open "<<args.out_csv<<"\n"; return 1; }
        out = fout.get();
    }
    (*out) << "pattern,impl,repeat,orders,elapsed_ns,ops_per_sec,checksum\n";

    vector<Pattern> patterns = { Pattern::HomogA, Pattern::Mixed50, Pattern::Bursty64_16 };
    for(auto p: patterns){
        auto orders = make_orders(args.orders);
        auto asg    = make_assign(p, args.orders);

        // 虚函数实现
        for(int r=0;r<args.repeats;r++){
            auto rr = run_virtual(orders, asg);
            (*out) << pat_name(p) << ",virtual," << r << ","
                   << args.orders << "," << rr.ns << "," << fixed << setprecision(3)
                   << rr.ops_per_sec << "," << rr.checksum << "\n";
        }
        // 非虚实现
        for(int r=0;r<args.repeats;r++){
            auto rr = run_nonvirtual(orders, asg);
            (*out) << pat_name(p) << ",nonvirtual," << r << ","
                   << args.orders << "," << rr.ns << "," << fixed << setprecision(3)
                   << rr.ops_per_sec << "," << rr.checksum << "\n";
        }
    }
    return 0;
}
