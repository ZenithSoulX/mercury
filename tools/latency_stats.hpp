#pragma once
#include <cstdint>
#include <vector>
#include <string>

namespace mercury{
    struct LatencyStats{
        std::size_t sample_count;
        double p50;
        double p90;
        double p95;
        double p99;
        double max;
    };
    LatencyStats compute_stats(std::vector<std::int64_t> data);
    void printstats(const std::string& label, const LatencyStats& stats);
}