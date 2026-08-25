#include "replay/lobster_orderbook_parser.hpp"
#include <sstream>

namespace mercury {
    LobsterOrderBookParser::LobsterOrderBookParser(const std::string& path, std::size_t level)
     : file_(path), level(level){}
    bool LobsterOrderBookParser::good() const noexcept {
        return file_.good();
    }
    std::optional<LobsterOrderBookRow> LobsterOrderBookParser::next() {
        std::string line;
        if(!std::getline(file_,line)) {
            return std::nullopt;
        }
        std::stringstream ss(line);
        std::string field;
        LobsterOrderBookRow row;
        for(std::size_t i=0;i<level;i++){
            LobsterLevel level;
            std::getline(ss, field, ',');
            level.ask_price = std::stoll(field);
            std::getline(ss, field, ',');
            level.ask_size = std::stoll(field);
            std::getline(ss, field, ',');
            level.bid_price = std::stoll(field);
            std::getline(ss, field, ',');
            level.bid_size = std::stoll(field);
            row.levels.push_back(level);
        }
        return row;
    }
}