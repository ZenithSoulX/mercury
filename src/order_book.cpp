#include "domain/order_book.hpp"
#include <cassert>
#include <algorithm>

namespace mercury {
    //TODO : Add FOK, IOC, Iceberg and other advanced order types/time_in_force.
namespace {
    bool crosses(const Order& incoming, Price resting_price){
        if(incoming.type() == OrderType::Market){
            return true;
        }
        return incoming.side() == Side::Buy ? incoming.price() >= resting_price : incoming.price() <= resting_price;
    }
}
    PriceLevel* OrderBook::bestBid() noexcept {return bids_.best();}
    PriceLevel* OrderBook::bestAsk() noexcept {return asks_.best();}
    const PriceLevel* OrderBook::bestBid() const noexcept {return bids_.best();}
    const PriceLevel* OrderBook::bestAsk() const noexcept {return asks_.best();}
    bool OrderBook::contains(OrderID id) const noexcept {
        return order_lookup_.find(id) != order_lookup_.end();
    }
    bool OrderBook::empty() const noexcept {
        return order_lookup_.empty();
    }
    std::size_t OrderBook::orderCount() const noexcept {
        return order_lookup_.size();
    }
    bool OrderBook::cancelOrder(OrderID id){
        auto it = order_lookup_.find(id);
        if(it == order_lookup_.end()){
            return false;
        }
        const OrderLocation& location = it->second.location;
        //First cancel the order before removing it from the book. This ensures that the order's state is updated before it is removed from the book.
        Order& order = *(*location.iterator);
        order.cancel();
        assert(location.level != nullptr && "OrderLocation contains null PriceLevel");
        Side side = location.level->side();
        BookSide& book_side = (side == Side::Buy) ? bids_ :asks_;
        book_side.removeOrder(location);
        order_lookup_.erase(it);
        return true;
    }
    std::vector<Trade> OrderBook::submitOrder(Order& incoming){
        std::vector<Trade> trades;
        BookSide& own_side = (incoming.side() == Side::Buy) ? bids_ : asks_;
        BookSide& opposing_side = (incoming.side() == Side::Buy) ? asks_ : bids_;
        while(incoming.isActive()){
            PriceLevel* level = opposing_side.best();
            if(level == nullptr || !crosses(incoming, level->price())){
                break;
            }
            Order& resting_order = level->front();
            std::uint64_t matched_amt = std::min(incoming.remainingQuantity().get(), resting_order.remainingQuantity().get());
            Quantity matched{matched_amt};
            incoming.fill(matched);
            resting_order.fill(matched);
            level->reduceVolume(matched);
            trades.push_back(Trade{incoming.id(),resting_order.id(),
                level->price(),matched,incoming.timestamp()});
            if(resting_order.isFilled()){
                auto it = order_lookup_.find(resting_order.id());
                assert(it != order_lookup_.end() && "Resting order matched but missing from order_lookup_");
                opposing_side.removeOrder(it->second.location);
                order_lookup_.erase(it);
            }
        }
        if(incoming.isActive() && (incoming.type() == OrderType::Limit || incoming.type() == OrderType::Iceberg)){
            OrderLocation location = own_side.addOrder(incoming);
            order_lookup_.emplace(incoming.id(), OrderEntry{location});
        }
        return trades;
    }    
}