#pragma once
#include <fstream>
#include <optional>
#include <string>
#include "lobster_orderbook_row.hpp"

namespace mercury {
    class LobsterOrderBookParser {
        public :
            explicit LobsterOrderBookParser(const std::string& path, std::size_t levels);
            [[nodiscard]] bool good() const noexcept;
            [[nodiscard]] std::optional<LobsterOrderBookRow> next();
        private :
           std::ifstream file_; 
           std::size_t level;
    };
}