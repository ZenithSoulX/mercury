#pragma once
#include "core/types.hpp"
namespace mercury {
    struct Trade {
                OrderID incomingId;
                OrderID restingId;
                Price price;
                Quantity quantity;
                EventTimestamp timestamp;
            };
}
