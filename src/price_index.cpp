#include "domain/price_index.hpp"
#include <algorithm>

namespace mercury {
namespace {
        constexpr auto ComparePrice =
            [](const PriceLevelEntry& entry, Price price) {
            return entry.price < price;
        };
        template <typename Iterator, typename Sentinel> //Two independent type parameters instead of forcing two arguments to be the same type
        constexpr bool matches(Iterator it, Sentinel end, Price price) noexcept {
            return it != end && it->price == price;
        }
}
    PriceIndex::Iterator PriceIndex::lowerBound(Price price) noexcept {
        return std::lower_bound(levels_.begin(), levels_.end(), price, ComparePrice);
    }

    PriceIndex::ConstIterator PriceIndex::lowerBound(Price price) const noexcept {
        return std::lower_bound(levels_.begin(), levels_.end(), price, ComparePrice);
    }

    bool PriceIndex::contains(Price price) const noexcept {
        const auto it = lowerBound(price);
        return matches(it,levels_.end(),price);
    }

    PriceLevel* PriceIndex::find(Price price) noexcept {
        auto it = lowerBound(price);
        if (matches(it,levels_.end(),price)) {
            return it->level.get();
        }
        return nullptr;
    }

    const PriceLevel* PriceIndex::find(Price price) const noexcept {
        auto it = lowerBound(price);
        if (matches(it,levels_.end(),price)) {
            return it->level.get();
        }
        return nullptr;
    }

    PriceLevel& PriceIndex::getOrCreate(Price price, Side side) {
        auto it = lowerBound(price);
        if (matches(it,levels_.end(),price)) {
            assert(it->level->side() == side && "getOrCreate called with mismatched Side for an existing PriceLevel");
            return *it->level;
        }

        // Insert-in-place at the correct sorted position. This shifts every
        // element after 'it' — O(n), but a contiguous memmove over small n,
        // not a red-black tree rebalance; see design notes on this trade-off.
        auto inserted = levels_.emplace(it, price, std::make_unique<PriceLevel>(price, side));
        return *inserted->level;
    }

    void PriceIndex::erase(Price price) {
        auto it = lowerBound(price);
        if (!matches(it,levels_.end(),price)) {
            return; // no-op — mirrors OrderBook's cancel-on-missing-id convention
        }
        levels_.erase(it); // O(n) shift, same trade-off as insert
    #ifndef NDEBUG
        verifyInvariants();
    #endif
    }

    bool PriceIndex::empty() const noexcept {
        return levels_.empty();
    }

    std::size_t PriceIndex::size() const noexcept {
        return levels_.size();
    }

#ifndef NDEBUG
    void PriceIndex::verifyInvariants() const {
        for(std::size_t i=0;i<levels_.size();i++){
            const auto& entry = levels_[i];
            assert(entry.level);
            assert(!entry.level->empty() && "PriceLevel should never be empty while in PriceIndex");
            assert(entry.level->price() == entry.price);
            if(i>0){
                assert(levels_[i-1].price < entry.price);
            }
        }
    }
#endif
    PriceIndex::Iterator PriceIndex::begin() noexcept { return levels_.begin(); }
    PriceIndex::Iterator PriceIndex::end() noexcept { return levels_.end(); }
    PriceIndex::ConstIterator PriceIndex::begin() const noexcept { return levels_.begin(); }
    PriceIndex::ConstIterator PriceIndex::end() const noexcept { return levels_.end(); }
    PriceIndex::ConstIterator PriceIndex::cbegin() const noexcept { return levels_.cbegin(); }
    PriceIndex::ConstIterator PriceIndex::cend() const noexcept { return levels_.cend(); }

} 