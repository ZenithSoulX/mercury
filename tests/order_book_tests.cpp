#include <gtest/gtest.h>
#include "domain/order_book.hpp"
using namespace mercury;

class OrderBookTest : public ::testing::Test {
    protected:
        OrderBook book;
};

Order makeBuy(
    OrderID id,
    Price price,
    Quantity qty,
    SequenceNumber seq = SequenceNumber{1},
    EventTimestamp ts = EventTimestamp{0})
{
    return Order(
        id,
        Side::Buy,
        OrderType::Limit,
        TimeInForce::GTC,
        price,
        qty,
        seq,
        ts
    );
}

Order makeSell(
    OrderID id,
    Price price,
    Quantity qty,
    SequenceNumber seq = SequenceNumber{1},
    EventTimestamp ts = EventTimestamp{0})
{
    return Order(
        id,
        Side::Sell,
        OrderType::Limit,
        TimeInForce::GTC,
        price,
        qty,
        seq,
        ts
    );
}

TEST_F(OrderBookTest, BuyLimitOrderRestsOnEmptyBook)
{
    auto buy = makeBuy(
        OrderID{1},
        Price{100},
        Quantity{10}
    );
    std::cout << sizeof(Order) << '\n';
    auto trades = book.submitOrder(buy);
    EXPECT_TRUE(trades.empty());
    EXPECT_FALSE(book.empty());
    EXPECT_EQ(book.orderCount(), 1);
    EXPECT_TRUE(book.contains(OrderID{1}));
    ASSERT_NE(book.bestBid(), nullptr);
    EXPECT_EQ(book.bestBid()->price(), Price{100});
    EXPECT_EQ(book.bestAsk(),nullptr);

}

TEST_F(OrderBookTest, ExactMatchFullFill)
{
    auto sell = makeSell(
        OrderID{1},
        Price{100},
        Quantity{10}
    );
    auto sell_trades = book.submitOrder(sell);
    EXPECT_TRUE(sell_trades.empty());
    auto buy = makeBuy(
        OrderID{2},
        Price{100},
        Quantity{10}
    );
    auto trades = book.submitOrder(buy);
    ASSERT_EQ(trades.size(), 1);
    EXPECT_EQ(trades[0].restingId, OrderID{1});
    EXPECT_EQ(trades[0].incomingId, OrderID{2});
    EXPECT_EQ(trades[0].price, Price{100});
    EXPECT_EQ(trades[0].quantity, Quantity{10});
    EXPECT_TRUE(book.empty());
    EXPECT_EQ(book.orderCount(), 0);
    EXPECT_EQ(book.bestBid(), nullptr);
    EXPECT_EQ(book.bestAsk(), nullptr);
    EXPECT_TRUE(buy.isFilled());
    EXPECT_TRUE(sell.isFilled());
}

TEST_F(OrderBookTest, PartialFill)
{
    auto sell = makeSell(
        OrderID{1},
        Price{100},
        Quantity{10}
    );

    book.submitOrder(sell);

    auto buy = makeBuy(
        OrderID{2},
        Price{100},
        Quantity{4}
    );

    auto trades = book.submitOrder(buy);

    ASSERT_EQ(trades.size(), 1);
    EXPECT_EQ(trades[0].price, Price{100});
    EXPECT_EQ(trades[0].quantity, Quantity{4});
    EXPECT_TRUE(buy.isFilled());
    EXPECT_EQ(sell.status(),OrderStatus::PartiallyFilled);
    EXPECT_EQ(sell.remainingQuantity().get(),6);
    EXPECT_TRUE(book.contains(OrderID{1}));
    EXPECT_FALSE(book.contains(OrderID{2}));
    EXPECT_EQ(book.orderCount(), 1);
    ASSERT_NE(book.bestAsk(), nullptr);
    EXPECT_EQ(book.bestAsk()->price(), Price{100});
}

