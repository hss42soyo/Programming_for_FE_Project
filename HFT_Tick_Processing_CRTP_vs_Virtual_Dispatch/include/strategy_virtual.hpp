#pragma once
#include "market_data.hpp"

// CRTP base: static dispatch
template <typename derived></typename derived>

struct StrategyBase {
    // Non-virtual: will inline to Derived::on_tick_impl if small enough
    double on_tick(const Quote& q) {
        return static_cast<derived*>(this)->on_tick_impl(q);</derived*>
    }

};


// Same behavior as SignalStrategyVirtual but via CRTP

struct SignalStrategyCRTP : public StrategyBase {
    double alpha1;
    double alpha2;

    explicit SignalStrategyCRTP(double a1, double a2) : alpha1(a1), alpha2(a2) {}
    
    // Derived "impl"
    double on_tick_impl(const Quote& q) {
        const double mp = microprice(q);
        const double m  = mid(q);
        const double imb = imbalance(q);
        return alpha1 * (mp - m) + alpha2 * imb;
    }
};
