cmake_minimum_required(VERSION 3.10)
project(HFT_Phase4)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(hft_phase4_lib
    src/MarketData.cpp
    src/OrderBook.cpp
    src/MatchingEngine.cpp
    src/OrderManager.cpp
    src/TradeLogger.cpp
)

target_include_directories(hft_phase4_lib PUBLIC include)

add_executable(hft_phase4_app
    src/main.cpp
)
target_link_libraries(hft_phase4_app PRIVATE hft_phase4_lib)

# optional test
add_executable(hft_phase4_test test/test_latency.cpp)
target_link_libraries(hft_phase4_test PRIVATE hft_phase4_lib)