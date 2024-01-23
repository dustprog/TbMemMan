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
// TbTreeIntVD.h
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
#ifndef __TBTREE_INT_VD_H
#define __TBTREE_INT_VD_H
//================================================================================================================
//================================================================================================================

//
// Includes
//

#include "TbManUtility.h"
#include "TbTreeIntNodeVD.h"
#include "Callback.h"

//================================================================================================================
//================================================================================================================
namespace Core {
    // tree of void* as key (no dedicated value)
    class TbTreeIntVD
    {
    public:
        
        void init();

        /// Creates a new btree_vd (allows to specify alloc function because this tree type is used in memory management)
        static Core::TbTreeIntVD* create(alloc_callback alloc);

        /// Deletes a btree_vd
        void discard();

        TbTreeIntNodeVD* newNode();

        // Deleted nodes are marked by setting all children NULL
        // and chained together using pointer btree_node_vd_s.parent.
        void deleteNode(TbTreeIntNodeVD* node);

        // recursively pushes an element into the tree
        void push(TbTreeIntNodeVD* node, void* key, TbTreeIntNodeVD* child0, TbTreeIntNodeVD* child1);

        // Recursively pulls an element from a non-leaf into an empty child node
        void pull(TbTreeIntNodeVD* node);

        /// Checks existence of key
        bool exists(void* key);

        /// Returns the largest stored key euqal or below <key>. Returns NULL in case all stored keys are larger than <key>.
        void* largestBelowEqual(void* key);
        
        /** Sets a key in the tree.
         *  Return value:
         *    0: key already existed -> nothing changed
         *    1: key did not exist and was created
         *   -2: internal error
         */
        int set(void* key);

        /** Removes a key from the tree.
         *  Return value:
         *    0: key did not exist -> nothing changed
         *    1: key found and removed
         *   -1: internal error
         */
        int remove(void* key);

        /// calls a function for all tree elements
        void run(void_callback2 func, void* arg);

        /// counts entries for which func returns true; counts all entries in case func is NULL
        size_t count(bool_callback2 func, void* arg);

        /// return depth of tree
        size_t depth();

        void printStatus();

    public:

        void setRoot(TbTreeIntNodeVD* value) { mRoot = value; }

        void setChainBeg(TbTreeIntNodeVD* value) { mChainBeg = value; }

        void setChainEnd(TbTreeIntNodeVD* value) { mChainEnd = value; }

        void setChainIns(TbTreeIntNodeVD* value) { mChainIns = value; }

        void setDelChain(TbTreeIntNodeVD* value) { mDelChain = value; }

        void setAlloc(alloc_callback value) { mAlloc = value; }

        void setBlockSize(size_t value) { mBlockSize = value; }

    private:

        TbTreeIntNodeVD* mRoot;

        // begin of chain of blocks of btree_node_vd_s[] with last element being pointer to next block
        TbTreeIntNodeVD* mChainBeg;

        // end of chain of blocks
        TbTreeIntNodeVD* mChainEnd;

        // pointer for new insertions
        TbTreeIntNodeVD* mChainIns;

        // chain of deleted elements (preferably used by new insertions)
        TbTreeIntNodeVD* mDelChain;

        alloc_callback mAlloc;

        size_t mBlockSize;
    };
}
//================================================================================================================
//================================================================================================================
#endif//__TBTREE_INT_VD_H