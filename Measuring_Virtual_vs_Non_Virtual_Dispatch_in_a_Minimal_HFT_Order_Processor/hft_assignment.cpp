// hft_assignment.cpp
#include <bits/stdc++.h>
using namespace std;

// Order structure
struct Order {
    uint64_t id;
    int side;   // 0 or 1
    int qty;
    int price;
    int payload[2];
};

// Configuration
uint64_t orders_num = 800000;
int repeats = 12;
uint64_t warmup = 1000000;
string out_csv = "hft_assignment.csv";

void Configuration_change(uint64_t orders, int reps, uint64_t warm, const string& csv){
    orders_num = orders;
    repeats = reps;
    warmup = warm;
    out_csv = csv;
}

// Generate the orders
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

// Three patterns for assigning strategies to orders
enum class Pattern { 
    HomogA, 
    Mixed50, 
    Bursty64_16 
};

static const char* pat_name(Pattern p){
    switch(p){
        case Pattern::HomogA: return "homog_A";
        case Pattern::Mixed50: return "mixed_50_50";
        case Pattern::Bursty64_16: return "bursty_64A16B";
    }
    return "unknown";
}

// Make assignment vector according to pattern
// 0 for Strategy A, 1 for Strategy B
static vector<uint8_t> make_assign(Pattern p, uint64_t N){
    vector<uint8_t> asg(N,0);
    if(p==Pattern::HomogA) {
        // All A
        return asg;
    } else if(p==Pattern::Mixed50) {
        // 50/50 random, fixed seed
        std::mt19937 rng(424242u);
        std::bernoulli_distribution b(0.5);
        for(uint64_t i=0;i<N;i++) asg[i] = b(rng) ? 1u : 0u;
    } else {
        // Bursty: 64 A then 16 B, repeat
        const int A=64,B=16, P=A+B;
        for(uint64_t i=0;i<N;i++){
            int r = (int)(i % P);
            asg[i] = (r < A) ? 0u : 1u;
        }
    }
    return asg;
}

// Use static thread_local members to simulate L1/L2 cache activity
struct Book {
    static thread_local uint32_t t1[64];
    static thread_local uint32_t t2[64];
    static thread_local uint64_t ctr;
};
thread_local uint32_t Book::t1[64]={0};
thread_local uint32_t Book::t2[64]={0};
thread_local uint64_t Book::ctr=0;

// Virtual implementation
class Processor {
public:
    virtual ~Processor() = default;
    virtual uint64_t process(Order& o) = 0;
};

class StrategyA_V : public Processor {
public:
    // Operations per order:
    uint64_t process(Order& o) override {
        auto& t1 = Book::t1; 
        auto& t2 = Book::t2; 
        auto& ctr = Book::ctr;
        uint64_t x = o.id ^ (uint64_t)(o.price * (o.side ? 3 : 5));
        x += (uint64_t)o.qty;
        // Write to t1, t2
        uint32_t i1 = (uint32_t)(o.id) & 63;
        uint32_t i2 = ((uint32_t)o.id >> 6) & 63;
        t1[i1] = (uint32_t)(t1[i1] + (uint32_t)o.price) ^ (uint32_t)o.qty;
        t2[i2] ^= (uint32_t)(o.payload[0] + 3);
        // Small branch
        if ((o.qty & 1)==0) { 
            ctr++; 
        } 
        else { 
            t1[i1] += 1u; 
        }
        // Summary check
        x = (x ^ t1[i1]) + t2[i2] + ctr;
        return x;
    }
};

