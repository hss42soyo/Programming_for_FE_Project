cmake_minimum_required(VERSION 3.10)
project(HFT_Phase4_Allocator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# default use new/delete
option(USE_POOL_ALLOC "Use custom memory pool for Order" OFF)

add_library(hft_alloc_lib
    src/MarketData.cpp
    src/OrderBook.cpp
    src/MatchingEngine.cpp
    src/OrderManager.cpp
    src/TradeLogger.cpp
)
target_include_directories(hft_alloc_lib PUBLIC include)

add_executable(hft_alloc_app src/main.cpp)
target_link_libraries(hft_alloc_app PRIVATE hft_alloc_lib)

if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/test/test_latency.cpp")
  add_executable(hft_alloc_test test/test_latency.cpp)
  target_link_libraries(hft_alloc_test PRIVATE hft_alloc_lib)
endif()

foreach(tgt hft_alloc_lib hft_alloc_app hft_alloc_test)
  if (TARGET ${tgt})
    target_compile_definitions(${tgt} PRIVATE
      USE_POOL_ALLOC=$<IF:$<BOOL:${USE_POOL_ALLOC}>,1,0>
    )
  endif()
endforeach()

# CSV output path
target_compile_definitions(hft_alloc_app PRIVATE CSV_DIR="${CMAKE_BINARY_DIR}")
