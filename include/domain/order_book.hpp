#pragma once
#include "core/trade.hpp"
#include "book_side.hpp"
#include <cstddef>
#include <unordered_map>
#include <vector>
#include <algorithm>

namespace mercury{
    class OrderBook {
        public :
            OrderBook() noexcept : bids_(Side::Buy), asks_(Side::Sell) {}
            [[nodiscard]] std::vector<Trade> submitOrder(Order&);
            bool cancelOrder(OrderID);
            [[nodiscard]] bool reduceOrder(OrderID, Quantity);
            OrderBook(const OrderBook&) = delete;
            OrderBook& operator=(const OrderBook&) = delete;
            OrderBook(OrderBook&&) = default;
            OrderBook& operator=(OrderBook&&) = default;
            [[nodiscard]] PriceLevel* bestBid() noexcept;
            [[nodiscard]] PriceLevel* bestAsk() noexcept;
            [[nodiscard]] const PriceLevel* bestBid() const noexcept;
            [[nodiscard]] const PriceLevel* bestAsk() const noexcept;
            [[nodiscard]] bool contains(OrderID) const noexcept;
            [[nodiscard]] bool empty() const noexcept;
            [[nodiscard]] std::size_t orderCount() const noexcept;
            [[nodiscard]] std::size_t peakActiveOrders() const noexcept ;
        private :
            BookSide bids_;
            BookSide asks_;
            std::size_t peak_active_orders_ = 0; // Tracks the peak number of active orders in the book at any point in time.
            // Maps every active resting order to its current location in the book.
            // Must remain synchronized with both BookSides.
            std::unordered_map<OrderID, Order*> order_lookup_;
    };
}