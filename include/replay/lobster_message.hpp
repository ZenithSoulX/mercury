#pragma once
#include "core/types.hpp"

namespace mercury{
    enum class LobsterEventType {
        Submission =1,
        PartialCancellation = 2,
        Deletion = 3,
        VisibleExecution = 4,
        HiddenExecution = 5,
        TradingHalt = 7
    };
    struct LobsterMessage {
        double timestamp;
        LobsterEventType event_type;
        std::uint64_t order_id;
        std::uint64_t size;
        std::uint64_t price;
        int direction;
    };
}