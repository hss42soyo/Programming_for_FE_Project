#pragma once

// 4 variables (compile-time):
// 0/1 only. CMake will set these via -D...=0/1 when configuring.

#ifndef USE_RAW_PTR
#define USE_RAW_PTR 0    // 0=unique_ptr, 1=raw pointer
#endif

#ifndef USE_ALIGN64
#define USE_ALIGN64 1    // 1=alignas(64) on MarketData, 0=off
#endif

#ifndef USE_POOL_ALLOC
#define USE_POOL_ALLOC 0 // 1=custom pool for Order, 0=new/delete
#endif

#ifndef BOOK_IMPL
#define BOOK_IMPL 0      // 0=multimap, 1=flat vector (sorted insert)
#endif
