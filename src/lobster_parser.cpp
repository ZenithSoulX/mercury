#include "replay/lobster_parser.hpp"
#include <sstream>
#include <cassert>

namespace mercury {
    LobsterParser::LobsterParser(const std::string& path) : file_(path) 
    {}
    bool LobsterParser::good() const noexcept {
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
        std::string int_part, dec_part;
        std::istringstream f_stream(field);
        std::getline(f_stream, int_part, '.');
        std::getline(f_stream, dec_part);
        dec_part.resize(9,'0'); // pad with zeros to ensure 9 digits for nanoseconds
        std::uint64_t secs = std::stoull(int_part);
        std::uint64_t nanos = std::stoull(dec_part);
        message.timestamp = secs*1'000'000'000ULL + nanos; // convert to nanoseconds
        std::getline(ss, field, ',');
        int type = std::stoi(field);
        //safety check. Helps if file has corrupted data. 
        assert((type ==1 || type ==2 || type ==3 || type ==4 || type ==5 || type ==7) && "Invalid LobsterEventType");
        message.event_type = static_cast<LobsterEventType>(type);
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