TEST_F(OrderBookTest, PriceTimePriority)
{
    auto sellA = makeSell(
        OrderID{1},
        Price{100},
        Quantity{5}
    );

    auto sellB = makeSell(
        OrderID{2},
        Price{100},
        Quantity{5}
    );

    book.submitOrder(sellA);
    book.submitOrder(sellB);

    auto buy = makeBuy(
        OrderID{3},
        Price{100},
        Quantity{7}
    );

    auto trades = book.submitOrder(buy);

    ASSERT_EQ(trades.size(), 2);
    EXPECT_EQ(trades[0].restingId, OrderID{1});
    EXPECT_EQ(trades[0].quantity, Quantity{5});
    EXPECT_EQ(trades[1].restingId, OrderID{2});
    EXPECT_EQ(trades[1].quantity, Quantity{2});
    EXPECT_TRUE(sellA.isFilled());
    EXPECT_EQ(sellB.status(),OrderStatus::PartiallyFilled);
    EXPECT_EQ(sellB.remainingQuantity().get(),3);
    EXPECT_TRUE(book.contains(OrderID{2}));
    EXPECT_FALSE(book.contains(OrderID{1}));
    EXPECT_FALSE(book.contains(OrderID{3}));
    EXPECT_EQ(book.orderCount(), 1);
}

TEST_F(OrderBookTest, MultiLevelWalk)
{
    auto ask1 = makeSell(
        OrderID{1},
        Price{101},
        Quantity{5}
    );

    auto ask2 = makeSell(
        OrderID{2},
        Price{102},
        Quantity{7}
    );

    auto ask3 = makeSell(
        OrderID{3},
        Price{103},
        Quantity{4}
    );

    book.submitOrder(ask1);
    book.submitOrder(ask2);
    book.submitOrder(ask3);

    auto buy = makeBuy(
        OrderID{10},
        Price{105},
        Quantity{20}
    );

    auto trades = book.submitOrder(buy);

    ASSERT_EQ(trades.size(), 3);
    EXPECT_EQ(trades[0].price, Price{101});
    EXPECT_EQ(trades[0].quantity, Quantity{5});
    EXPECT_EQ(trades[1].price, Price{102});
    EXPECT_EQ(trades[1].quantity, Quantity{7});
    EXPECT_EQ(trades[2].price, Price{103});
    EXPECT_EQ(trades[2].quantity, Quantity{4});
    EXPECT_TRUE(ask1.isFilled());
    EXPECT_TRUE(ask2.isFilled());
    EXPECT_TRUE(ask3.isFilled());
    EXPECT_FALSE(book.contains(OrderID{1}));
    EXPECT_FALSE(book.contains(OrderID{2}));
    EXPECT_FALSE(book.contains(OrderID{3}));
    EXPECT_TRUE(book.contains(OrderID{10}));
    EXPECT_EQ(buy.remainingQuantity().get(),4);
    ASSERT_NE(book.bestBid(), nullptr);
    EXPECT_EQ(book.bestBid()->price(),Price{105});
    EXPECT_EQ(book.orderCount(), 1);
}

TEST_F(OrderBookTest, CancelOrderRemovesRestingOrder)
{
    auto buy = makeBuy(
        OrderID{1},
        Price{100},
        Quantity{10}
    );

    book.submitOrder(buy);

    ASSERT_TRUE(book.contains(OrderID{1}));
    ASSERT_EQ(book.orderCount(), 1u);

    bool cancelled = book.cancelOrder(OrderID{1});

    EXPECT_TRUE(cancelled);
    EXPECT_FALSE(book.contains(OrderID{1}));
    EXPECT_TRUE(book.empty());
    EXPECT_EQ(book.orderCount(), 0u);
    EXPECT_EQ(book.bestBid(), nullptr);
    EXPECT_EQ(book.bestAsk(), nullptr);
    EXPECT_TRUE(buy.isCancelled());
}

TEST_F(OrderBookTest, CancelMissingOrderReturnsFalse)
{
    EXPECT_FALSE(book.cancelOrder(OrderID{999}));
}

TEST_F(OrderBookTest, MarketOrderDoesNotRest)
{
    Order market_buy(
        OrderID{1},
        Side::Buy,
        OrderType::Market,
        TimeInForce::IOC,
        Price{1},              // Market orders carry a dummy price, matching logic ignores it.
        Quantity{10},
        SequenceNumber{1},
        EventTimestamp{0}
    );

    auto trades = book.submitOrder(market_buy);

    EXPECT_TRUE(trades.empty());
    EXPECT_TRUE(book.empty());
    EXPECT_EQ(book.orderCount(), 0u);
    EXPECT_FALSE(book.contains(OrderID{1}));
    EXPECT_EQ(book.bestBid(), nullptr);
    EXPECT_EQ(book.bestAsk(), nullptr);
}

