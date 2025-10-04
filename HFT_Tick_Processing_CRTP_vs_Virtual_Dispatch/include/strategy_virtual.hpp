#pragma once
#include "market_data.hpp"


struct IStrategy {
    // Non-virtual: will inline to Derived::on_tick_impl if small enough
    virtual double on_tick(const Quote& q) = 0;
    virtual ~IStrategy() = default;
};

// Same behavior as SignalStrategyVirtual but via CRTP

struct SignalStrategyVirtual : public IStrategy {
    double alpha1;
    double alpha2;

    explicit SignalStrategyVirtual(double a1, double a2) : alpha1(a1), alpha2(a2) {}

    double on_tick(const Quote& q) override{
        const double mp = microprice(q);
        const double m  = mid(q);
        const double imb = imbalance(q);
        return alpha1 * (mp - m) + alpha2 * imb;
    }
};
