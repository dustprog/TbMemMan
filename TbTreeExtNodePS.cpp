#include "TbTreeExtNodePS.h"

//extern Core::TbTreeExtNodePS Core::btree_node_ps_s_null_g;
//Core::TbTreeExtNodePS* Core::btree_node_ps_s_null_g = NULL;
Core::TbTreeExtNodePS Core::TbTreeExtNodePS::btree_node_ps_s_null_g;

//================================================================================================================
//================================================================================================================

Core::TbTreeExtNodePS::TbTreeExtNodePS()
{
    // This original code did a memset
    /*Core::TbManUtility::reset(mKeyT1);
    Core::TbManUtility::reset(mValT1);
    Core::TbManUtility::reset(mKeyT2);
    Core::TbManUtility::reset(mValT2);
    Core::TbManUtility::reset(mParent);
    Core::TbManUtility::reset(mChild0);
    Core::TbManUtility::reset(mChild1);
    Core::TbManUtility::reset(mChild2);*/

    mKeyT1 = NULL;
    mValT1 = 0;
    mKeyT2 = NULL;
    mValT2 = 0;
    mParent = NULL;
    mChild0 = NULL;
    mChild1 = NULL;
    mChild2 = NULL;
}

//================================================================================================================
//================================================================================================================

Core::TbTreeExtNodePS::TbTreeExtNodePS(
    void* keyT1, size_t valT1,
    void* keyT2, size_t valT2,
    TbTreeExtNodePS* parent,
    TbTreeExtNodePS* child0,
    TbTreeExtNodePS* child1,
    TbTreeExtNodePS* child2)
:   mKeyT1(keyT1), mValT1(valT1),
    mKeyT2(keyT2), mValT2(valT2),
    mParent(parent),
    mChild0(child0), mChild1(child1), mChild2(child2)
{
}

//================================================================================================================
//================================================================================================================

Core::TbTreeExtNodePS::~TbTreeExtNodePS()
{
}

//================================================================================================================
//================================================================================================================

void Core::TbTreeExtNodePS::init()
{
    mKeyT1 = 0;
    mValT1 = 0;

    mKeyT2 = 0;
    mValT2 = 0;

    mParent = mChild0 = mChild1 = mChild2 = BNUL_PS;
}

//================================================================================================================
//================================================================================================================

Core::TbTreeExtNodePS* Core::TbTreeExtNodePS::find(/*TbTreeExtNodePS* root, */void* key)
{
    /*if (!root) return NULL;
    
    TbTreeExtNodePS* node = NULL;
    
    while (root != NULL && root->getChild0() != BNUL_PS && root != node)
    {
        node = root;
        root = (key < node->getKeyT1()) ? node->getChild0() :
            (!node->getChild2() && key > node->getKeyT1()) ? node->getChild1() :
            (node->getChild2() && key > node->getKeyT2()) ? node->getChild2() :
            (node->getChild2() && key > node->getKeyT1() && key < node->getKeyT2()) ? node->getChild1() : node;
    }
    
    return root;*/

    TbTreeExtNodePS* root = this;

    if (!root) return NULL;

    TbTreeExtNodePS* node = NULL;

    while (mChild0 != BNUL_PS && root != node)
    {
        node = root;
        root = (key < node->getKeyT1()) ? node->getChild0() :
            (!node->getChild2() && key > node->getKeyT1()) ? node->getChild1() :
            (node->getChild2() && key > node->getKeyT2()) ? node->getChild2() :
            (node->getChild2() && key > node->getKeyT1() && key < node->getKeyT2()) ? node->getChild1() : node;
    }

    return root;
}

//================================================================================================================
//================================================================================================================

