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

enable_testing()
include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.17.0
)

FetchContent_MakeAvailable(googletest)
add_subdirectory(tests)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
    )
endif()

option(MERCURY_ENABLE_SANITIZERS "Enable sanitizers" ON)

if(MERCURY_ENABLE_SANITIZERS AND NOT MSVC)
    add_compile_options(
        -fsanitize=address,undefined
        -fno-omit-frame-pointer
    )
    add_link_options(
        -fsanitize=address,undefined
    )
    
endif()

add_library(mercury
    src/order.cpp
    src/price_level.cpp
    src/price_index.cpp
    src/book_side.cpp
    src/order_book.cpp
    src/lobster_parser.cpp
)

target_include_directories(mercury
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

add_executable(layout
    experiments/layout.cpp
)

target_link_libraries(layout
    PRIVATE
        mercury
)

add_executable(replay_demo
    experiments/replay_demo.cpp
)

target_link_libraries(replay_demo
    PRIVATE
        mercury
)

add_executable(lobster_stats
    experiments/lobster_stats.cpp)
 target_link_libraries(lobster_stats
    PRIVATE
        mercury)