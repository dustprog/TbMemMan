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
// TbMemMan.h
//
// Memory-Manager
//
//  Contains a fixed-size array of block-managers with exponentially increasing block_size.
//  (E.g. via size-doubling, but other arrangements are also possible)
//
//  Alloc request:
//     - directed to the block-manager with the smallest fitting bock-size
//     - if the largest block size is yet too small, the request is passed on to the OS (-->aligned_alloc)
//       --> O(1) for size requests equal or below largest block size assuming alloc and free requests are statistically
//           balanced such the overall memory in use is not dramatically varying.
//
//  Free request:
//     - If the previously allocated size is available and all token managers are aligned
//       the address of the token manager is directly calculated from the allocated address. (O(1))
//     - Otherwise: The corresponding token-manager is determined via internal_btree from the memory-address
//       (O(log(n)) - where 'n' is the current amount of token managers.)
//
//================================================================================================================
// History
//
// Created on 12/22/2023 by Dustin Watson
//================================================================================================================
#ifndef __TBMAN_H
#define __TBMAN_H
//================================================================================================================
//================================================================================================================

//
// Includes
//

#include "TbManUtility.h"
#include "TbBlockManager.h"
#include "TbTreeExtPS.h"
#include "TbTreeIntVD.h"
#include "Callback.h"
#include "tinycthread.h"

/// Minimum alignment of memory blocks
#define TBMAN_ALIGN 0x100

//================================================================================================================
//================================================================================================================
namespace Core {
    class TbMemMan
    {
    public:
        
        TbMemMan();
        ~TbMemMan();

        void init(size_t poolSize, size_t minBlockSize, size_t maxBlockSize, size_t steppingMethod, bool fullAlign);

        void down();

        /// opens global memory manager (call this once before first usage of global Core functions below)
        static void open();

        /// closes global memory manager (call this once at the end of your program)
        
        // testing closes this
        void close();

        /// creates a dedicated memory manager instance ( close with discard )
        //static TbMemMan* instanceOpen();

        /// Creates a dedicated manager with default parameters
        static TbMemMan* createDefault();

        /// Creates a dedicated manager with specified parameters (consider using createDefault)
        static TbMemMan* create(size_t poolSize, size_t minBlockSize, size_t maxBlockSize, size_t steppingMethod, bool fullAlign);

        /// closes dedicated  memory manager instance
        static void discard(TbMemMan* value);

        /**********************************************************************************************************************/
        /** Advanced memory management using the internal manager (thread-safe).
         *  This function provides allocation, re-allocation and freeing of memory
         *  with advanced controls to improve memory efficiency.
         *  In this context, a free request is represented as re-allocation with requested_size == 0
         *
         *  Arguments
         *    current_ptr:
         *      Pointer to current memory location:
         *        ==NULL for pure-allocation
         *        !=NULL for re-allocation or freeing
         *
         *    current_size:
         *      Optional information to memory manager about last reserved or requested amount.
         *      Allowed values: 0 or previously requested or reserved amount.
         *                      0 makes the function ignore current_ptr (assumes it is NULL)
         *
         *    requested_size:
         *       > 0 for pure-allocation or re-allocation
         *      == 0 for freeing
         *
         *    granted_size
         *      Memory granted to requester.
         *      The memory manager grants at least the requested amount of bytes. But it may grant more memory.
         *      The requester may use granted memory without re-allocation. (E.g. for dynamic arrays.)
         *      Retrieving the granted amount is optional. Use NULL when not desired.
         *
         *  Return
         *    Allocated memory address. NULL in case all memory was freed.
         *
         *  Alignment: (default behavior)
         *    A request of size of n*m bytes, where n,m are positive integers and m is (largest possible)
         *    integer power of 2, returns an address aligned to the lesser of m and TBMAN_ALIGN
         *    (TBMAN_ALIGN is defined in Core.c).
         *    This provides correct alignment of standard data types but also for composite types
         *    (e.g. int32x4_t) for use with a SIMD extension of the CPU (e.g. Intel's SSE or ARM's Neon).
         *
         */
        void* memAlloc(void* currentPtr, size_t requestedSize, size_t* grantedSize);
        
        /// malloc, free and realloc (thread-safe).
        void* memMalloc(size_t size);
        void* memRealloc(void* ptr, size_t size);
        void memFree(void* ptr);

        /// realloc, specifying current size (thread-safe).
        void* memNrealloc(void* currentPtr, size_t currentSize, size_t newSize);

        /// free, specifying current size (thread-safe).
        void memNfree(void* currentPtr, size_t currentSize);

        void* memAlloc(size_t requestedSize, size_t* grantedSize);
        void memFree(void* currentPtr, const size_t* currentSize);
        void* memRealloc(void* currentPtr, const size_t* currentSize, size_t requestedSize, size_t* grantedSize);

        /// Returns currently granted space for a specified memory instance (thread-safe)
        size_t grantedSpace(const void* currentPtr);