struct StrategyB_V : Processor {
    uint64_t process(Order& o) override {
        auto& t1 = Book::t1; 
        auto& t2 = Book::t2; 
        auto& ctr = Book::ctr;
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

// Non-virtual implementation
class StrategyA_NV {
public:
    uint64_t run(Order& o){
        auto& t1 = Book::t1; 
        auto& t2 = Book::t2; 
        auto& ctr = Book::ctr;
        uint64_t x = o.id ^ (uint64_t)(o.price * (o.side ? 3 : 5));
        x += (uint64_t)o.qty;
        uint32_t i1 = (uint32_t)(o.id) & 63;
        uint32_t i2 = ((uint32_t)o.id >> 6) & 63;
        t1[i1] = (uint32_t)(t1[i1] + (uint32_t)o.price) ^ (uint32_t)o.qty;
        t2[i2] ^= (uint32_t)(o.payload[0] + 3);
        if ((o.qty & 1)==0) { 
            ctr++; 
        } else { 
            t1[i1] += 1u; 
        }
        x = (x ^ t1[i1]) + t2[i2] + ctr;
        return x;
    }
};
class StrategyB_NV {
public:
    uint64_t run(Order& o){
        auto& t1 = Book::t1; auto& t2 = Book::t2; auto& ctr = Book::ctr;
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

// Result of a run
struct RunResult { 
    uint64_t ns; 
    double ops_per_sec; 
    uint64_t checksum; 
};

static volatile uint64_t g_sink = 0;

RunResult time_loop(std::function<uint64_t(Order&, uint8_t)> f, vector<Order>& orders, const vector<uint8_t>& asg) {
    using clk = std::chrono::high_resolution_clock;
    g_sink = 0;
    auto t0 = clk::now();
    for (size_t i = 0; i < orders.size(); i++) {
        g_sink += f(orders[i], asg[i]);
    }
    auto t1 = clk::now();
    uint64_t ns = std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();
    double ops = (double)orders.size() * 1e9 / (double)ns;
    return { ns, ops, (uint64_t)g_sink };
}

// Virtual and non-virtual runners
uint64_t process_virtual(Order& o, uint8_t which, Processor* arr[2]) {
    return arr[which]->process(o);
}
uint64_t process_nonvirtual(Order& o, uint8_t which, StrategyA_NV* a, StrategyB_NV* b) {
    return (which == 0) ? a->run(o) : b->run(o);
}

RunResult run_virtual(vector<Order>& orders, const vector<uint8_t>& asg){
    StrategyA_V a; 
    StrategyB_V b;
    Processor* arr[2] = { &a, &b };
    auto fun = std::bind(process_virtual, 
        std::placeholders::_1, std::placeholders::_2, arr);
    return time_loop(fun, orders, asg);
}

RunResult run_nonvirtual(vector<Order>& orders, const vector<uint8_t>& asg){
    StrategyA_NV a; StrategyB_NV b;
    auto fun = std::bind(process_nonvirtual,
        std::placeholders::_1, std::placeholders::_2, &a, &b);
    return time_loop(fun, orders, asg);
}


// warm up
void do_warmup(uint64_t warm_ops){
    auto ord = make_orders(warm_ops);
    auto asg = make_assign(Pattern::Mixed50, warm_ops);
    (void) run_virtual(ord, asg);
    (void) run_nonvirtual(ord, asg);
}


int main(){
    // Configuration
    // orders_num = 800000;
    // repeats = 12;
    // warmup = 1000000;
    // out_csv = "hft_assignment.csv";

    // Use Configuration_change to modify parameters if needed
    // Configuration_change(1000000, 10, 500000, "output.csv");

    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    do_warmup(warmup);

    unique_ptr<ofstream> fout;
    ostream* out = &cout;
    if(!out_csv.empty()){
        fout = make_unique<ofstream>(out_csv);
        if(!fout->is_open()){ cerr<<"Cannot open "<<out_csv<<"\n"; return 1; }
        out = fout.get();
    }
    (*out) << "pattern,impl,repeat,orders,elapsed_ns,ops_per_sec,checksum\n";

    vector<Pattern> patterns = { Pattern::HomogA, 
        Pattern::Mixed50, Pattern::Bursty64_16 };
    
    for(auto p: patterns){
        auto orders = make_orders(orders_num);
        auto asg    = make_assign(p, orders_num);

        // Virtual implementation
        vector<uint64_t> elapsed_ns_v;
        for(int r=0;r<repeats;r++){
            auto rr = run_virtual(orders, asg);
            elapsed_ns_v.push_back(rr.ns);
            (*out) << pat_name(p) << ",virtual," << r << ","
                   << orders_num << "," << rr.ns << "," << fixed << setprecision(3)
                   << rr.ops_per_sec << "," << rr.checksum << "\n";
        }
        std::sort(elapsed_ns_v.begin(), elapsed_ns_v.end());
        uint64_t best_v = elapsed_ns_v.front();
        uint64_t median_v = elapsed_ns_v[elapsed_ns_v.size()/2];
        (*out) << pat_name(p) << ",virtual,best,-,-," << best_v << ",-,-\n";
        (*out) << pat_name(p) << ",virtual,median,-,-," << median_v << ",-,-\n";

        // Non-virtual implementation
        vector<uint64_t> elapsed_ns_nv;
        for(int r=0;r<repeats;r++){
            auto rr = run_nonvirtual(orders, asg);
            elapsed_ns_nv.push_back(rr.ns);
            (*out) << pat_name(p) << ",nonvirtual," << r << ","
                   << orders_num << "," << rr.ns << "," << fixed << setprecision(3)
                   << rr.ops_per_sec << "," << rr.checksum << "\n";
        }
        std::sort(elapsed_ns_nv.begin(), elapsed_ns_nv.end());
        uint64_t best_nv = elapsed_ns_nv.front();
        uint64_t median_nv = elapsed_ns_nv[elapsed_ns_nv.size()/2];
        (*out) << pat_name(p) << ",nonvirtual,best,-,-," << best_nv << ",-,-\n";
        (*out) << pat_name(p) << ",nonvirtual,median,-,-," << median_nv << ",-,-\n";
    }
    return 0;
}
