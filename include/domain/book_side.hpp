#pragma once
#include <cstddef>
#include "core/types.hpp"
#include "price_index.hpp"
#include "price_level.hpp"

namespace mercury {
    /* Manages one side (bids or asks) of an order book.
    *
    * Owns all PriceLevels for a single side and defines the notion of
    * the best price (highest bid, lowest ask).
    */
    struct OrderLocation {
        PriceLevel* level;
        PriceLevel::Iterator iterator{};
    };

    class BookSide {
    public:
        explicit BookSide(Side side) noexcept : side_(side) {
        }
        BookSide(const BookSide&) = delete;
        BookSide& operator=(const BookSide&) = delete;
        BookSide(BookSide&&) = default;
        BookSide& operator=(BookSide&&) = default;

        [[nodiscard]] PriceLevel* best() noexcept;
        [[nodiscard]] const PriceLevel* best() const noexcept;

        // Adds an order to its price level, creating the level if necessary.
        // Returns the inserted order's location.
        [[nodiscard]]
        OrderLocation addOrder(Order& order);

        void removeOrder(const OrderLocation&);

        [[nodiscard]] bool empty() const noexcept;
        [[nodiscard]] std::size_t pricelevelCount() const noexcept;
        [[nodiscard]] Side side() const noexcept { return side_; }

        //Iteration not exposed. MatchingEngine only ever calls best() repeatdely.

    private:
        PriceIndex index_;
        Side side_;
    };

} 