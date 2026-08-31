#include <gtest/gtest.h>
#include "replay/lobster_parser.hpp"
#include <filesystem>

using namespace mercury;

namespace {
constexpr const char* kMiniMessageFile = "../../tests/fixtures/mini_message.csv";
}

TEST(LobsterParserTest, OpensValidFileSuccessfully) {
    std::cout
        << "Current path: "
        << std::filesystem::current_path()
        << '\n';
    std::ifstream f(kMiniMessageFile);
    std::cout
        << "Fixture path: "
        << kMiniMessageFile
        << '\n';
    LobsterParser parser(kMiniMessageFile);
    std::cout
        << "Parser good = "
        << parser.good()
        << '\n';
    EXPECT_TRUE(parser.good());
}

TEST(LobsterParserTest, FailsGracefullyOnMissingFile) {
    LobsterParser parser("this_file_does_not_exist.csv");
    EXPECT_FALSE(parser.good());
}

TEST(LobsterParserTest, ParsesAllNineRows) {
    LobsterParser parser(kMiniMessageFile);
    ASSERT_TRUE(parser.good());

    int count = 0;
    while (parser.next().has_value()) {
        ++count;
    }
    EXPECT_EQ(count, 9);
}

TEST(LobsterParserTest, ParsesFirstRowFieldsExactly) {
    LobsterParser parser(kMiniMessageFile);
    ASSERT_TRUE(parser.good());

    auto msg = parser.next();
    ASSERT_TRUE(msg.has_value());

    EXPECT_EQ(msg->timestamp, 34200'100000000ULL);
    EXPECT_EQ(msg->event_type, LobsterEventType::Submission);
    EXPECT_EQ(msg->order_id, 1001u);
    EXPECT_EQ(msg->size, 100u);
    EXPECT_EQ(msg->price, 1000000u);
    EXPECT_EQ(msg->direction, 1);
}

TEST(LobsterParserTest, ParsesPartialCancellationRow) {
    LobsterParser parser(kMiniMessageFile);
    ASSERT_TRUE(parser.good());

    for (int i = 0; i < 8; ++i) {
        ASSERT_TRUE(parser.next().has_value());
    }
    auto msg = parser.next();
    ASSERT_TRUE(msg.has_value());

    EXPECT_EQ(msg->event_type, LobsterEventType::PartialCancellation);
    EXPECT_EQ(msg->order_id, 1006u);
    EXPECT_EQ(msg->size, 50u);
    EXPECT_EQ(msg->price, 1000000u);
    EXPECT_EQ(msg->direction, -1);
}

TEST(LobsterParserTest, ReturnsNulloptAfterLastRow) {
    LobsterParser parser(kMiniMessageFile);
    ASSERT_TRUE(parser.good());

    for (int i = 0; i < 9; ++i) {
        ASSERT_TRUE(parser.next().has_value());
    }
    EXPECT_FALSE(parser.next().has_value());
}