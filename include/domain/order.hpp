#pragma once
#include <cassert>
#include "core/types.hpp"
#include "order_status.hpp"
#include "order_type.hpp"
#include "time_in_force.hpp"
#include "side.hpp"

namespace mercury {
    /**
     * Represents a single immutable trading instruction.
     * 
     * Identity and trading parameters are immutable after construction.
     * Only execution progress (remaining quantity) and lifecycle state
     * may change during the lifetime of an Order.
     */
    class PriceLevel;
    class Order {
        private:
            // TODO : Decide which component (MatchingEngine or OrderBook) is allowed to mutate order state.
            // Resolved: only OrderBook may mutate order lifecycle state.
            friend class OrderBook;  
            friend class PriceLevel;
            const OrderID id_;
            const Side side_;
            const OrderType type_;
            const TimeInForce tif_;
            const Price price_;
            const Quantity original_quantity_;
            Volume remaining_quantity_;
            const SequenceNumber sequence_;
            const EventTimestamp timestamp_;
            OrderStatus status_;
            void fill(const Quantity& quantity);
            void cancel();
            void reduceQuantity(const Quantity& amount);
            Order* prev_ = nullptr;
            Order* next_ = nullptr;
            PriceLevel* level_ = nullptr;

        public:
            Order() = delete;
            Order(OrderID id,
                Side side, 
                OrderType type,
                TimeInForce tif,
                Price price, 
                Quantity original_quantity, 
                SequenceNumber sequence, 
                EventTimestamp timestamp)
                : id_(id), 
                side_(side), 
                type_(type),
                tif_(tif),
                price_(price), 
                original_quantity_(original_quantity), 
                remaining_quantity_(Volume{original_quantity.get()}), 
                sequence_(sequence), 
                timestamp_(timestamp), 
                status_(OrderStatus::Active) 
            {
                // Defense-in-depth.
                // Quantity and Price are expected to enforce their own invariants,
                // but Order verifies assumptions about its collaborators.
                assert(original_quantity.get() > 0 && "Order must start with positive quantity");
                if(type == OrderType::Limit || type == OrderType::Iceberg) {
                    assert(price_.get() > 0 && "Order price must be positive");
                }
            }
            // Moving or copying an Order is not allowed.
            Order(const Order&) = delete;
            Order(Order&&) = delete;
            Order& operator=(const Order&) = delete;
            Order& operator=(Order&&) = delete;

            OrderID id() const noexcept { return id_; }
            Price price() const noexcept { return price_; }
            Volume remainingQuantity() const noexcept { return remaining_quantity_; }
            Quantity originalQuantity() const noexcept { return original_quantity_; }
            SequenceNumber sequenceNumber() const noexcept { return sequence_; }
            EventTimestamp timestamp() const noexcept { return timestamp_; }
            std::uint64_t filledQuantity() const noexcept { return original_quantity_.get() - remaining_quantity_.get(); }
            OrderStatus status() const noexcept { return status_; }
            OrderType type() const noexcept { return type_; }
            TimeInForce timeInForce() const noexcept { return tif_; }
            Side side() const noexcept { return side_; }

            bool isActive() const noexcept {
                return status_ == OrderStatus::Active || status_ == OrderStatus::PartiallyFilled;
            }
            bool isFilled() const noexcept {
                return status_ == OrderStatus::Filled;
            }
            bool isCancelled() const noexcept {
                return status_ == OrderStatus::Cancelled;
            }
            Order* prev() const noexcept {return prev_;}
            Order* next() const noexcept {return next_;}
            PriceLevel* level() const noexcept {return level_;}
            
    };
}