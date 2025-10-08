#pragma once
#include <cstdint>
#include <vector>

struct Quote {
    double bid;
    double ask;
    double bid_qty;
    double ask_qty;
};

// Pure math helpers (header-only, likely to inline)
inline double mid(const Quote& q) noexcept {
    return (q.bid + q.ask) * 0.5;
}


inline double microprice(const Quote& q) noexcept {
    const double denom = q.bid_qty + q.ask_qty;
    // Assumption: denom > 0 in our synthetic stream
    return (q.bid * q.ask_qty + q.ask * q.bid_qty) / denom;
}

inline double imbalance(const Quote& q) noexcept {
    const double denom = q.bid_qty + q.ask_qty;
    return (q.bid_qty - q.ask_qty) / denom;
}
