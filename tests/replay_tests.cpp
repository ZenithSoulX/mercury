#include <gtest/gtest.h>
#include "replay/replay_engine.hpp"
#include "domain/order_book.hpp"
using namespace mercury;

TEST(ReplayEngineTest, MiniFileProducesExpectedFinalBookState) {
    OrderBook book;
    ReplayEngine engine("../../tests/fixtures/mini_message.csv", book);

    std::size_t processed = engine.runAll();
    EXPECT_EQ(processed, 9u);

    EXPECT_EQ(book.orderCount(), 1u);
    EXPECT_EQ(book.bestBid(), nullptr);

    ASSERT_NE(book.bestAsk(), nullptr);
    EXPECT_EQ(book.bestAsk()->price(), Price{1001000});
    EXPECT_EQ(book.bestAsk()->totalVolume(), Volume{300});

    EXPECT_FALSE(book.contains(OrderID{1001}));
    EXPECT_FALSE(book.contains(OrderID{1002}));
    EXPECT_FALSE(book.contains(OrderID{1003}));
    EXPECT_FALSE(book.contains(OrderID{1004}));
    EXPECT_TRUE(book.contains(OrderID{1005}));
    EXPECT_FALSE(book.contains(OrderID{1006}));

    EXPECT_EQ(engine.hiddenExecutionCount(), 0u);
    EXPECT_EQ(engine.haltCount(), 0u);
}
