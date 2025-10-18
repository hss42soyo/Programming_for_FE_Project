cmake_minimum_required(VERSION 3.10)
project(HFT_Phase4_Alignment)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# option for alignas(64)
option(USE_ALIGN64 "Enable alignas(64) on hot structs" OFF)

add_library(hft_align_lib
    src/MarketData.cpp
    src/OrderBook.cpp
    src/MatchingEngine.cpp
    src/OrderManager.cpp
    src/TradeLogger.cpp
)
target_include_directories(hft_align_lib PUBLIC include)

add_executable(hft_align_app src/main.cpp)
target_link_libraries(hft_align_app PRIVATE hft_align_lib)

if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/test/test_latency.cpp")
  add_executable(hft_align_test test/test_latency.cpp)
  target_link_libraries(hft_align_test PRIVATE hft_align_lib)
endif()

foreach(tgt hft_align_lib hft_align_app hft_align_test)
  if (TARGET ${tgt})
    target_compile_definitions(${tgt} PRIVATE
      USE_ALIGN64=$<IF:$<BOOL:${USE_ALIGN64}>,1,0>
    )
  endif()
endforeach()

# fix csv output path
target_compile_definitions(hft_align_app PRIVATE CSV_DIR="${CMAKE_BINARY_DIR}")
