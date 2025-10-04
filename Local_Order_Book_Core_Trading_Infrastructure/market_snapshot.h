#include <map>
#include <memory>

struct PriceLevel {
    double price;
    int quantity;

    PriceLevel(double p, int q) : price(p), quantity(q) {}
};

class MarketSnapshot {
public:
    std::map<double, std::unique_ptr<PriceLevel>, std::greater<double>> bids; // sorted descending
    std::map<double, std::unique_ptr<PriceLevel>, std::less<double>> asks; // sorted ascending

    void update_bid(double price, int qty);
    void update_ask(double price, int qty);
    const PriceLevel* get_best_bid() const;
    const PriceLevel* get_best_ask() const;
    void print(bool isBuy) const;
};