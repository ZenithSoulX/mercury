#include "replay/replay_engine.hpp"
#include <cassert>

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
    std::size_t ReplayEngine::hiddenExecutionCount() const noexcept {
        return hidden_execution_count_;
    }
    std::size_t ReplayEngine::haltCount() const noexcept {
        return halt_count_;
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
                book_.submitOrder(order_storage_.back());
                break;
            }
            case LobsterEventType::PartialCancellation : {
                book_.reduceOrder(OrderID{msg.order_id},Quantity{msg.size});
                break;
            }
            case LobsterEventType::Deletion : {
                book_.cancelOrder(OrderID{msg.order_id});
                break;
            }
            case LobsterEventType::VisibleExecution : {
                //Validation checkpoint only. Comparing Mercury's trade history 
                //against these rows is a validation step.
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
            }
            default : assert(false && "Unrecognized Event type for LOBSTER dataset");
        }
    }
}