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
            // Processes an incoming order according to price-time priority.
            // Executes against the opposing side while the order crosses.
            // Any remaining quantity is inserted as a resting order.
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
        private :
            struct OrderEntry {
                OrderLocation location;
            };
            BookSide bids_;
            BookSide asks_;
            // Maps every active resting order to its current location in the book.
            // Must remain synchronized with both BookSides.
            std::unordered_map<OrderID, OrderEntry> order_lookup_;
    };
}