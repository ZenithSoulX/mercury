#include "replay/book_validator.hpp"
#include <sstream>
#include <iostream>

namespace mercury {
    BookValidator::BookValidator(ReplayEngine& engine, OrderBook& book, LobsterOrderBookParser& book_parser)
     : engine_(engine), book_(book), book_parser_(book_parser) {}
    std::optional<ValidationMismatch> BookValidator::run() {
        std::size_t row_index =0;
        std::size_t mismatch_count =0;
        std::optional<ValidationMismatch> first_mismatch;
        while(true){
            auto lobster_row = book_parser_.next();
            bool advanced = engine_.step();
            if(!lobster_row.has_value()||!advanced)break;
            //Skip validation for Halts and Hidden Executions as the ReplayEngine
            //tracks how many were skipped. 
            const auto& top = lobster_row->levels[0];
            std::int64_t engine_bid = book_.bestBid() ? book_.bestBid()->price().get() : -1;
            std::int64_t engine_ask = book_.bestAsk() ? book_.bestAsk()->price().get() : -1;
            std::uint64_t engine_bid_size = book_.bestBid() ? book_.bestBid()->totalVolume().get() : 0;
            std::uint64_t engine_ask_size = book_.bestAsk() ? book_.bestAsk()->totalVolume().get() : 0;
            bool bid_price_ok = engine_bid==top.bid_price;
            bool bid_size_ok = engine_bid_size==top.bid_size;
            bool ask_price_ok = engine_ask==top.ask_price;
            bool ask_size_ok = engine_ask_size==top.ask_size;
            if(!bid_price_ok || !bid_size_ok || !ask_price_ok || !ask_size_ok){
                mismatch_count++;
                std::ostringstream desc;
                desc <<"engine bid = "<<engine_bid<<"/"<<engine_bid_size<<" vs lobster bid = "<<top.bid_price<<"/"<<top.bid_size
                <<" | engine ask = "<<engine_ask<<"/"<<engine_ask_size<<" vs lobster ask = "<<top.ask_price<<"/"<<top.ask_size;
                if(!first_mismatch){
                    first_mismatch = ValidationMismatch{row_index, desc.str()};
                }
                if(mismatch_count <=10){
                    std::cout<<"Mismatch at row "<<row_index<<" : "<<desc.str()<<std::endl;
                }
            }
        row_index++;
        }
        std::cout<<"Total mismatches found: "<<mismatch_count<<" out of "<<row_index<<" rows processed."<<std::endl;
        return first_mismatch; //Return the first mismatch found
    }
}