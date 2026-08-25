#pragma once
#include <cstddef>
#include <optional>
#include <string>
#include "domain/order_book.hpp"
#include "replay/replay_engine.hpp"
#include "lobster_orderbook_parser.hpp"

namespace mercury {
    struct ValidationMismatch {
        std::size_t row_index;
        std::string desc;
    };
    class BookValidator {
        public :
            BookValidator(ReplayEngine& engine, OrderBook& book, LobsterOrderBookParser& book_parser);
            std::optional<ValidationMismatch> run(); //Runs until end of file or first mismatch. Returns nullopt if every validated row matched.
        private :
            ReplayEngine& engine_;
            OrderBook& book_;
            LobsterOrderBookParser& book_parser_;
    };
}