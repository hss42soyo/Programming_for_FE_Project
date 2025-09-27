// hft_assignment.cpp
#include <bits/stdc++.h>
using namespace std;

// Order structure representing a trading order
struct Order {
    uint64_t id;        // Unique order ID
    int side;           // 0 or 1 (buy/sell)
    int qty;            // Order quantity
    int price;          // Order price
    int payload[2];     // Additional data payload
};

// Configuration parameters
uint64_t orders_num = 800000;   // Number of orders to process
int repeats = 12;               // Number of benchmark repeats
uint64_t warmup = 1000000;      // Number of warmup operations
string out_csv = "hft_assignment.csv"; // Output CSV file name

// Function to change configuration parameters
void Configuration_change(uint64_t orders, int reps, uint64_t warm, const string& csv){
    orders_num = orders;
    repeats = reps;
    warmup = warm;
    out_csv = csv;
}

// Generate random orders for testing
static vector<Order> make_orders(uint64_t N){
    vector<Order> v;
    v.resize(N);
    std::mt19937 rng(12345u); // Fixed seed for reproducibility
    std::uniform_int_distribution<int> uq(1,127), up(10000,20000), upay(0,1000);
    
    for(uint64_t i=0;i<N;i++){
        v[i].id = i+1;
        v[i].side = (int)(rng() & 1u);  // Random 0 or 1
        v[i].qty = uq(rng);             // Random quantity
        v[i].price = up(rng);           // Random price
        v[i].payload[0] = upay(rng);    // Random payload data
        v[i].payload[1] = upay(rng);
    }
    return v;
}

// Three patterns for assigning strategies to orders
enum class Pattern { 
    HomogA,         // All orders use Strategy A
    Mixed50,        // 50% Strategy A, 50% Strategy B (random)
    Bursty64_16     // 64 orders Strategy A, then 16 orders Strategy B, repeat
};

// Convert pattern enum to string name
static const char* pat_name(Pattern p){
    switch(p){
        case Pattern::HomogA: return "homog_A";
        case Pattern::Mixed50: return "mixed_50_50";
        case Pattern::Bursty64_16: return "bursty_64A16B";
    }
    return "unknown";
}

// Create assignment vector based on pattern
// Returns vector where 0 = Strategy A, 1 = Strategy B
static vector<uint8_t> make_assign(Pattern p, uint64_t N){
    vector<uint8_t> asg(N,0); // Default all to Strategy A
    
    if(p==Pattern::HomogA) {
        // All orders use Strategy A - already initialized to 0
        return asg;
    } else if(p==Pattern::Mixed50) {
        // 50/50 random assignment with fixed seed
        std::mt19937 rng(424242u);
        std::bernoulli_distribution b(0.5);
        for(uint64_t i=0;i<N;i++) {
            asg[i] = b(rng) ? 1u : 0u;
        }
    } else {
        // Bursty pattern: 64 A orders, then 16 B orders, repeat
        const int A=64, B=16, P=A+B;
        for(uint64_t i=0;i<N;i++){
            int r = (int)(i % P);
            asg[i] = (r >= A) ? 1u : 0u;
        }
    }
    return asg;
}

// Simulate cache activity using thread-local storage
struct Book {
    static thread_local uint32_t t1[64];   // Simulate L1 cache table
    static thread_local uint32_t t2[64];   // Simulate L2 cache table  
    static thread_local uint64_t ctr;      // Global counter
};
thread_local uint32_t Book::t1[64]={0};
thread_local uint32_t Book::t2[64]={0};
thread_local uint64_t Book::ctr=0;

// Reset the shared per-run state so checksums are comparable
static inline void reset_book() {
    // no extra headers needed; keep it simple
    for (int i = 0; i < 64; ++i) {
        Book::t1[i] = 0u;
        Book::t2[i] = 0u;
    }
    Book::ctr = 0;
}

// Abstract base class for processors
class Processor {
public:
    virtual ~Processor() {}
    virtual uint64_t process(Order& o) = 0; // Pure virtual function
};