void Core::TbTreeExtNodePS::run(/*TbTreeExtNodePS* root, */void_callback1 func, void* arg)
{
    //if (!root) return;
    if (!func) return;

    if (mChild0)
    {
        if (mChild0 != BNUL_PS)
        {
            mChild0->run(func, arg);
        }
    }
    
    if (mChild1)
    {
        func(arg, mKeyT1, mValT1);

        if (mChild1 != BNUL_PS)
        {
            mChild1->run(func, arg);
        }
    }
    
    if (mChild2)
    {
        func(arg, mKeyT2, mValT2);

        if (mChild2 != BNUL_PS)
        {
            mChild2->run(func, arg);
        }
    }
}

//================================================================================================================
//================================================================================================================

size_t Core::TbTreeExtNodePS::count(/*TbTreeExtNodePS* root, */bool_callback1 func, void* arg)
{
    size_t count = 0;

    /*if (!root)
    {
        return count;
    }*/

    if (mChild0)
    {
        if (mChild0 != BNUL_PS)
        {
            count += mChild0->count(func, arg);
        }
    }

    if (mChild1)
    {
        count += (func) ? func(arg, mKeyT1, mValT1) : 1;

        if (mChild1 != BNUL_PS)
        {
            count += mChild1->count(func, arg);
        }
    }

    if (mChild2)
    {
        count += (func) ? func(arg, mKeyT2, mValT2) : 1;

        if (mChild2 != BNUL_PS)
        {
            count += mChild2->count(func, arg);
        }
    }

    return count;
}

//================================================================================================================
//================================================================================================================

size_t Core::TbTreeExtNodePS::sum(/*TbTreeExtNodePS* root, */bool_callback1 func, void* arg)
{
    size_t sum = 0;

    /*if (!root)
    {
        return sum;
    }*/

    if (mChild0)
    {
        if (mChild0 != BNUL_PS)
        {
            sum += mChild0->sum(func, arg);
        }
    }

    if (mChild1)
    {
        sum += mValT1 * ((func) ? func(arg, mKeyT1, mValT1) : 1);

        if (mChild1 != BNUL_PS)
        {
            sum += mChild1->sum(func, arg);
        }
    }

    if (mChild2)
    {
        sum += mValT2 * ((func) ? func(arg, mKeyT2, mValT2) : 1);

        if (mChild2 != BNUL_PS)
        {
            sum += mChild2->sum(func, arg);
        }
    }

    return sum;
}

//================================================================================================================
//================================================================================================================

size_t Core::TbTreeExtNodePS::keys()//TbTreeExtNodePS* root)
{
    //if (!root || root == BNUL_PS) return 0;
    if (this == BNUL_PS) return 0;

    size_t keys = mChild2 ? 2 : 1;
    keys += mChild0->keys();
    keys += mChild1->keys();
    
    if (mChild2)
    {
        keys += mChild2->keys();
    }
    
    return keys;
}

//================================================================================================================
//================================================================================================================

size_t Core::TbTreeExtNodePS::depth()//TbTreeExtNodePS* root)
{
    if (!mChild0 || mChild0 == BNUL_PS)
    {
        return 0;
    }

    return 1 + mChild0->depth();
}

//================================================================================================================
//================================================================================================================

void Core::TbTreeExtNodePS::setParentChild0()//TbTreeExtNodePS* root)
{
    if (mChild0 && mChild0 != BNUL_PS)
    {
        mChild0->setParent(this);
    }
}

//================================================================================================================
//================================================================================================================

void Core::TbTreeExtNodePS::setParentChild1()//TbTreeExtNodePS* root)
{
    if (mChild1 && mChild1 != BNUL_PS)
    {
        mChild1->setParent(this);
    }
}

//================================================================================================================
//================================================================================================================

void Core::TbTreeExtNodePS::setParentChild2()//TbTreeExtNodePS* root)
{
    if (mChild2 && mChild2 != BNUL_PS)
    {
        mChild2->setParent(this);
    }
}

//================================================================================================================
//================================================================================================================

int Core::TbTreeExtNodePS::isLeaf()//TbTreeExtNodePS* root)
{
    return mChild0 == BNUL_PS;
}

