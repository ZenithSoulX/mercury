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
    class BookSide {
    public:
        explicit BookSide(Side side) noexcept : side_(side) {
        }
        BookSide(const BookSide&) = delete;
        BookSide& operator=(const BookSide&) = delete;
        BookSide(BookSide&&) = default;
        BookSide& operator=(BookSide&&) = default;
        void addOrder(Order& order);
        void removeOrder(Order& order);
        [[nodiscard]] PriceLevel* best() noexcept;
        [[nodiscard]] const PriceLevel* best() const noexcept;
        [[nodiscard]] bool empty() const noexcept;
        [[nodiscard]] std::size_t pricelevelCount() const noexcept;
        [[nodiscard]] Side side() const noexcept { return side_; }
        
    private:
        PriceIndex index_;
        Side side_;
    };

} 