#include <iostream>
#include "domain/order_book.hpp"
#include "replay/replay_engine.hpp"
#include "replay/lobster_orderbook_parser.hpp"
#include "replay/book_validator.hpp"

int main(int argc, char** argv) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0]<< " <message-file.csv> <orderbook-file.csv> <num-levels>\n";
        return 1;
    }

    const std::string message_path = argv[1];
    const std::string orderbook_path = argv[2];
    const std::size_t num_levels = std::stoul(argv[3]);

    mercury::OrderBook book;
    mercury::ReplayEngine engine(message_path, book);
    mercury::LobsterOrderBookParser orderbook_parser(orderbook_path, num_levels);

    if (!engine.good()) {
        std::cerr << "Failed to open message file: " << message_path << "\n";
        return 1;
    }
    if (!orderbook_parser.good()) {
        std::cerr << "Failed to open orderbook file: " << orderbook_path << "\n";
        return 1;
    }
    mercury::BookValidator validator(engine, book, orderbook_parser);
    auto mismatch = validator.run();
    std::cout
        << "Hidden executions : "
        << engine.hiddenExecutionCount()
        << '\n';

    std::cout
        << "Trading halts : "
        << engine.haltCount()
        << '\n';

    std::cout
        << "Untracked Type 2 (Partial Cancel) : "
        << engine.untrackedPartialCancelCount()
        << '\n';

    std::cout
        << "Untracked Type 3 (Deletion) : "
        << engine.untrackedDeletionCount()
        << '\n';

    std::cout
        << "Untracked Type 4 (Visible Execution) : "
        << engine.untrackedVisibleExecutionCount()
        << '\n';

    if (mismatch.has_value()) {
        std::cout << "VALIDATION FAILED\n";
        std::cout << mismatch->desc << "\n";
        return 1;
    }
    std::cout << "VALIDATION PASSED and every row matched.\n";
    return 0;
}