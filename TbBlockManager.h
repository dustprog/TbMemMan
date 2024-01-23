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
// TbBlockManager.h
//
//  Contains an array of token-managers, each of the same block-size.
//  A token-manager has one of three stats: 'full', 'free' and 'empty'.
//  A  'full'  token-manager has no space left for allocation
//  A  'free'  token-manager has (some) space available for allocation.
//  An 'empty' token-manager has all space available for allocation.
//  Token managers are linearly arranged by state in the order: full, free, empty.
//  An index (free_index) points to the full-free border.
//
//  Alloc request: O(1)
//    - redirected to the free_indexe(d) token-manager.
//    - if that token-manager becomes 'full', free_index is incremented
//    - if all token-managers are full, a new token-manager is appended at the next alloc request
//
//  Free request:
//    - block_manager_s does not directly receive free requests. Instead the parent-manager directly invokes the
//      the corresponding token manager.
//    - If a token-manager turns from full to free, it reports to the block manager, which swaps its position
//      with the last full token_manager and decrements free_index.
//    - If a token-manager turns from free to empty, it reports to the block manager, which swaps its position
//      with the last free token_manager. When enough empty token-managers accumulated (sweep_hysteresis), they
//      are discarded (memory returned to the system).
//
//================================================================================================================
// History
//
// Created on 12/22/2023 by Dustin Watson
//================================================================================================================
#ifndef __TBBLOCK_MANAGER_H
#define __TBBLOCK_MANAGER_H
//================================================================================================================
//================================================================================================================

//
// Includes
//

#include "TbManUtility.h"
#include "TbTokenManager.h"
#include "TbMemMan.h"
#include "TbTreeIntVD.h"
#include "Callback.h"

//================================================================================================================
//================================================================================================================
namespace Core {
    class TbBlockManager
    {
    public:
        
        void init();
        void down();

        static TbBlockManager* create(size_t pool_size, size_t block_size, bool align);
        static void discard(TbBlockManager* bm);

        void* alloc();

        // A child reports turning full --> free
        void fullToFree(TbTokenManager* child);

        size_t emptyTail();

        // A child reports turning free --> empty
        void freeToEmpty(TbTokenManager* child);

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

        size_t getSize() { return mSize; }
        void setSize(size_t value) { mSize = value; }

        size_t getSpace() { return mSpace; }
        void setSpace(size_t value) { mSpace = value; }

        bool isAligned() { return mAligned; }
        void setIsAligned(bool value) { mAligned = value; }

        bool getAlign() { return mAlign; }
        void setAlign(bool value) { mAlign = value; }

        size_t getFreeIndex() { return mFreeIndex; }
        void setFreeIndex(size_t value) { mFreeIndex = value; }

        double getSweepHysteresis() { return mSweepHysteresis; }
        void setSweepHysteresis(double value) { mSweepHysteresis = value; }

        void setInternalBTree(TbTreeIntVD* value) { mInternalBTree = value; }

        void setParent(TbMemMan* value) { mParent = value; }

    private:

        // pool size of all token-managers
        size_t mPoolSize;

        // block size of all token-managers
        size_t mBlockSize;

        // attempt to align token_managers to mPoolSize
        bool mAlign;

        TbTokenManager** mData;

        size_t mSize;
        size_t mSpace;

        // entries equal or above mFreeIndex have space for allocation
        size_t mFreeIndex;

        // if ( empty token-managers ) / ( used token-managers ) < sweep_hysteresis, empty token-managers are discarded
        double mSweepHysteresis;

        // all token managers are aligned to mPoolSize
        bool mAligned;

        TbMemMan* mParent;

        TbTreeIntVD* mInternalBTree;
    };
}
//================================================================================================================
//================================================================================================================
#endif//__TBBLOCK_MANAGER_H