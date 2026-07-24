#pragma once
namespace mercury {
    enum class OrderStatus {
        Pending,
        Active,
        PartiallyFilled,
        Filled,
        Cancelled,
        Rejected,
        Expired
    };
}