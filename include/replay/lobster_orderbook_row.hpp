#pragma once
#include<cstdint>
#include <vector>

namespace mercury {
    struct LobsterLevel {
        std::int64_t ask_price;
        std::int64_t ask_size;
        std::int64_t bid_price;
        std::int64_t bid_size;
    };
    struct LobsterOrderBookRow {
        std::vector<LobsterLevel> levels;
    };
}