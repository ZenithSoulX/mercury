#pragma once
#include <fstream>
#include <string>
#include <optional>
#include "lobster_message.hpp"

namespace mercury {
    class LobsterParser {
        public :
            explicit LobsterParser(const std::string& path);
            [[nodiscard]] bool good() const noexcept;
            [[nodiscard]] std::optional<LobsterMessage> next();
        
        private :
            std::ifstream file_;
    };
}