TEST_F(OrderBookTest, MarketOrderConsumesLiquidity)
{
    auto sell = makeSell(
        OrderID{1},
        Price{100},
        Quantity{10}
    );

    book.submitOrder(sell);

    Order market_buy(
        OrderID{2},
        Side::Buy,
        OrderType::Market,
        TimeInForce::IOC,
        Price{1},
        Quantity{5},
        SequenceNumber{2},
        EventTimestamp{0}
    );

    auto trades = book.submitOrder(market_buy);

    ASSERT_EQ(trades.size(), 1u);
    EXPECT_EQ(trades[0].price, Price{100});
    EXPECT_EQ(trades[0].quantity, Quantity{5});
    EXPECT_TRUE(market_buy.isFilled());
    EXPECT_FALSE(book.contains(OrderID{2}));
    EXPECT_TRUE(book.contains(OrderID{1}));
    EXPECT_EQ(sell.remainingQuantity(),Volume{5});
    EXPECT_EQ(book.orderCount(), 1u);
    ASSERT_NE(book.bestAsk(), nullptr);
    EXPECT_EQ(book.bestAsk()->price(), Price{100});
}

TEST_F(OrderBookTest, BestPriceSelection)
{
    auto bid1 = makeBuy(OrderID{1}, Price{100}, Quantity{10});
    auto bid2 = makeBuy(OrderID{2}, Price{105}, Quantity{10});
    auto bid3 = makeBuy(OrderID{3}, Price{103}, Quantity{10});

    book.submitOrder(bid1);
    book.submitOrder(bid2);
    book.submitOrder(bid3);

    ASSERT_NE(book.bestBid(), nullptr);
    EXPECT_EQ(book.bestBid()->price(), Price{105});

    auto ask1 = makeSell(OrderID{4}, Price{110}, Quantity{10});
    auto ask2 = makeSell(OrderID{5}, Price{108}, Quantity{10});
    auto ask3 = makeSell(OrderID{6}, Price{112}, Quantity{10});

    book.submitOrder(ask1);
    book.submitOrder(ask2);
    book.submitOrder(ask3);
    ASSERT_NE(book.bestAsk(), nullptr);
    EXPECT_EQ(book.bestAsk()->price(), Price{108});
}

TEST_F(OrderBookTest, PartialFillPreservesPriceLevel)
{
    auto sell = makeSell(
        OrderID{1},
        Price{100},
        Quantity{10}
    );

    book.submitOrder(sell);

    auto buy = makeBuy(
        OrderID{2},
        Price{100},
        Quantity{5}
    );

    auto trades = book.submitOrder(buy);

    ASSERT_EQ(trades.size(), 1u);
    EXPECT_EQ(trades[0].price, Price{100});
    EXPECT_EQ(trades[0].quantity, Quantity{5});
    EXPECT_TRUE(book.contains(OrderID{1}));
    EXPECT_FALSE(book.contains(OrderID{2}));
    EXPECT_EQ(sell.remainingQuantity(), Volume{5});
    ASSERT_NE(book.bestAsk(), nullptr);
    EXPECT_EQ(book.bestAsk()->price(), Price{100});
    EXPECT_EQ(book.orderCount(), 1u);
}

TEST_F(OrderBookTest, CancelPartiallyFilledOrder)
{
    auto sell = makeSell(
        OrderID{1},
        Price{100},
        Quantity{10}
    );

    book.submitOrder(sell);

    auto buy = makeBuy(
        OrderID{2},
        Price{100},
        Quantity{4}
    );

    auto trades = book.submitOrder(buy);

    ASSERT_EQ(trades.size(), 1u);
    EXPECT_EQ(sell.remainingQuantity(),Volume{6});
    EXPECT_EQ(sell.status(),OrderStatus::PartiallyFilled);
    EXPECT_TRUE(book.contains(OrderID{1}));
    EXPECT_TRUE(book.cancelOrder(OrderID{1}));
    EXPECT_TRUE(sell.isCancelled());
    EXPECT_FALSE(book.contains(OrderID{1}));
    EXPECT_TRUE(book.empty());
    EXPECT_EQ(book.orderCount(), 0u);
    EXPECT_EQ(book.bestAsk(), nullptr);
}

