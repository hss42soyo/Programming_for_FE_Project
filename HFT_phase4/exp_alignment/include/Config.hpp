#pragma once

#if USE_ALIGN64
#define HFT_ALIGN alignas(64)
#else
#define HFT_ALIGN
#endif

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
