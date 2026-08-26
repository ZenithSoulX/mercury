#include <iostream>
#include "domain/order_book.hpp"
#include "replay/replay_engine.hpp"
#include "latency_stats.hpp"

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <message-file.csv>\n";
        return 1;
    }
    mercury::OrderBook book;
    mercury::ReplayEngine engine(argv[1], book);
    if (!engine.good()) {
        std::cerr << "Failed to open file: " << argv[1] << "\n";
        return 1;
    }
    engine.runAll();
    mercury::printstats("submit", mercury::compute_stats(engine.submitLatencies()));
    mercury::printstats("cancel", mercury::compute_stats(engine.cancelLatencies()));
    mercury::printstats("reduce", mercury::compute_stats(engine.reduceLatencies()));
    return 0;
}