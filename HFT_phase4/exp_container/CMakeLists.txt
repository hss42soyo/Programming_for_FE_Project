cmake_minimum_required(VERSION 3.10)
project(HFT_Phase4_Container)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Container layout switch
option(USE_FLAT_CONTAINER "Use flat array (vector) instead of multimap" OFF)

add_library(hft_container_lib
    src/MarketData.cpp
    src/OrderBook.cpp
    src/MatchingEngine.cpp
    src/OrderManager.cpp
    src/TradeLogger.cpp
)
target_include_directories(hft_container_lib PUBLIC include)

add_executable(hft_container_app src/main.cpp)
target_link_libraries(hft_container_app PRIVATE hft_container_lib)

if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/test/test_latency.cpp")
  add_executable(hft_container_test test/test_latency.cpp)
  target_link_libraries(hft_container_test PRIVATE hft_container_lib)
endif()

foreach(tgt hft_container_lib hft_container_app hft_container_test)
  if (TARGET ${tgt})
    target_compile_definitions(${tgt} PRIVATE
      USE_FLAT_CONTAINER=$<IF:$<BOOL:${USE_FLAT_CONTAINER}>,1,0>
    )
  endif()
endforeach()

target_compile_definitions(hft_container_app PRIVATE CSV_DIR="${CMAKE_BINARY_DIR}")
