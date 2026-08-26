#pragma once
#include "lobster_parser.hpp"
#include "domain/order_book.hpp"
#include <string>
#include <cstddef>
#include <deque>
#include <vector>
#include <cstdint>

namespace mercury {
    class ReplayEngine {
        public :
            ReplayEngine(const std::string& path, OrderBook& book);
            bool step();
            std::size_t runAll();
            std::size_t hiddenExecutionCount() const noexcept;
            std::size_t haltCount() const noexcept;
            [[nodiscard]] bool good() const noexcept;
            std::size_t untrackedPartialCancelCount() const noexcept;
            std::size_t untrackedDeletionCount() const noexcept;
            std::size_t untrackedVisibleExecutionCount() const noexcept;
            std::size_t tradeCount() const noexcept;
            [[nodiscard]] const std::vector<std::int64_t>& submitLatencies() const noexcept;
            [[nodiscard]] const std::vector<std::int64_t>& cancelLatencies() const noexcept;
            [[nodiscard]] const std::vector<std::int64_t>& reduceLatencies() const noexcept;

        private :
            LobsterParser parser_;
            OrderBook& book_;
            std::size_t hidden_execution_count_ =0;
            std::size_t halt_count_ =0;
            std::size_t untracked_partial_cancel_count_ =0;
            std::size_t untracked_deletion_count_ =0;
            std::size_t untracked_visible_execution_count_ =0;
            std::size_t trade_count_ =0;
            void dispatch(const LobsterMessage& msg);
            std::deque<Order> order_storage_; // Storage for orders to ensure they remain valid while in the book. Orders are moved into the book and removed when filled or cancelled.
            std::uint64_t next_sequence_ =0; // sequence number.
            std::vector<std::int64_t> submit_latencies_ns_;
            std::vector<std::int64_t> cancel_latencies_ns_;
            std::vector<std::int64_t> reduce_latencies_ns_;
    };  
}