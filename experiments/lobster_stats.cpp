#include <iostream>
#include <unordered_map>
#include "replay/lobster_parser.hpp"
using namespace mercury;

int main(){
    LobsterParser parser("../data/messageL1.csv");
    std::cout<<parser.good()<<"\n";
    std::unordered_map<int, std::size_t> count;
    std::size_t total =0;
    while(auto message = parser.next()){
        count[static_cast<int>(message->event_type)]++;
        total++;
    }
    std::cout<<"Total Events : "<<total<<"\n";
    for(const auto& [type, num]:count){
        std::cout<<"Type : "<<type<<" : "<<num<<"\n";
    }
}