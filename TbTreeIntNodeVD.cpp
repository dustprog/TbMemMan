#include "TbTreeIntNodeVD.h"

//extern Core::TbTreeIntNodeVD Core::btree_node_vd_s_null_g;
//Core::TbTreeIntNodeVD* Core::btree_node_vd_s_null_g = NULL;
Core::TbTreeIntNodeVD Core::TbTreeIntNodeVD::btree_node_vd_s_null_g;

//================================================================================================================
//================================================================================================================

Core::TbTreeIntNodeVD::TbTreeIntNodeVD()
{
    // This original code did a memset
    /*Core::TbManUtility::reset(mKeyT1);
    Core::TbManUtility::reset(mKeyT2);
    Core::TbManUtility::reset(mParent);
    Core::TbManUtility::reset(mChild0);
    Core::TbManUtility::reset(mChild1);
    Core::TbManUtility::reset(mChild2);*/

    mKeyT1 = NULL;
    mKeyT2 = NULL;
    mParent = NULL;
    mChild0 = NULL;
    mChild1 = NULL;
    mChild2 = NULL;
}

//================================================================================================================
//================================================================================================================

Core::TbTreeIntNodeVD::TbTreeIntNodeVD(
    void* keyT1,
    void* keyT2,
    TbTreeIntNodeVD* parent,
    TbTreeIntNodeVD* child0,
    TbTreeIntNodeVD* child1,
    TbTreeIntNodeVD* child2)
:   mKeyT1(keyT1),
    mKeyT2(keyT2),
    mParent(parent),
    mChild0(child0), mChild1(child1), mChild2(child2)
{
}

//================================================================================================================
//================================================================================================================

Core::TbTreeIntNodeVD::~TbTreeIntNodeVD()
{
}

//================================================================================================================
//================================================================================================================

void Core::TbTreeIntNodeVD::init()
{
    mKeyT1 = 0;
    mKeyT2 = 0;
    
    mParent = mChild0 = mChild1 = mChild2 = BNUL_VP;
}

//================================================================================================================
//================================================================================================================

Core::TbTreeIntNodeVD* Core::TbTreeIntNodeVD::find(void* key)
{
    TbTreeIntNodeVD* root = this;

    if (!root) return NULL;

    TbTreeIntNodeVD* node = NULL;

    while (mChild0 != BNUL_VP && root != node)
    {
        node = root;
        root = ( key < node->getKeyT1())                                                ? node->getChild0() :
               (!node->getChild2() && key > node->getKeyT1())                           ? node->getChild1() :
               ( node->getChild2() && key > node->getKeyT2())                           ? node->getChild2() :
               ( node->getChild2() && key > node->getKeyT1() && key < node->getKeyT2()) ? node->getChild1() : node;
    }

    return root;
}

//================================================================================================================
//================================================================================================================

void* Core::TbTreeIntNodeVD::largestBelowEqual(void* key)
{
    //if (!root) return 0;

    if (mChild0 == BNUL_VP)
    {
        if ((mChild2) && key >= mKeyT2) return mKeyT2;
        return (key >= mKeyT1) ? mKeyT1 : 0;
    }
    else if ((mChild2) && key >= mKeyT2)
    {
        void* branch_key = mChild2->largestBelowEqual(key);
        return (branch_key >= mKeyT2) ? branch_key : mKeyT2;
    }
    else if (key >= mKeyT1)
    {
        void* branch_key = mChild1->largestBelowEqual(key);
        return (branch_key >= mKeyT1) ? branch_key : mKeyT1;
    }
    else
    {
        return mChild0->largestBelowEqual(key);
    }
}

//================================================================================================================
//================================================================================================================

void Core::TbTreeIntNodeVD::run(void_callback2 func, void* arg)
{
    if (!func) return;

    if (mChild0)
    {
        if (mChild0 != BNUL_VP)
        {
            mChild0->run(func, arg);
        }
    }

    if (mChild1)
    {
        func(arg, mKeyT1);

        if (mChild1 != BNUL_VP)
        {
            mChild1->run(func, arg);
        }
    }

    if (mChild2)
    {
        func(arg, mKeyT2);

        if (mChild2 != BNUL_VP)
        {
            mChild2->run(func, arg);
        }
    }
}

//================================================================================================================
//================================================================================================================

size_t Core::TbTreeIntNodeVD::count(bool_callback2 func, void* arg)
{
    size_t count = 0;

    if (mChild0)
    {
        if (mChild0 != BNUL_VP)
        {
            count += mChild0->count(func, arg);
        }
    }

    if (mChild1)
    {
        count += (func) ? func(arg, mKeyT1) : 1;

        if (mChild1 != BNUL_VP)
        {
            count += mChild1->count(func, arg);
        }
    }

    if (mChild2)
    {
        count += (func) ? func(arg, mKeyT2) : 1;

        if (mChild2 != BNUL_VP)
        {
            count += mChild2->count(func, arg);
        }
    }

    return count;
}

//================================================================================================================
//================================================================================================================

size_t Core::TbTreeIntNodeVD::keys()//TbTreeIntNodeVD* root)
{
    if (this == BNUL_VP) return 0;

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

size_t Core::TbTreeIntNodeVD::depth()//TbTreeIntNodeVD* root)
{
    if (!mChild0 || mChild0 == BNUL_VP)
    {
        return 0;
    }

    return 1 + mChild0->depth();
}

//================================================================================================================
//================================================================================================================

void Core::TbTreeIntNodeVD::setParentChild0()
{
    if (mChild0 && mChild0 != BNUL_VP)
    {
        mChild0->setParent(this);
    }
}

//================================================================================================================
//================================================================================================================

void Core::TbTreeIntNodeVD::setParentChild1()
{
    if (mChild1 && mChild1 != BNUL_VP)
    {
        mChild1->setParent(this);
    }
}

//================================================================================================================
//================================================================================================================

void Core::TbTreeIntNodeVD::setParentChild2()
{
    if (mChild2 && mChild2 != BNUL_VP)
    {
        mChild2->setParent(this);
    }
}

//================================================================================================================
//================================================================================================================

int Core::TbTreeIntNodeVD::isLeaf()
{
    return mChild0 == BNUL_VP;
}

//================================================================================================================
//================================================================================================================

int Core::TbTreeIntNodeVD::isFull()
{
    return mChild2 != NULL;
}

//================================================================================================================
//================================================================================================================

int Core::TbTreeIntNodeVD::isEmpty()
{
    return mChild1 == NULL;
}

//================================================================================================================
//================================================================================================================

void Core::TbTreeIntNodeVD::checkConsistency()
{
    if (btree_node_vd_s_null_g.getKeyT1() != 0) ERR("btree_node_ps_s_null was modified");
    if (btree_node_vd_s_null_g.getKeyT2() != 0) ERR("btree_node_ps_s_null was modified");
    if (btree_node_vd_s_null_g.getParent() != NULL) ERR("btree_node_ps_s_null was modified");
    if (btree_node_vd_s_null_g.getChild0() != NULL) ERR("btree_node_ps_s_null was modified");
    if (btree_node_vd_s_null_g.getChild1() != NULL) ERR("btree_node_ps_s_null was modified");
    if (btree_node_vd_s_null_g.getChild2() != NULL) ERR("btree_node_ps_s_null was modified");

    //if (!o) return;

    if (isEmpty())
    {
        ERR("empty node");
    }

    if (mChild0 == NULL)
    {
        ERR("deleted leaf");
    }

    if (mChild1 && mChild1 != BNUL_VP)
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

    if (mChild2 && mChild2 != BNUL_VP)
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