TEST_F(OrderBookTest, CancelOneOrderAtSharedPriceLevel)
{
    auto buyA = makeBuy(
        OrderID{1},
        Price{100},
        Quantity{10}
    );

    auto buyB = makeBuy(
        OrderID{2},
        Price{100},
        Quantity{10}
    );

    book.submitOrder(buyA);
    book.submitOrder(buyB);

    ASSERT_EQ(book.orderCount(), 2u);
    EXPECT_TRUE(book.contains(OrderID{1}));
    EXPECT_TRUE(book.contains(OrderID{2}));
    EXPECT_TRUE(book.cancelOrder(OrderID{2}));
    EXPECT_TRUE(book.contains(OrderID{1}));
    EXPECT_FALSE(book.contains(OrderID{2}));
    EXPECT_TRUE(buyB.isCancelled());
    EXPECT_EQ(book.orderCount(), 1u);
    ASSERT_NE(book.bestBid(), nullptr);
    EXPECT_EQ(book.bestBid()->price(), Price{100});
}

TEST_F(OrderBookTest, ExactMultiLevelExhaustion)
{
    auto sell1 = makeSell(
        OrderID{1},
        Price{100},
        Quantity{5}
    );

    auto sell2 = makeSell(
        OrderID{2},
        Price{101},
        Quantity{5}
    );

    auto sell3 = makeSell(
        OrderID{3},
        Price{102},
        Quantity{5}
    );

    book.submitOrder(sell1);
    book.submitOrder(sell2);
    book.submitOrder(sell3);

    auto buy = makeBuy(
        OrderID{4},
        Price{102},
        Quantity{15}
    );

    auto trades = book.submitOrder(buy);

    ASSERT_EQ(trades.size(), 3u);
    EXPECT_EQ(trades[0].price, Price{100});
    EXPECT_EQ(trades[1].price, Price{101});
    EXPECT_EQ(trades[2].price, Price{102});
    EXPECT_TRUE(buy.isFilled());
    EXPECT_FALSE(book.contains(OrderID{1}));
    EXPECT_FALSE(book.contains(OrderID{2}));
    EXPECT_FALSE(book.contains(OrderID{3}));
    EXPECT_TRUE(book.empty());
    EXPECT_EQ(book.orderCount(), 0u);
    EXPECT_EQ(book.bestAsk(), nullptr);
}

TEST_F(OrderBookTest, ReduceOrderQuantity)
{
    auto buy = makeBuy(
        OrderID{1},
        Price{100},
        Quantity{10}
    );

    book.submitOrder(buy);

    ASSERT_EQ(book.orderCount(), 1u);
    EXPECT_TRUE(book.contains(OrderID{1}));
    EXPECT_EQ(buy.remainingQuantity(), Volume{10});

    bool reduced = book.reduceOrder(OrderID{1}, Quantity{4});

    EXPECT_TRUE(reduced);
    EXPECT_EQ(buy.remainingQuantity(), Volume{6});
    EXPECT_TRUE(book.contains(OrderID{1}));
    EXPECT_EQ(book.orderCount(), 1u);
}

TEST_F(OrderBookTest, ReduceOrderToZero1)
{
    auto buy = makeBuy(
        OrderID{1},
        Price{100},
        Quantity{100}
    );

    book.submitOrder(buy);

    EXPECT_TRUE(
        book.reduceOrder(
            OrderID{1},
            Quantity{40}
        )
    );

    EXPECT_TRUE(book.contains(OrderID{1}));

    EXPECT_EQ(
        buy.remainingQuantity(),
        Volume{60}
    );

    EXPECT_EQ(book.orderCount(), 1);

    ASSERT_NE(book.bestBid(), nullptr);

    EXPECT_EQ(
        book.bestBid()->price(),
        Price{100}
    );
}

TEST_F(OrderBookTest, ReduceOrderToZeroRemovesOrder)
{
    auto buy = makeBuy(
        OrderID{1},
        Price{100},
        Quantity{100}
    );

    book.submitOrder(buy);

    EXPECT_TRUE(
        book.reduceOrder(
            OrderID{1},
            Quantity{100}
        )
    );

    EXPECT_FALSE(
        book.contains(OrderID{1})
    );

    EXPECT_TRUE(book.empty());

    EXPECT_EQ(book.orderCount(), 0);

    EXPECT_EQ(book.bestBid(), nullptr);
}