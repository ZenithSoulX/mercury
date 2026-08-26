#include "latency_stats.hpp"
#include <algorithm>
#include <iostream>
#include <iomanip>

namespace mercury {
    LatencyStats compute_stats(std::vector<std::int64_t>data){
        std::sort(data.begin(),data.end());
        auto pick = [&](double p)->double{
            if(data.empty()) return 0.0;
            std::size_t idx = static_cast<std::size_t>(p*(data.size()-1));
            return static_cast<double>(data[idx]);
        };
        return LatencyStats{
            data.size(),
            pick(0.50),
            pick(0.90),
            pick(0.95),
            pick(0.99),
            data.empty()?0.0:static_cast<double>(data.back())
        };
    }
    void printstats(const std::string& label, const LatencyStats& stats){
        std::cout<<std::left<<std::setw(10)<<label
            <<" n = "<<stats.sample_count
            <<" p50 = "<<stats.p50<<"ns"
            <<" p90 = "<<stats.p90<<"ns"
            <<" p99 = "<<stats.p99<<"ns"
            <<" max = "<<stats.max<<"ns\n";
    }
}