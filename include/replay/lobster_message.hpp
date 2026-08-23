#pragma once

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
        //double timestamp;  double has around 15-17 significant digits which has a risk of off-by-a-few-nanoseconds errors.
        // instead we can work with int and decimal part differently.
        std::uint64_t timestamp;
        LobsterEventType event_type;
        std::uint64_t order_id;
        std::uint64_t size;
        std::uint64_t price;
        int direction;
    };
}