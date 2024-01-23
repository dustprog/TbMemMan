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
// TbTreeIntNodeVD.h
//
// The internal node for the self-balancing 2-3 tree
//================================================================================================================
// History
//
// Created on 12/22/2023 by Dustin Watson
//================================================================================================================
#ifndef __TBTREE_INT_NODE_VD_H
#define __TBTREE_INT_NODE_VD_H
//================================================================================================================
//================================================================================================================

//
// Includes
//

#include "TbManUtility.h"
#include "Callback.h"

//================================================================================================================
//================================================================================================================
namespace Core {
    /** Node of a 2-3 btree.
     *  Child pointers can have one of three states:
     *  NULL: corresponding key-value pair is not used (for normal nodes this state only applies to child2)
     *  BNUL_VP: node is a leaf
     *  else: node has children
     */
    class TbTreeIntNodeVD
    {
    public:
        TbTreeIntNodeVD();
        TbTreeIntNodeVD(
            void* keyT1,
            void* keyT2,
            TbTreeIntNodeVD* parent,
            TbTreeIntNodeVD* child0,
            TbTreeIntNodeVD* child1,
            TbTreeIntNodeVD* child2);
        ~TbTreeIntNodeVD();
        
        void init();

        TbTreeIntNodeVD* find(void* key);

        /// Returns the largest stored key euqal or below <key>. Returns NULL in case all stored keys are larger than <key>.
        void* largestBelowEqual(void* key);

        void run(void_callback2 func, void* arg);
        size_t count(bool_callback2 func, void* arg);

        size_t keys();
        size_t depth();

        void setParentChild0();
        void setParentChild1();
        void setParentChild2();

        int isLeaf();
        int isFull();
        int isEmpty();

        void checkConsistency();

    public:

        void setKeyT1(void* value) { mKeyT1 = value; }
        void* getKeyT1() { return mKeyT1; }

        void setKeyT2(void* value) { mKeyT2 = value; }
        void* getKeyT2() { return mKeyT2; }

        void setParent(TbTreeIntNodeVD* value) { mParent = value; }
        TbTreeIntNodeVD* getParent() { return mParent; }

        void setChild0(TbTreeIntNodeVD* value) { mChild0 = value; }
        TbTreeIntNodeVD* getChild0() { return mChild0; }

        void setChild1(TbTreeIntNodeVD* value) { mChild1 = value; }
        TbTreeIntNodeVD* getChild1() { return mChild1; }

        void setChild2(TbTreeIntNodeVD* value) { mChild2 = value; }
        TbTreeIntNodeVD* getChild2() { return mChild2; }

        static TbTreeIntNodeVD btree_node_vd_s_null_g;

    private:
        
        void* mKeyT1;
        void* mKeyT2;

        TbTreeIntNodeVD* mParent;
        TbTreeIntNodeVD* mChild0;
        TbTreeIntNodeVD* mChild1;
        TbTreeIntNodeVD* mChild2;
    };

    /// children of leaf-nodes point to btree_node_vd_s_null
    //TbTreeIntNodeVD* btree_node_vd_s_null_g(NULL, NULL, NULL, NULL, NULL, NULL);
    //static TbTreeIntNodeVD* btree_node_vd_s_null_g;
    #define BNUL_VP (&TbTreeIntNodeVD::btree_node_vd_s_null_g)
}

//================================================================================================================
//================================================================================================================
#endif//__TBTREE_INT_NODE_VD_H