// Virtual implementation - Strategy A
class StrategyA_V : public Processor {
public:
    // Process order using Strategy A algorithm
    uint64_t process(Order& o) {
        auto& t1 = Book::t1; 
        auto& t2 = Book::t2; 
        auto& ctr = Book::ctr;
        
        // Initial computation based on order fields
        uint64_t x = o.id ^ (uint64_t)(o.price * (o.side ? 3 : 5));
        x += (uint64_t)o.qty;
        
        // Memory operations to simulate cache activity
        uint32_t i1 = (uint32_t)(o.id) & 63;        // Index into t1 array
        uint32_t i2 = ((uint32_t)o.id >> 6) & 63;   // Index into t2 array
        t1[i1] = (uint32_t)(t1[i1] + (uint32_t)o.price) ^ (uint32_t)o.qty;
        t2[i2] ^= (uint32_t)(o.payload[0] + 3);
        
        // Conditional branch to simulate real trading logic
        if ((o.qty & 1)==0) { 
            ctr++; 
        } else { 
            t1[i1] += 1u; 
        }
        
        // Final computation combining all elements
        x = (x ^ t1[i1]) + t2[i2] + ctr;
        return x;
    }
};

// Virtual implementation - Strategy B
class StrategyB_V : public Processor {
public:
    // Process order using Strategy B algorithm
    uint64_t process(Order& o) {
        auto& t1 = Book::t1; 
        auto& t2 = Book::t2; 
        auto& ctr = Book::ctr;
        
        // Different computation pattern from Strategy A
        uint64_t x = (o.id * 1315423911ull) ^ (uint64_t)(o.price - o.qty);
        
        // Different indexing pattern
        uint32_t i1 = (uint32_t)(o.price + o.qty) & 63;
        uint32_t i2 = (uint32_t)(o.id + (uint32_t)o.side) & 63;
        
        // Different memory operations
        t1[i1] = (uint32_t)(t1[i1] * 1664525u + 1013904223u); // Linear congruential generator
        t2[i2] += (uint32_t)(o.payload[1] ^ (uint32_t)o.side);
        
        // Different conditional logic
        if (o.side) { 
            ctr++; 
        } else { 
            t2[i2] ^= t1[i1]; 
        }
        
        // Final computation
        x ^= (uint64_t)t1[i1] + t2[i2] + ctr;
        return x;
    }
};

// Non-virtual implementation - Strategy A (same logic as virtual version)
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

