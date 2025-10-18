cmake_minimum_required(VERSION 3.10)
project(HFT_Phase4)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(USE_RAW_PTR "Use raw pointers instead of unique_ptr" OFF)

add_library(hft_pointers_lib
    src/MarketData.cpp
    src/OrderBook.cpp
    src/MatchingEngine.cpp
    src/OrderManager.cpp
    src/TradeLogger.cpp
)
target_include_directories(hft_pointers_lib PUBLIC include)

add_executable(hft_pointers_app src/main.cpp)
target_link_libraries(hft_pointers_app PRIVATE hft_pointers_lib)

# optional test
if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/test/test_latency.cpp")
  add_executable(hft_pointers_test test/test_latency.cpp)
  target_link_libraries(hft_pointers_test PRIVATE hft_pointers_lib)
endif()

foreach(tgt hft_pointers_lib hft_pointers_app hft_pointers_test)
  if (TARGET ${tgt})
    target_compile_definitions(${tgt} PRIVATE
      USE_RAW_PTR=$<IF:$<BOOL:${USE_RAW_PTR}>,1,0>
    )
  endif()
endforeach()

# ---- ASan: default OFF----
option(ENABLE_ASAN "Enable AddressSanitizer" OFF)

# MSVC's ASan indicator£º/fsanitize=address
if (MSVC AND ENABLE_ASAN)
  add_compile_options(/fsanitize=address)
  add_link_options(/fsanitize=address)
endif()

# fix csv output path
target_compile_definitions(hft_pointers_app PRIVATE CSV_DIR="${CMAKE_BINARY_DIR}")