        /// Returns total of currently granted space (thread-safe)
        size_t totalGrantedSpace();

        /// Returns number of open allocation instances (thread-safe)
        size_t totalInstances();

        /** Iterates through all open instances and calls 'callback' per instance (thread-safe)
         *  The callback function may change the manager's state.
         *  Only instances which where open at the moment of entering 'bcore_tbman_s_for_each_instance' are iterated.
         *  While 'bcore_tbman_s_for_each_instance' executes, any instance closed or newly opened will
         *  not change the iteration.
         */
        void forEachInstance(void_callback1 cb, void* arg);

        /// prints internal status to stdout (use only for debugging/testing - not thread-safe)
        void printStatus(int detail_level);

        void lostAlignment();

        size_t externalTotalAlloc();
        size_t externalTotalInstances();

        void externalForEachInstance(void_callback1 cb, void* arg);

        size_t internalTotalAlloc();
        size_t internalTotalInstances();

        void internalForEachInstance(void_callback1 cb, void* arg);

        size_t totalAlloc();
        size_t totalSpace();

        /** The following is the test and evaluation program for a memory manager.
         *  This application realistically simulates memory-intensive usage of
         *  memory management, monitoring data integrity and processing speed.
         *  It compares Core with stdlib's memory manager.
         */

        /** Rigorous Monte Carlo based Memory Manager Test.
         *
         *  This routine evaluates the integrity and speed of a chosen memory
         *  management (MM) system by randomly allocating, reallocating and
         *  freeing memory within a contingent of memory instances using randomized
         *  size distribution.
         *
         *  For speed measurements we simulate realistic conditions by choosing a
         *  Zipfian distribution of instance-size and by keeping the total memory at
         *  equilibrium.
         *  Thus, malloc & free need to be tested in combination but realloc can be
         *  tested in isolation.
         *
         *  In tests labeled 'general' a vast amount of arbitrary instances are
         *  processed. In tests labeled 'local', few instances get repeatedly allocated
         *  and free-d. In local tests we can expect MM's relevant metadata
         *  to remain in cache. Thus, the local test better reflect MM's algorithmic
         *  overhead and is most representative for routines with high computational
         *  but little memory complexity.
         *
         *  Time values are given in approximate averaged nanoseconds (ns) needed
         *  executing a single call (malloc, free or realloc).
         *
         *  Note that apart form the MM's algorithms used, speed measurements are also
         *  highly sensitive to platform specifications such as
         *    - CPU speed
         *    - Type and speed of memory
         *    - Type and amount of cache
         *    - The distribution of free system memory
         *      (--> makes results fluctuate)
         *
         *  Hence, we recommend to run tests repeatedly and to consider testing
         *  different platforms in order to obtain an adequate picture of the MM's
         *  ability.
         */
        static void allocChallenge(
            fp_alloc alloc,
            size_t table_size,
            size_t cycles,
            size_t max_alloc,
            uint32_t seed,
            bool cleanup,
            bool verbose);

        // internal alloc without passing current_bytes
        static void* nallocNoCurrentBytes(void* currentPtr, size_t currentBytes, size_t requestedBytes, size_t* grantedBytes);

        static void diagnosticTest();
        static void test();

        //static void* memSAlloc(void* currentPtr, size_t requestedSize, size_t* grantedSize);
        static void* memNalloc(void* currentPtr, size_t currentSize, size_t requestedSize, size_t* grantedSize);
        static void diagnosticTestCallback(void* arg, void* ptr, size_t space);

        static void eval();

    public:

        mtx_t getMutex() { return mMutex; }

    private:

        // block managers are sorted by increasing block size
        TbBlockManager** mData;

        size_t mSize;

        // pool size for all token managers
        size_t mPoolSize;

        size_t mMinBlockSize;
        size_t mMaxBlockSize;

        // all token managers are aligned
        bool mAligned;

        // copy of block size values (for fast access)
        size_t* mBlockSizeArray;

        TbTreeIntVD* mInternalBTree;
        TbTreeExtPS* mExternalBTree;

        mtx_t mMutex;

        // size of a memory pool in a token manager
        static const size_t mDefaultPoolSize = 0x10000;
        
        // minimal block size
        static const size_t mDefaultMinBlockSize = 8;

        // maximal block size
        static const size_t mDefaultMaxBlockSize = 1024 * 16;

        // 1: uses power-2 block size stepping; > 1 uses more fine grained stepping
        static const size_t mDefaultSteppingMethod = 1;

        // true: uses full memory alignment (fastest)
        static const bool mDefaultFullAlign = true;

        ext_for_instance_arg mIArg;
        mnode mMNode;
        mnode_arr mMNodeArr;
        diagnostic mDiag;

        static TbMemMan* gDefaultTbman;
    };
}

//================================================================================================================
//================================================================================================================
#endif//__TBMAN_H