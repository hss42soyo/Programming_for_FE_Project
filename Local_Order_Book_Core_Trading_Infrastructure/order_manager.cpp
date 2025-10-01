#include "order_manager.h"
#include <iostream>

int OrderManager::place_order(Side side, double price, int qty) {
    static int next_id = 1;
    auto order = std::make_unique<MyOrder>();
    order->id = next_id++;
    order->side = side;
    order->price = price;
    order->quantity = qty;
    int order_id = order->id;
    orders[order->id] = std::move(order);
    std::cout << "[Strategy] Placing " << (side == Side::Buy ? "BUY" : "SELL") 
              << " order at " << price << " x " << qty <<" ( ID = "<< order_id << " )\n";
    return order_id;
}

void OrderManager::cancel(int id) {
    auto it = orders.find(id);
    if (it != orders.end()) {
        it->second->status = OrderStatus::Cancelled;
        orders.erase(it);
        std::cout << "[Order] Order " << id << " cancelled\n";
    }
}

void OrderManager::handle_fill(int id, int filled_qty) {
    auto it = orders.find(id);
    if (it != orders.end()) {
        std::cout << "[Execution] Order " << id << " filled: " << filled_qty << "\n";
        it->second->filled += filled_qty;
        if (it->second->filled >= it->second->quantity) {
            it->second->status = OrderStatus::Filled;
            it->second->filled = it->second->quantity; // Cap filled to quantity
            int qty = it->second->quantity;
            orders.erase(it); // Remove filled orders
            std::cout << "[Order] Order " << id << " completed (" << qty << "/" << qty << ") and removed\n";
        } 
        else {
            it->second->status = OrderStatus::PartiallyFilled;
            std::cout << "[Order] Order " << id << " partially filled: " << it->second->filled << "/" << it->second->quantity << "\n";
        }
    }
}

void OrderManager::print_active_orders() const {
    for (const auto& [id, order] : orders) {
        if (order->status == OrderStatus::New || order->status == OrderStatus::PartiallyFilled) {
            std::cout << "Order ID: " << order->id
                      << ", Side: " << (order->side == Side::Buy ? "Buy" : "Sell")
                      << ", Price: " << order->price
                      << ", Quantity: " << order->quantity
                      << ", Filled: " << order->filled
                      << ", Status: " << (order->status == OrderStatus::New ? "New" : "PartiallyFilled")
                      << std::endl;
        }
    }
}