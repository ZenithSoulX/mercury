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
        assert(order.level_ == nullptr && "Order is already in a PriceLevel");
        order.prev_ = tail_;
        order.next_ = nullptr;
        order.level_ = this;
        if(tail_){
            tail_->next_ = &order;
        }
        else head_ = &order;
        tail_ = &order;
        ++order_count_;
        total_volume_ = Volume{total_volume_.get()+order.remainingQuantity().get()};

    #ifndef NDEBUG
        verifyInvariants();
    #endif
        return;
    }
    void PriceLevel::removeFront(){
        assert(head_ != nullptr && "Cannot remove from an empty PriceLevel");
        erase(*head_);
    }
    void PriceLevel::reduceVolume(Quantity amount){
        assert(head_ != nullptr && "Cannot reduce volume of an empty PriceLevel");
        assert(amount.get() > 0 && "Reduction amount must be positive");
        assert(amount.get() <= total_volume_.get() && "Cannot reduce more than total volume");
        total_volume_ = Volume{total_volume_.get() - amount.get()};
        // We dont need to verify invariants as it is a transient state and will be verified when the order is filled or cancelled.
    }
    void PriceLevel::erase(Order& order){
        assert(order.level_ == this && "Order does not belong to this PriceLevel");
        if(order.prev_){
            order.prev_->next_ = order.next_;
        }
        else{
            head_ = order.next_;
        }
        if(order.next_){
            order.next_->prev_ = order.prev_;
        }
        else{
            tail_ = order.prev_;
        }
        order.level_ = nullptr;
        total_volume_ = Volume{total_volume_.get() - order.remainingQuantity().get()};
        --order_count_;
        order.prev_ = nullptr;
        order.next_ = nullptr;
    #ifndef NDEBUG
        verifyInvariants();
    #endif 
        return;
    }

    Order& PriceLevel::front(){
        assert(head_ != nullptr && "Cannot access front of an empty PriceLevel");
        return *head_;
    }
    const Order& PriceLevel::front() const {
        assert(head_ != nullptr && "Cannot access front of an empty PriceLevel");
        return *head_;
    }
    bool PriceLevel::empty() const noexcept {
        return head_ == nullptr; 
    }
    [[nodiscard]] std::size_t PriceLevel::size() const noexcept {
        return order_count_;
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
    #ifndef NDEBUG
    void PriceLevel::verifyInvariants() const 
    {
        std::uint64_t volume =0;
        std::size_t count =0;
        const Order* current = head_;
        const Order* prev = nullptr;
        while(current){
            assert(current->level_==this && "Order's level_ pointer does not point to this PriceLevel");
            assert(current->side()==side_ && "Order's side does not match PriceLevel's side");
            assert(current->price()==price_ && "Order's price does not match PriceLevel's price");
            assert(current->prev_==prev && "Order's prev_ pointer is inconsistent with the linked list");
            assert(current->isActive() && "Inactive order found in PriceLevel");
            volume += current->remainingQuantity().get();
            ++count;
            prev = current;
            current = current->next_;
        }
        assert(prev==tail_ && "Tail pointer is inconsistent with the linked list");
        assert(volume == total_volume_.get() && "Total volume does not match sum of remaining quantities");
        assert(count == order_count_ && "Order count does not match number of orders in the linked list");
        assert((head_==nullptr) == (tail_==nullptr) && "Head and tail pointers are inconsistent");
        assert((head_==nullptr) == (order_count_==0) && "Head pointer and order count are inconsistent");
        assert((head_==nullptr) == (total_volume_.get()==0) && "Head pointer and total volume are inconsistent");
    }
    #endif
}