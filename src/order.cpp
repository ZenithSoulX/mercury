#include "domain/order.hpp"

namespace mercury {
    void Order::fill(const Quantity& quantity) {
        assert(isActive() && "Cannot fill an order that is not active");
        assert(quantity.get()>0 && "Fill quantity must be positive");
        assert(quantity.get() <= remaining_quantity_.get() && "Cannot fill more than remaining quantity");
        const auto updated_remaining = remaining_quantity_.get() - quantity.get();
        remaining_quantity_ = Quantity{updated_remaining};
        status_ = updated_remaining == 0 ? OrderStatus::Filled : OrderStatus::PartiallyFilled;
    }
    void Order::cancel() {
        assert(isActive() && "Cannot cancel an order that is not active");
        status_ = OrderStatus::Cancelled;
    }
}