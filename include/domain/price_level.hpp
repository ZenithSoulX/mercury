#pragma once
#include "order.hpp"

namespace mercury{
        /* Represents all active resting orders at a single price.
         *
         * A PriceLevel maintains FIFO ordering for orders sharing the
         * same price and side while tracking aggregate resting quantity.
         *
         * Invariants:
         * - Every order has the same price.
         * - Every order has the same side.
         * - FIFO ordering is preserved.
         * - total_quantity_ equals the sum of remaining quantities.
         */

    class PriceLevel {
        public:

            explicit PriceLevel(Price price, Side side);

            PriceLevel() = delete;
            PriceLevel(const PriceLevel&) = delete;
            PriceLevel& operator=(const PriceLevel&) = delete;
            PriceLevel(PriceLevel&&) = delete;
            PriceLevel& operator=(PriceLevel&&) = delete;

            void addOrder(Order& order);
            void removeFront();
            void reduceVolume(Quantity amount);
            void erase(Order& order);
            [[nodiscard]] Order& front();
            [[nodiscard]] const Order& front() const;
            [[nodiscard]] bool empty() const noexcept;
            std::size_t size() const noexcept;
            Price price() const noexcept;
            Side side() const noexcept;
            [[nodiscard]] Volume totalVolume() const noexcept;

    private:
        Price price_;
        Side side_;
        Volume total_volume_;
        Order* head_ = nullptr;
        Order* tail_ = nullptr;
        std::size_t order_count_ =0;
    #ifndef NDEBUG
        void verifyInvariants() const;
    #endif
    };
}