#include "replay/lobster_parser.hpp"
#include <sstream>

namespace mercury {
    LobsterParser::LobsterParser(const std::string& path) : file_(path) 
    {}
    bool LobsterParser::good() const {
        return file_.good();
    }
    std::optional<LobsterMessage> LobsterParser::next(){
        std::string line;
        if(!std::getline(file_,line)){
            return std::nullopt;
        }
        std::stringstream ss(line);
        std::string field;
        LobsterMessage message;
        std::getline(ss, field, ',');
        message.timestamp = std::stod(field);
        std::getline(ss, field, ',');
        message.event_type = static_cast<LobsterEventType>(std::stoi(field));
        std::getline(ss, field, ',');
        message.order_id = std::stoull(field);
        std::getline(ss, field, ',');
        message.size = std::stoull(field);
        std::getline(ss, field, ',');
        message.price = std::stoull(field);
        std::getline(ss, field, ',');
        message.direction = std::stoi(field);
        return message;
    }
}