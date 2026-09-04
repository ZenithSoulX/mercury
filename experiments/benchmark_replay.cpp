#include <chrono>
#include <iomanip>
#include <iostream>

#include "domain/order_book.hpp"
#include "replay/replay_engine.hpp"

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0]
                  << " <message-file.csv>\n";
        return 1;
    }

    mercury::OrderBook book;
    mercury::ReplayEngine engine(argv[1], book);

    if (!engine.good()) {
        std::cerr << "Failed to open file\n";
        return 1;
    }

    auto start = std::chrono::steady_clock::now();

    std::size_t events = engine.runAll();

    auto end = std::chrono::steady_clock::now();

    double seconds =
        std::chrono::duration<double>(end - start).count();

    double throughput = events / seconds;

    std::cout << "Events processed : "
              << events << '\n';

    std::cout << "Elapsed time     : "
              << std::fixed
              << std::setprecision(6)
              << seconds
              << " sec\n";

    std::cout << "Throughput       : "
              << static_cast<std::uint64_t>(throughput)
              << " events/sec\n";

    std::cout 
        << "Trades generated : "
        << engine.tradeCount()
        <<'\n';

    std::cout
        << "Peak active orders : "
        << book.peakActiveOrders()
        << '\n';

    return 0;
}