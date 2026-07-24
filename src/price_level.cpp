#include "domain/price_level.hpp"
#include <cassert>

namespace mercury {
    PriceLevel::PriceLevel(Price price, Side side) 
    : price_(price),side_(side),total_volume_(Volume{0})
    {
    }
    void PriceLevel::addOrder(Order& order){
        assert(order.price()==price_ && "Order price does not match this PriceLevel");
        assert(order.side()==side_ && "Order side does not match this PriceLevel");
        assert(order.isActive() && "Cannot add an order that is not active");
        orders_.push_back(&order);
        total_volume_ = Volume{total_volume_.get() + order.remainingQuantity().get()};
    #ifndef NDEBUG
        verifyInvariants();
    #endif
    }
    void PriceLevel::removeFront(){
        assert(!empty() && "Cannot remove from an empty PriceLevel");
        const Order& order = front();
        total_volume_ = Volume{total_volume_.get() - order.remainingQuantity().get()};
        orders_.pop_front();
    #ifndef NDEBUG
        verifyInvariants();
    #endif
    }
    PriceLevel::Iterator PriceLevel::erase(Iterator it){
        assert(it != orders_.end() && "Cannot erase end iterator");
        const Order& order = **it;
        total_volume_ = Volume{total_volume_.get() - order.remainingQuantity().get()};
        Iterator next_it = orders_.erase(it);
    #ifndef NDEBUG
        verifyInvariants();
    #endif 
        return next_it;
    }

    Order& PriceLevel::front(){
        assert(!empty() && "Cannot access front of an empty PriceLevel");
        return *orders_.front();
    }
    const Order& PriceLevel::front() const {
        assert(!empty() && "Cannot access front of an empty PriceLevel");
        return *orders_.front();
    }
    bool PriceLevel::empty() const noexcept {
        return orders_.empty(); 
    }
    std::size_t PriceLevel::size() const noexcept {
        return orders_.size();
    }
    Price PriceLevel::price() const noexcept {
        return price_; 
    }
    Side PriceLevel::side() const noexcept {
        return side_; 
    }
    Volume PriceLevel::totalVolume() const noexcept {
        return total_volume_;
    }
    PriceLevel::Iterator PriceLevel::begin() noexcept { return orders_.begin(); }
    PriceLevel::Iterator PriceLevel::end() noexcept { return orders_.end(); }
    PriceLevel::ConstIterator PriceLevel::begin() const noexcept { return orders_.begin(); }
    PriceLevel::ConstIterator PriceLevel::end() const noexcept { return orders_.end(); }
    PriceLevel::ConstIterator PriceLevel::cbegin() const noexcept { return orders_.cbegin(); }
    PriceLevel::ConstIterator PriceLevel::cend() const noexcept { return orders_.cend(); }
    #ifndef NDEBUG
    void PriceLevel::verifyInvariants() const 
    {
        std::uint64_t computed_volume =0;
        for(const Order* order : orders_){
            assert(order!=nullptr);
            assert(order->price()==price_);
            assert(order->side()==side_);
            assert(order->isActive());
            computed_volume += order->remainingQuantity().get();
        }
        assert(computed_volume == total_volume_.get() && "Total volume invariant violated");
        assert((orders_.empty() == (total_volume_.get() == 0)) && "Empty state invariant violated");
    }
    #endif
}