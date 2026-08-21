#include <iostream>

#include "domain/order_book.hpp"

using namespace mercury;

int main() {

    OrderBook book;

    Order buy1(
        OrderID{1},
        Side::Buy,
        OrderType::Limit,
        TimeInForce::GTC,
        Price{100},
        Quantity{10},
        SequenceNumber{1},
        EventTimestamp{1}
    );

    book.submitOrder(buy1);

    Order sell1(
        OrderID{2},
        Side::Sell,
        OrderType::Limit,
        TimeInForce::GTC,
        Price{101},
        Quantity{5},
        SequenceNumber{2},
        EventTimestamp{2}
    );

    book.submitOrder(sell1);

    Order buy2(
        OrderID{3},
        Side::Buy,
        OrderType::Limit,
        TimeInForce::GTC,
        Price{101},
        Quantity{5},
        SequenceNumber{3},
        EventTimestamp{3}
    );

    auto trades = book.submitOrder(buy2);

    std::cout << "Trades generated: "
              << trades.size()
              << "\n\n";

    for(const auto& trade : trades) {

        std::cout
            << "Incoming=" << trade.incomingId.get()
            << " RestingID=" << trade.restingId.get()
            << " Price=" << trade.price.get()
            << " Qty=" << trade.quantity.get()
            << '\n';
    }

    std::cout << "\n";

    if(book.bestBid()) {
        std::cout
            << "Best Bid: "
            << book.bestBid()->price().get()
            << '\n';
    } else {
        std::cout << "Best Bid: NONE\n";
    }

    if(book.bestAsk()) {
        std::cout
            << "Best Ask: "
            << book.bestAsk()->price().get()
            << '\n';
    } else {
        std::cout << "Best Ask: NONE\n";
    }
}