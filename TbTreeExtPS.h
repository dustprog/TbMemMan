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
// TbTreeExtPS.h
//
// btree on the basis of a self-balancing 2-3 tree structure.
//  This structure can be used for well-scalable associative data management.
//  Worst case complexity is O(log(n)) for access, insertion and deletion.
//
//  Internally, this solution uses a node structure with three child pointers,
//  one parent pointer and two key-value containers.
//  Root-state is indicated by parent==NULL.
//  Leaf-state is indicated by child0==BNUL.
//  Single-key-state is indicated by child2==NULL.
//  Therefore additional state-flags are not needed.
//
//  Available btree structures
//    - btree_ps: key: void*,    value: size_t
//    - btree_vd: key: void*,    value: no dedicated value
//
//================================================================================================================
// History
//
// Created on 12/22/2023 by Dustin Watson
//================================================================================================================
#ifndef __TBTREE_EXT_PS_H
#define __TBTREE_EXT_PS_H
//================================================================================================================
//================================================================================================================

//
// Includes
//

#include "TbManUtility.h"
#include "TbTreeExtNodePS.h"
#include "Callback.h"

//================================================================================================================
//================================================================================================================
namespace Core {
    // tree of void* as key and size_t as value
    class TbTreeExtPS
    {
    public:
        
        void init();

        /// Creates a new btree_ip
        static TbTreeExtPS* create(alloc_callback alloc);

        /// Deletes a btree_ip
        void discard();

        TbTreeExtNodePS* newNode();

        // Deleted nodes are marked by setting all children NULL
        // and chained together using pointer btree_node_ps_s.parent.
        void deleteNode(TbTreeExtNodePS* node);

        // recursively pushes an element into the tree
        void push(TbTreeExtNodePS* node, void* key, size_t val, TbTreeExtNodePS* child0, TbTreeExtNodePS* child1);

        // Recursively pulls an element from a non-leaf into an empty child node
        void pull(TbTreeExtNodePS* node);

        /** Returns pointer to the value associated with given key.
         *  Returns NULL when the key does not exist.
         */
        size_t* val(void* key);

        /** Sets a key-value pair in the tree.
         *  If the key already exists, its value is overwritten.
         *  Return value:
         *    0: key, val already existed -> nothing changed
         *    1: key did not exist (key, val) was created
         *   -1: key already existed but with different value -> value was overwritten
         *   -2: internal error
         */
        int set(void* key, size_t val);

        /** Removes a key from the tree.
         *  Return value:
         *    0: key did not exist -> nothing changed
         *    1: key found and removed
         *   -1: internal error
         */
        int remove(void* key);

        /// calls a function for all tree elements
        void run(void_callback1 func, void* arg);

        /// counts entries for which func returns true; counts all entries in case func is NULL
        size_t count(bool_callback1 func, void* arg);
        
        /// sums entries for which func returns true; sums all entries in case func is NULL
        size_t sum(bool_callback1 func, void* arg);

        /// return depth of tree
        size_t depth();

        void printStatus();

    public:

        void setRoot(TbTreeExtNodePS* value) { mRoot = value; }

        void setChainBeg(TbTreeExtNodePS* value) { mChainBeg = value; }

        void setChainEnd(TbTreeExtNodePS* value) { mChainEnd = value; }

        void setChainIns(TbTreeExtNodePS* value) { mChainIns = value; }

        void setDelChain(TbTreeExtNodePS* value) { mDelChain = value; }

        void setAlloc(alloc_callback value) { mAlloc = value; }

        void setBlockSize(size_t value) { mBlockSize = value; }

    private:

        TbTreeExtNodePS* mRoot;

        // begin of chain of blocks of btree_node_ps_s[] with last element being pointer to next block
        TbTreeExtNodePS* mChainBeg;

        // end of chain of blocks
        TbTreeExtNodePS* mChainEnd;

        // pointer for new insertions
        TbTreeExtNodePS* mChainIns;

        // chain of deleted elements (preferably used by new insertions)
        TbTreeExtNodePS* mDelChain;

        alloc_callback mAlloc;

        size_t mBlockSize;
    };
}
//================================================================================================================
//================================================================================================================
#endif//__TBTREE_EXT_PS_H