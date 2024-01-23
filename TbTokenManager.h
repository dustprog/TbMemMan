/**
Author & Copyright (C) 2017 Johannes Bernhard Steffens.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

//================================================================================================================
// TbTokenManager.h
//
//  Fragmentation-free and fast (O(1)) pool based dynamic management using fixed sized blocks.
//  A free block is identified by a token representing its address. Tokens are managed in a stack.
//  An alloc-request consumes the top token from stack. A free-request pushes the token back onto the stack.
//
//  The instance token_manager_s occupies the memory-pool; being its header. This supports efficient (O(log(n))
//  determination of the correct token-manager by the memory-manager (s. algorithm below).
//
//  Token managers can be run in full-alignment-mode in which they are aligned to pool_size, which is
//  a power of two. This allows O(1) lookup of the pool manager from any of its managed allocations.
//
//================================================================================================================
// History
//
// Created on 12/22/2023 by Dustin Watson
//================================================================================================================
#ifndef __TBTOKEN_MANAGER_H
#define __TBTOKEN_MANAGER_H
//================================================================================================================
//================================================================================================================

//
// Includes
//

#include "TbManUtility.h"
#include "Callback.h"
//#include "TbBlockManager.h"

//================================================================================================================
//================================================================================================================
namespace Core {
    class TbBlockManager;
    class TbTokenManager
    {
    public:

        void init();
        void down();

        static TbTokenManager* create(size_t pool_size, size_t block_size, bool align);
        static void discard(Core::TbTokenManager* tm);

        bool isFull();
        bool isEmpty();

        void* alloc();

        void free(void* ptr);

        size_t totalAlloc();
        size_t totalInstances();
        size_t totalSpace();

        void forEachInstance(void_callback1 cb, void* arg);

        void printStatus(int detail_level);

    public:

        size_t getPoolSize() { return mPoolSize; }
        void setPoolSize(size_t value) { mPoolSize = value; }

        size_t getBlockSize() { return mBlockSize; }
        void setBlockSize(size_t value) { mBlockSize = value; }

        uint16_t getStackSize() { return mStackSize; }
        void setStackSize(uint16_t value) { mStackSize = value; }

        uint16_t getStackIndex() { return mStackIndex; }
        void setStackIndex(uint16_t value) { mStackIndex = value; }

        bool isAligned() { return mAligned; }
        void setIsAligned(bool value) { mAligned = value; }

        TbBlockManager* getParent() { return mParent; }
        void setParent(TbBlockManager* value) { mParent = value; }

        size_t getParentIndex() { return mParentIndex; }
        void setParentIndex(size_t value) { mParentIndex = value; }

        uint16_t getTokenStack(size_t i) { return mTokenStack[i]; }
        void setTokenStack(uint16_t value, size_t i) { mTokenStack[i] = value; }

    private:

        size_t mPoolSize;
        size_t mBlockSize;
        
        // size of token-stack
        uint16_t mStackSize;

        // index into token-stack
        uint16_t mStackIndex;

        /**
         *  The memory-pool is considered aligned when the integer-evaluation of its address
         *  is a multiple of pool_size, which means that the pool address can be obtained
         *  from any pointer inside the pool by a mere integer division.
         */
        bool mAligned;

        TbBlockManager* mParent;

        size_t mParentIndex;

        // stack of block-tokens (part of pool)
        uint16_t mTokenStack[];
    };
}
//================================================================================================================
//================================================================================================================
#endif//__TBTOKEN_MANAGER_H