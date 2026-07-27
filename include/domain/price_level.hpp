#pragma once
#include <list>
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
            using Iterator = std::list<Order*>::iterator;
            using ConstIterator = std::list<Order*>::const_iterator;
            using OrderList = std::list<Order*>;
            explicit PriceLevel(Price price, Side side);

            PriceLevel() = delete;
            PriceLevel(const PriceLevel&) = delete;
            PriceLevel& operator=(const PriceLevel&) = delete;
            PriceLevel(PriceLevel&&) = delete;
            PriceLevel& operator=(PriceLevel&&) = delete;

            PriceLevel::Iterator addOrder(Order& order);
            void removeFront();
            void reduceVolume(Quantity amount);
            Iterator erase(Iterator it);
            [[nodiscard]] Order& front();
            [[nodiscard]] const Order& front() const;
            [[nodiscard]] bool empty() const noexcept;
            std::size_t size() const noexcept;
            Price price() const noexcept;
            Side side() const noexcept;
            [[nodiscard]] Volume totalVolume() const noexcept;

            Iterator begin() noexcept;
            Iterator end() noexcept;
            ConstIterator begin() const noexcept;
            ConstIterator end() const noexcept;
            ConstIterator cbegin() const noexcept;
            ConstIterator cend() const noexcept;

    private:
        Price price_;
        Side side_;
        OrderList orders_;
        Volume total_volume_;
    #ifndef NDEBUG
        void verifyInvariants() const;
    #endif
    };
}
