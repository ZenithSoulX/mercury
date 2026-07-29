cmake_minimum_required(VERSION 3.20)

project(
    Mercury
    VERSION 0.1.0
    DESCRIPTION "High-performance limit order book"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ------------------------------------------------------------------------------
# Warnings
# ------------------------------------------------------------------------------

if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
    )
endif()

# ------------------------------------------------------------------------------
# Mercury Library
# ------------------------------------------------------------------------------

add_library(mercury
    src/order.cpp
    src/price_level.cpp
    src/price_index.cpp
    src/book_side.cpp
    src/order_book.cpp
)

target_include_directories(mercury
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

# ------------------------------------------------------------------------------
# Experiments
# ------------------------------------------------------------------------------

add_executable(layout
    experiments/layout.cpp
)

target_link_libraries(layout
    PRIVATE
        mercury
)