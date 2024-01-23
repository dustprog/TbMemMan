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
// TbTreeExtNodePS.h
//
// The external node for the self-balancing 2-3 tree
//================================================================================================================
// History
//
// Created on 12/22/2023 by Dustin Watson
//================================================================================================================
#ifndef __TBTREE_EXT_NODE_PS_H
#define __TBTREE_EXT_NODE_PS_H
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
     *  BNUL_PS: node is a leaf
     *  else: node has children
     */
    class TbTreeExtNodePS
    {
    public:
        TbTreeExtNodePS();
        TbTreeExtNodePS(
            void* keyT1, size_t valT1,
            void* keyT2, size_t valT2,
            TbTreeExtNodePS* parent,
            TbTreeExtNodePS* child0,
            TbTreeExtNodePS* child1,
            TbTreeExtNodePS* child2);
        ~TbTreeExtNodePS();

        void init();

        TbTreeExtNodePS* find(void* key);

        void run(void_callback1 func, void* arg);
        size_t count(bool_callback1 func, void* arg);
        size_t sum(bool_callback1 func, void* arg);

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

        void setValT1(size_t value) { mValT1 = value; }
        size_t getValT1() { return mValT1; }
        size_t* getValT1Ptr() { return &mValT1; }

        void setKeyT2(void* value) { mKeyT2 = value; }
        void* getKeyT2() { return mKeyT2; }

        void setValT2(size_t value) { mValT2 = value; }
        size_t getValT2() { return mValT2; }
        size_t* getValT2Ptr() { return &mValT2; }

        void setParent(TbTreeExtNodePS* value) { mParent = value; }
        TbTreeExtNodePS* getParent() { return mParent; }

        void setChild0(TbTreeExtNodePS* value) { mChild0 = value; }
        TbTreeExtNodePS* getChild0() { return mChild0; }

        void setChild1(TbTreeExtNodePS* value) { mChild1 = value; }
        TbTreeExtNodePS* getChild1() { return mChild1; }

        void setChild2(TbTreeExtNodePS* value) { mChild2 = value; }
        TbTreeExtNodePS* getChild2() { return mChild2; }

        static TbTreeExtNodePS btree_node_ps_s_null_g;

    private:

        void* mKeyT1;
        size_t mValT1;
        
        void* mKeyT2;
        size_t mValT2;

        TbTreeExtNodePS* mParent;
        TbTreeExtNodePS* mChild0;
        TbTreeExtNodePS* mChild1;
        TbTreeExtNodePS* mChild2;
    };

    /// children of leaf-nodes point to btree_node_ps_s_null
    //static inline TbTreeExtNodePS btree_node_ps_s_null_g(0, 0, 0, 0, NULL, NULL, NULL, NULL);
    //static TbTreeExtNodePS* btree_node_ps_s_null_g;
    #define BNUL_PS (&TbTreeExtNodePS::btree_node_ps_s_null_g)
}
//================================================================================================================
//================================================================================================================
#endif//__TBTREE_EXT_NODE_PS_H