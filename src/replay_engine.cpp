#include "replay/replay_engine.hpp"
#include <cassert>
#include <iostream>
#include <chrono>

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
    const std::vector<std::int64_t>& ReplayEngine::submitLatencies() const noexcept {
        return submit_latencies_ns_;
    }
    const std::vector<std::int64_t>& ReplayEngine::cancelLatencies() const noexcept {
        return cancel_latencies_ns_;
    }
    const std::vector<std::int64_t>& ReplayEngine::reduceLatencies() const noexcept {
        return reduce_latencies_ns_;
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
                auto t0 = std::chrono::steady_clock::now();
                auto trades = book_.submitOrder(order_storage_.back());
                auto t1 = std::chrono::steady_clock::now();
                submit_latencies_ns_.push_back(std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count());
                trade_count_ += trades.size();
                break;
            }
            case LobsterEventType::PartialCancellation : {
                auto t0 = std::chrono::steady_clock::now();
                bool found = book_.reduceOrder(OrderID{msg.order_id}, Quantity{msg.size});
                auto t1 = std::chrono::steady_clock::now();
                reduce_latencies_ns_.push_back(std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count());
                if(!found){
                    //If the order is not found, it might be pre-market activity.
                    //This is expected behavior for L1 dataset. 
                    untracked_partial_cancel_count_++;
                }
                break;
            }
            case LobsterEventType::Deletion : {
                bool flag = book_.cancelOrder(OrderID{msg.order_id});
                auto t0 = std::chrono::steady_clock::now();
                book_.cancelOrder(OrderID{msg.order_id});
                auto t1 = std::chrono::steady_clock::now();
                cancel_latencies_ns_.push_back(std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count());
                if(!flag){
                    //If the order is not found, it might be pre-market activity.
                    //This is expected behavior for L1 dataset. 
                    untracked_deletion_count_++;
                }
                break;
            }
            case LobsterEventType::VisibleExecution : {
                //Checkpoint only - no action.
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