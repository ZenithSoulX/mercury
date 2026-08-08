#include "domain/book_side.hpp"

namespace mercury {

    PriceLevel* BookSide::best() noexcept {
        if (index_.empty()) return nullptr;
        return side_ == Side::Buy ? std::prev(index_.end())->level.get() : index_.begin()->level.get();
    }

    const PriceLevel* BookSide::best() const noexcept {
        if (index_.empty()) return nullptr;
        return side_ == Side::Buy ? std::prev(index_.end())->level.get() : index_.begin()->level.get();
    }

    OrderLocation BookSide::addOrder(Order& order) {
        assert(order.side() == side_ && "Order side does not match this BookSide");
        PriceLevel& level = index_.getOrCreate(order.price(), side_);
        auto it = level.addOrder(order);
        return OrderLocation{&level, it};
    }

    void BookSide::removeOrder(const OrderLocation& location) {
        PriceLevel* level = location.level;
        assert(level != nullptr && "OderLocation contains null PriceLevel");
        assert(level->side() == side_ && "removeOrder called with mismatched Side for PriceLevel");
        level->erase(location.iterator);
        if (level->empty()) {
            index_.erase(level->price()); // keeps PriceIndex's no-empty-levels invariant true
        }
    }

    bool BookSide::empty() const noexcept {
        return index_.empty();
    }

    std::size_t BookSide::pricelevelCount() const noexcept {
        return index_.size();
    }
}