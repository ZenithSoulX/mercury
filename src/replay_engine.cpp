#include "replay/replay_engine.hpp"
#include <cassert>
#include <iostream>

namespace mercury {
    ReplayEngine::ReplayEngine(const std::string& path, OrderBook& book)
     : parser_(path), book_(book){}
    bool ReplayEngine::step(){
        auto msg = parser_.next();
        if(!msg.has_value()) return false;
        dispatch(*msg);
        return true;
    }
    std::size_t ReplayEngine::runAll(){
        std::size_t count =0;
        while(step()) count++;
        return count;
    }
    bool ReplayEngine::good() const noexcept {
        return parser_.good();
    }
    std::size_t ReplayEngine::tradeCount() const noexcept {
        return trade_count_;
    }
    std::size_t ReplayEngine::hiddenExecutionCount() const noexcept {
        return hidden_execution_count_;
    }
    std::size_t ReplayEngine::haltCount() const noexcept {
        return halt_count_;
    }
    std::size_t ReplayEngine::untrackedPartialCancelCount() const noexcept {
        return untracked_partial_cancel_count_;
    }
    std::size_t ReplayEngine::untrackedDeletionCount() const noexcept {
        return untracked_deletion_count_;
    }
    std::size_t ReplayEngine::untrackedVisibleExecutionCount() const noexcept {
        return untracked_visible_execution_count_;
    }
    void ReplayEngine::dispatch(const LobsterMessage& msg){
        switch(msg.event_type){
            case LobsterEventType::Submission : {
                Side side = (msg.direction ==1)?Side::Buy:Side::Sell;
                order_storage_.emplace_back(
                    OrderID{msg.order_id},
                    side,
                    OrderType::Limit,
                    TimeInForce::GTC,
                    Price{static_cast<std::int64_t>(msg.price)},
                    Quantity{msg.size},
                    SequenceNumber{next_sequence_++},
                    EventTimestamp{msg.timestamp}
                );
                auto trades = book_.submitOrder(order_storage_.back());
                trade_count_ += trades.size();
                break;
            }
            case LobsterEventType::PartialCancellation : {
                bool found = book_.reduceOrder(OrderID{msg.order_id}, Quantity{msg.size});
                if(!found){
                    untracked_partial_cancel_count_++;
                }
                break;
            }
            case LobsterEventType::Deletion : {
                if(!book_.cancelOrder(OrderID{msg.order_id})){
                    //If the order is not found, it might be pre-market activity.
                    //This is expected behavior for L1 dataset. 
                    untracked_deletion_count_++;
                    if(untracked_deletion_count_<=20){
                        std::cout<<"Untracked type 3 "<<msg.order_id<<" qty = "<<msg.size<<" price = "<<msg.price<<'\n';
                    }
                }
                break;
            }
            case LobsterEventType::VisibleExecution : {
                bool found = book_.reduceOrder(OrderID{msg.order_id}, Quantity{msg.size});
                if(!found){
                    untracked_visible_execution_count_++;
                    if(untracked_visible_execution_count_<=20){
                        std::cout<<"Untracked type 4 "<<msg.order_id<<" qty = "<<msg.size<<" price = "<<msg.price<<'\n';
                    }
                }
                break;
            }
            case LobsterEventType::HiddenExecution : {
                //Currently out of scope for L1
                hidden_execution_count_++;
                break;
            }
            case LobsterEventType::TradingHalt : {
                //Currently out of scope from L1 dataset
                halt_count_++;
                break;
            }
            default : assert(false && "Unrecognized Event type for LOBSTER dataset");
        }
    }
}