#include <iostream>
#include <chrono>
#include "replay/replay_engine.hpp"
#include "domain/order_book.hpp"

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <path-to-message-file.csv>\n";
        return 1;
    }

    mercury::OrderBook book;
    mercury::ReplayEngine engine(argv[1], book);

    if(!engine.good()){
        std::cerr << "Failed to open file: " << argv[1] << "\n";
        return 1;
    }

    auto start = std::chrono::steady_clock::now();
    std::size_t processed = engine.runAll();
    auto end = std::chrono::steady_clock::now();
    auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::cout << "Processed: " << processed << " messages\n";
    std::cout << "Elapsed: " << elapsed_ms << " ms\n";
    std::cout << "Final order count: " << book.orderCount() << "\n";
    std::cout << "Hidden executions skipped: " << engine.hiddenExecutionCount() << "\n";
    std::cout << "Halts skipped: " << engine.haltCount() << "\n";

    if (book.bestBid())std::cout << "Final best bid: " << book.bestBid()->price().get() << "\n";
    if (book.bestAsk())std::cout << "Final best ask: " << book.bestAsk()->price().get() << "\n";
    
    return 0;
}