//================================================================================================================
//================================================================================================================

int Core::TbTreeExtNodePS::isFull()//TbTreeExtNodePS* root)
{
    return mChild2 != NULL;
}

//================================================================================================================
//================================================================================================================

int Core::TbTreeExtNodePS::isEmpty()//TbTreeExtNodePS* root)
{
    return mChild1 == NULL;
}

//================================================================================================================
//================================================================================================================

void Core::TbTreeExtNodePS::checkConsistency()//TbTreeExtNodePS* root)
{
    if (btree_node_ps_s_null_g.getKeyT1() != 0) ERR("btree_node_ps_s_null was modified");
    if (btree_node_ps_s_null_g.getValT1() != 0) ERR("btree_node_ps_s_null was modified");
    if (btree_node_ps_s_null_g.getKeyT2() != 0) ERR("btree_node_ps_s_null was modified");
    if (btree_node_ps_s_null_g.getValT2() != 0) ERR("btree_node_ps_s_null was modified");
    if (btree_node_ps_s_null_g.getParent() != NULL) ERR("btree_node_ps_s_null was modified");
    if (btree_node_ps_s_null_g.getChild0() != NULL) ERR("btree_node_ps_s_null was modified");
    if (btree_node_ps_s_null_g.getChild1() != NULL) ERR("btree_node_ps_s_null was modified");
    if (btree_node_ps_s_null_g.getChild2() != NULL) ERR("btree_node_ps_s_null was modified");

    //if (!o) return;
    
    if (isEmpty())
    {
        ERR("empty node");
    }
    
    if (mChild0 == NULL)
    {
        ERR("deleted leaf");
    }
    
    if (mChild1 && mChild1 != BNUL_PS)
    {
        if (this != mChild0->getParent()) ERR("child0 incorrect parent");
        if (this != mChild1->getParent()) ERR("child1 incorrect parent");

        mChild0->checkConsistency();
        mChild1->checkConsistency();
        
        if (mKeyT1 <= mChild0->getKeyT1())
        {
            ERR("(%lu <= %lu)", mKeyT1, mChild0->getKeyT1());
        }

        if (mChild0->getChild2() && mKeyT1 <= mChild0->getKeyT2())
        {
            ERR("(%lu <= %lu)", mKeyT1, mChild0->getKeyT2());
        }

        if (mKeyT1 >= mChild1->getKeyT1())
        {
            ERR("(%lu >= %lu)", mKeyT1, mChild1->getKeyT1());
        }

        if (mChild1->getChild2() && mKeyT1 >= mChild1->getKeyT2())
        {
            ERR("(%lu >= %lu)", mKeyT1, mChild1->getKeyT2());
        }
    }
    
    if (mChild2 && mChild2 != BNUL_PS)
    {
        if (mKeyT1 >= mKeyT2)
        {
            ERR("(%lu >= %lu)", mKeyT1, mKeyT2);
        }

        if (this != mChild2->getParent())
        {
            ERR("child2 incorrect parent");
        }
        
        mChild2->checkConsistency();
        
        if (mKeyT2 <= mChild1->getKeyT1())
        {
            ERR("(%lu <= %lu)", mKeyT2, mChild1->getKeyT1());
        }

        if (mChild1->getChild2() && mKeyT2 <= mChild1->getKeyT2())
        {
            ERR("(%lu <= %lu)", mKeyT2, mChild1->getKeyT2());
        }
        
        if (mKeyT2 >= mChild2->getKeyT1())
        {
            ERR("(%lu >= %lu)", mKeyT2, mChild2->getKeyT1());
        }
        
        if (mChild2->getChild2() && mKeyT2 >= mChild2->getKeyT2())
        {
            ERR("(%lu >= %lu)", mKeyT2, mChild2->getKeyT2());
        }
    }
}

//================================================================================================================
//================================================================================================================