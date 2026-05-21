//===-- i_builder_tree.h ----------------------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#ifndef EEA73704_BBAE_43AD_9799_F1F919E04250
#define EEA73704_BBAE_43AD_9799_F1F919E04250

#include "compiler/ast/nodes/interfaces/i_node.h"
#include "compiler/lexer/lexer.h"
#include "llvm/Support/Allocator.h"
#include <cstddef>
#include <llvm/ADT/SmallVector.h>
#include <new>
#include <utility>
#include <vector>

class IBuilderTree
{
public:
virtual ~IBuilderTree() = default;
    IBuilderTree() = default;
    IBuilderTree(const IBuilderTree&) = delete;
    auto operator=(const IBuilderTree&) -> IBuilderTree& = delete;
    IBuilderTree(IBuilderTree&&) = delete;
    auto operator=(IBuilderTree&&) -> IBuilderTree& = delete;
    
    virtual auto build(std::vector<Token>& tokens) -> INode* = 0;
    virtual auto build(std::vector<Token>& tokens, std::size_t& index) -> INode* = 0;
    virtual auto getArena() -> llvm::BumpPtrAllocator& = 0;

    template<typename T, typename... Args>
    auto allocate(Args&&... args) -> T* {
        void* mem = getArena().Allocate(sizeof(T), alignof(T));
        T* obj = new (mem) T(std::forward<Args>(args)...); // NOLINT(cppcoreguidelines-owning-memory)
        return obj;
    }

    // We need to copy the values of the array into the bump allocator because we do not know in advance the memory size it will take; it is a vector that can grow and take more elements depending on the circumstances.
    // We must make this choice to gain performance in the future during data destruction and also for data immutability, a single copy for future reading
    // Two fundamental advantages of this pattern: cache locality (fast reading thanks to memory contiguity) and global deallocation (Arena destruction in O(1)).
    template<typename T>
    auto allocateArray(llvm::ArrayRef<T> elements) -> llvm::ArrayRef<T> {
        if (elements.empty()) 
        { 
            return {};

        }
        T* mem = static_cast<T*>(getArena().Allocate(elements.size() * sizeof(T), alignof(T)));
        std::uninitialized_copy(elements.begin(), elements.end(), mem);
        return llvm::ArrayRef<T>(mem, elements.size());
    }

    static constexpr std::size_t kArenaAlignment = 8;

    auto operator new(std::size_t size) -> void* { return ::operator new(size); }
    static void operator delete(void* ptr) { ::operator delete(ptr); }

    auto operator new(std::size_t size, llvm::BumpPtrAllocator& arena) -> void* {
        return arena.Allocate(size, kArenaAlignment); 
    }
    
    static void operator delete(void* ptr, llvm::BumpPtrAllocator& allocator) {
        // Do nothing here!
        // The BumpPtrAllocator frees all memory at once upon its destruction.
        (void)ptr;
        (void)allocator;
    }
};

#endif /* EEA73704_BBAE_43AD_9799_F1F919E04250 */
