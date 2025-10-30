#pragma once
#include <cstddef>
#include <memory>
#include <vector>

// Simple fixed-size object pool. Not thread-safe.
template <typename T, std::size_t CHUNK = 4096>
class ObjectPool {
   struct Node { Node* next; };

   std::vector<std::unique_ptr<char[]>> blocks_;
   Node* free_ = nullptr;

   void allocate_block() {
      // allocate CHUNK contiguous T objects
      auto block = std::make_unique<char[]>(CHUNK * sizeof(T));
      char* base = block.get();
      // push all nodes into free list
      for (std::size_t i = 0; i < CHUNK; ++i) {
         auto* n = reinterpret_cast<Node*>(base + i * sizeof(T));
         n->next = free_;
         free_ = n;
      }
      blocks_.push_back(std::move(block));
   }

public:
   ObjectPool() = default;

   void* allocate() {
      if (!free_) allocate_block();
      Node* n = free_;
      free_ = n->next;
      return n;
   }

   void deallocate(void* p) noexcept {
      auto* n = static_cast<Node*>(p);
      n->next = free_;
      free_ = n;
   }
};
