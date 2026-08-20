add_executable(
    order_book_tests
    order_book_tests.cpp
)

target_link_libraries(
    order_book_tests
    PRIVATE
        mercury
        GTest::gtest_main
)

target_compile_options(order_book_tests PRIVATE
    -fsanitize=address,undefined
    -fno-omit-frame-pointer
)

target_link_options(order_book_tests PRIVATE
    -fsanitize=address,undefined
)

include(GoogleTest)

gtest_discover_tests(order_book_tests)