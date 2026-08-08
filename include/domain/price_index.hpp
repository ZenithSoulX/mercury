#pragma once
#include <vector>
#include <memory>
#include "price_level.hpp"
#include "side.hpp"

namespace mercury {

    struct PriceLevelEntry {
        Price price;
        std::unique_ptr<PriceLevel> level;
        PriceLevelEntry(Price price, std::unique_ptr<PriceLevel> level):
            price(price), level(std::move(level)) {}
    };

    /* Stores PriceLevels in ascending price order.
     *
     * PriceIndex owns PriceLevels and provides efficient lookup,
     * insertion, removal, and ordered iteration.
     *
     * Matching semantics and best bid/ask logic belong to BookSide.
     */
    class PriceIndex {
        public:
            using Container = std::vector<PriceLevelEntry>;
            using Iterator = std::vector<PriceLevelEntry>::iterator; //Any insertion or erase may invalidate iterators and pointers into the vector, so we don't expose them to clients. Clients should use PriceLevel* instead.
            using ConstIterator = std::vector<PriceLevelEntry>::const_iterator;

            PriceIndex() = default;
            PriceIndex(const PriceIndex&) = delete;
            PriceIndex& operator=(const PriceIndex&) = delete;
            PriceIndex(PriceIndex&&) = default;
            PriceIndex& operator=(PriceIndex&&) = default;

            [[nodiscard]] bool contains(Price price) const noexcept;
            [[nodiscard]] PriceLevel* find(Price price) noexcept;
            [[nodiscard]] const PriceLevel* find(Price price) const noexcept;

            PriceLevel& getOrCreate(Price price, Side side);

            void erase(Price price); //mirrors the OrderBook::cancel no-op-on-missing convention established earlier.

            [[nodiscard]] PriceLevel& front();
            [[nodiscard]] const PriceLevel& front() const;
            [[nodiscard]] PriceLevel& back();
            [[nodiscard]] const PriceLevel& back() const;

            [[nodiscard]] Price bestPrice() const noexcept;
            [[nodiscard]] Price worstPrice() const noexcept;

            [[nodiscard]] bool isEmpty() const noexcept;
            [[nodiscard]] std::size_t count() const noexcept;

            [[nodiscard]] bool empty() const noexcept;
            [[nodiscard]] std::size_t size() const noexcept;

            Iterator begin() noexcept;
            Iterator end() noexcept;
            ConstIterator begin() const noexcept;
            ConstIterator end() const noexcept;
            ConstIterator cbegin() const noexcept;
            ConstIterator cend() const noexcept;

        private:
            std::vector<PriceLevelEntry> levels_;
            Iterator lowerBound(Price price) noexcept;
            ConstIterator lowerBound(Price price) const noexcept;
        #ifndef NDEBUG
            void verifyInvariants() const;
        #endif
    };

} 