// Non-virtual implementation - Strategy B (same logic as virtual version)
class StrategyB_NV {
public:
    uint64_t run(Order& o){
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

// Structure to hold benchmark results
struct RunResult { 
    uint64_t ns;            // Elapsed time in nanoseconds
    double ops_per_sec;     // Operations per second
    uint64_t checksum;      // Checksum to prevent compiler optimization
};

// Global sink to prevent dead code elimination
static volatile uint64_t g_sink = 0;

// Benchmark virtual dispatch implementation
RunResult run_virtual(vector<Order>& orders, const vector<uint8_t>& asg) {
    reset_book(); // Reset shared state before run
    using clk = std::chrono::high_resolution_clock;
    
    // Create strategy instances
    Processor* strategyA = new StrategyA_V();
    Processor* strategyB = new StrategyB_V();
    
    g_sink = 0;
    auto t0 = clk::now();
    
    // Process each order with assigned strategy
    for (size_t i = 0; i < orders.size(); i++) {
        if (asg[i] == 0) {
            g_sink += strategyA->process(orders[i]);  // Use Strategy A
        } else {
            g_sink += strategyB->process(orders[i]);  // Use Strategy B
        }
    }
    
    auto t1 = clk::now();
    uint64_t ns = std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();
    double ops = (double)orders.size() * 1e9 / (double)ns;
    return { ns, ops, (uint64_t)g_sink };
}

// Benchmark non-virtual dispatch implementation
RunResult run_nonvirtual(vector<Order>& orders, const vector<uint8_t>& asg) {
    reset_book(); // Reset shared state before run
    using clk = std::chrono::high_resolution_clock;
    
    // Create strategy instances
    StrategyA_NV strategyA;
    StrategyB_NV strategyB;
    
    g_sink = 0;
    auto t0 = clk::now();
    
    // Process each order with assigned strategy
    for (size_t i = 0; i < orders.size(); i++) {
        if (asg[i] == 0) {
            g_sink += strategyA.run(orders[i]);      // Use Strategy A
        } else {
            g_sink += strategyB.run(orders[i]);      // Use Strategy B
        }
    }
    
    auto t1 = clk::now();
    uint64_t ns = std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();
    double ops = (double)orders.size() * 1e9 / (double)ns;
    return { ns, ops, (uint64_t)g_sink };
}

// Warm up the CPU and memory system before benchmarking
void do_warmup(uint64_t warm_ops){
    auto ord = make_orders(warm_ops);
    auto asg = make_assign(Pattern::Mixed50, warm_ops);
    (void) run_virtual(ord, asg);      // Warm up virtual dispatch
    (void) run_nonvirtual(ord, asg);   // Warm up non-virtual dispatch
}

int main(){
    // Set up I/O for better performance
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    // Warm up system before actual benchmarking
    do_warmup(warmup);

    // Set up output stream (file or cout)
    unique_ptr<ofstream> fout;
    ostream* out = &cout;
    if(!out_csv.empty()){
        fout = make_unique<ofstream>(out_csv);
        if(!fout->is_open()){ 
            cerr << "Cannot open " << out_csv << "\n"; 
            return 1; 
        }
        out = fout.get();
    }
    
    // Write CSV header
    (*out) << "pattern,impl,repeat,orders,elapsed_ns,ops_per_sec,checksum\n";

    // Test all three assignment patterns
    vector<Pattern> patterns = { Pattern::HomogA, Pattern::Mixed50, Pattern::Bursty64_16 };
    
    // Ensure at least 10 repeats as required
    int actual_repeats = max(repeats, 10);
    
    for(auto p: patterns){
        // Generate orders and assignment pattern
        auto orders = make_orders(orders_num);
        auto asg = make_assign(p, orders_num);

        // Test virtual dispatch implementation
        vector<uint64_t> elapsed_ns_v;
        for(int r = 0; r < actual_repeats; r++){
            auto rr = run_virtual(orders, asg);
            elapsed_ns_v.push_back(rr.ns);
            (*out) << pat_name(p) << ",virtual," << r << ","
                   << orders_num << "," << rr.ns << "," << fixed << setprecision(3)
                   << rr.ops_per_sec << "," << rr.checksum << "\n";
        }
        
        // Calculate and output best and median for virtual dispatch
        sort(elapsed_ns_v.begin(), elapsed_ns_v.end());
        uint64_t best_v = elapsed_ns_v.front();
        uint64_t median_v = elapsed_ns_v[elapsed_ns_v.size()/2];
        uint64_t average_v = accumulate(elapsed_ns_v.begin(), elapsed_ns_v.end(), 0ull) / elapsed_ns_v.size();
        (*out) << pat_name(p) << ",virtual,best,-,-," << best_v << ",-,-\n";
        (*out) << pat_name(p) << ",virtual,median,-,-," << median_v << ",-,-\n";
        (*out) << pat_name(p) << ",virtual,average,-,-," << average_v << ",-,-\n";

        // Test non-virtual dispatch implementation
        vector<uint64_t> elapsed_ns_nv;
        for(int r = 0; r < actual_repeats; r++){
            auto rr = run_nonvirtual(orders, asg);
            elapsed_ns_nv.push_back(rr.ns);
            (*out) << pat_name(p) << ",nonvirtual," << r << ","
                   << orders_num << "," << rr.ns << "," << fixed << setprecision(3)
                   << rr.ops_per_sec << "," << rr.checksum << "\n";
        }
        
        // Calculate and output best and median for non-virtual dispatch
        sort(elapsed_ns_nv.begin(), elapsed_ns_nv.end());
        uint64_t best_nv = elapsed_ns_nv.front();
        uint64_t median_nv = elapsed_ns_nv[elapsed_ns_nv.size()/2];
        uint64_t average_nv = accumulate(elapsed_ns_nv.begin(), elapsed_ns_nv.end(), 0ull) / elapsed_ns_nv.size();
        (*out) << pat_name(p) << ",nonvirtual,best,-,-," << best_nv << ",-,-\n";
        (*out) << pat_name(p) << ",nonvirtual,median,-,-," << median_nv << ",-,-\n";
        (*out) << pat_name(p) << ",nonvirtual,average,-,-," << average_nv << ",-,-\n";
    }
    
    return 0;
}