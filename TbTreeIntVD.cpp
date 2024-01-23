#include "TbTreeIntVD.h"

//================================================================================================================
//================================================================================================================

void Core::TbTreeIntVD::init()
{
    /*Core::TbManUtility::reset(mRoot);
    Core::TbManUtility::reset(mChainBeg);
    Core::TbManUtility::reset(mChainEnd);
    Core::TbManUtility::reset(mChainIns);
    Core::TbManUtility::reset(mDelChain);
    Core::TbManUtility::reset(mAlloc);
    Core::TbManUtility::reset(mBlockSize);*/

    mRoot = NULL;
    mChainBeg = NULL;
    mChainEnd = NULL;
    mChainIns = NULL;
    mDelChain = NULL;
    mBlockSize = 0;
}

//================================================================================================================
//================================================================================================================

Core::TbTreeIntVD* Core::TbTreeIntVD::create(alloc_callback alloc)
{
    TbTreeIntVD* o = NULL;

    o = (TbTreeIntVD*) alloc(NULL, sizeof(TbTreeIntVD));

    o->setAlloc(alloc);
    o->setRoot(NULL);
    o->setChainBeg(NULL);
    o->setChainEnd(NULL);
    o->setChainIns(NULL);
    o->setDelChain(NULL);
    o->setBlockSize(1024);

    return o;
}

//================================================================================================================
//================================================================================================================

void Core::TbTreeIntVD::discard()
{
    TbTreeIntVD* o = this;

    mRoot = NULL;

    TbTreeIntNodeVD* chainBeg = mChainBeg;

    while (chainBeg)
    {
        TbTreeIntNodeVD* newBeg = *(TbTreeIntNodeVD**)(chainBeg + mBlockSize);

        mAlloc(chainBeg, 0);

        chainBeg = newBeg;
    }

    mAlloc(o, 0);
}

//================================================================================================================
//================================================================================================================

Core::TbTreeIntNodeVD* Core::TbTreeIntVD::newNode()
{
    if (mDelChain)
    {
        TbTreeIntNodeVD* nn = mDelChain;

        mDelChain = nn->getParent();

        nn->init();

        return nn;
    }
    else
    {
        if (mChainIns == mChainEnd)
        {
            TbTreeIntNodeVD* newPtr = (TbTreeIntNodeVD*)mAlloc(NULL, mBlockSize * sizeof(TbTreeIntNodeVD) + sizeof(TbTreeIntNodeVD*));

            if (!mChainBeg)
            {
                mChainBeg = newPtr;
            }
            else
            {
                ((TbTreeIntNodeVD**)(mChainEnd))[0] = newPtr;
            }

            mChainIns = newPtr;
            mChainEnd = newPtr + mBlockSize;

            *(TbTreeIntNodeVD**)(mChainEnd) = NULL;
        }

        TbTreeIntNodeVD* nn = mChainIns;

        nn->init();

        mChainIns++;

        return nn;
    }
}

//================================================================================================================
//================================================================================================================

void Core::TbTreeIntVD::deleteNode(TbTreeIntNodeVD* node)
{
    node->setChild0(NULL);
    node->setChild1(NULL);
    node->setChild2(NULL);

    node->setParent(mDelChain);

    mDelChain = node;
}

//================================================================================================================
//================================================================================================================

void Core::TbTreeIntVD::push(TbTreeIntNodeVD* node, void* key, TbTreeIntNodeVD* child0, TbTreeIntNodeVD* child1)
{
    if (node->isFull())
    {
        TbTreeIntNodeVD* lnode = node;
        TbTreeIntNodeVD* rnode = newNode();

        void* root_key;

        if (key < node->getKeyT1())
        {
            root_key = lnode->getKeyT1();

            rnode->setKeyT1(lnode->getKeyT2());
            rnode->setChild0(lnode->getChild1());
            rnode->setChild1(lnode->getChild2());

            lnode->setKeyT1(key);
            lnode->setChild0(child0);
            lnode->setChild1(child1);
        }
        else if (key > node->getKeyT2())
        {
            root_key = lnode->getKeyT2();

            rnode->setKeyT1(key);
            rnode->setChild0(child0);
            rnode->setChild1(child1);
        }
        else
        {
            root_key = key;

            rnode->setKeyT1(lnode->getKeyT2());
            rnode->setChild1(lnode->getChild2());
            rnode->setChild0(child1);

            lnode->setChild1(child0);
        }

        rnode->setChild2(NULL);
        rnode->setParentChild0();
        rnode->setParentChild1();

        lnode->setChild2(NULL);
        lnode->setParentChild0();
        lnode->setParentChild1();

        if (lnode->getParent())
        {
            push(lnode->getParent(), root_key, lnode, rnode);
        }
        else
        {
            mRoot = newNode();

            mRoot->setKeyT1(root_key);
            mRoot->setChild0(lnode);
            mRoot->setChild1(rnode);

            lnode->setParent(mRoot);
            rnode->setParent(mRoot);
        }
    }
    else
    {
        if (key < node->getKeyT1())
        {
            node->setKeyT2(node->getKeyT1());
            node->setKeyT1(key);
            node->setChild2(node->getChild1());
            node->setChild1(child1);
            node->setChild0(child0);
            node->setParentChild0();
            node->setParentChild1();
            node->setParentChild2();
        }
        else
        {
            node->setKeyT2(key);
            node->setChild2(child1);
            node->setChild1(child0);
            node->setParentChild1();
            node->setParentChild2();
        }
    }
}

//================================================================================================================
//================================================================================================================

void Core::TbTreeIntVD::pull(TbTreeIntNodeVD* node)
{
    if (node->getChild0()->isFull())
    {
        if (node->getChild1()->isFull())
        {
            node->getChild0()->setKeyT1(node->getKeyT1());
            node->getChild0()->setChild1(node->getChild1()->getChild0());
            node->getChild0()->setParentChild1();
            node->setKeyT1(node->getChild1()->getKeyT1());
            node->getChild1()->setKeyT1(node->getChild1()->getKeyT2());
            node->getChild1()->setChild0(node->getChild1()->getChild1());
            node->getChild1()->setChild1(node->getChild1()->getChild2());
            node->getChild1()->setChild2(NULL);
        }
        else if (node->isFull())
        {
            node->getChild1()->setKeyT2(node->getChild1()->getKeyT1());
            node->getChild1()->setKeyT1(node->getKeyT1());
            node->getChild1()->setChild2(node->getChild1()->getChild1());
            node->getChild1()->setChild1(node->getChild1()->getChild0());
            node->getChild1()->setChild0(node->getChild0()->getChild0());
            node->getChild1()->setParentChild0();

            deleteNode(node->getChild0());

            node->setKeyT1(node->getKeyT2());
            node->setChild0(node->getChild1());
            node->setChild1(node->getChild2());
            node->setChild2(NULL);
        }
        else
        {
            node->getChild1()->setKeyT2(node->getChild1()->getKeyT1());
            node->getChild1()->setKeyT1(node->getKeyT1());
            node->getChild1()->setChild2(node->getChild1()->getChild1());
            node->getChild1()->setChild1(node->getChild1()->getChild0());
            node->getChild1()->setChild0(node->getChild0()->getChild0());
            node->getChild1()->setParentChild0();

            deleteNode(node->getChild0());

            node->setChild0(node->getChild1());
            node->setChild1(NULL);
            node->setChild2(NULL);

            if (node->getParent())
            {
                pull(node->getParent());
            }
            else
            {
                mRoot = node->getChild0();

                mRoot->setParent(NULL);

                deleteNode(node);
            }
        }
    }
    else if (node->getChild1()->isEmpty())
    {
        if (node->getChild0()->isFull())
        {
            node->getChild1()->setKeyT1(node->getKeyT1());
            node->getChild1()->setChild1(node->getChild1()->getChild0());
            node->getChild1()->setChild0(node->getChild0()->getChild2());
            node->getChild1()->setParentChild0();
            node->setKeyT1(node->getChild0()->getKeyT2());
            node->getChild0()->setChild2(NULL);
        }
        else if (node->isFull())
        {
            node->getChild0()->setKeyT2(node->getKeyT1());
            node->getChild0()->setChild2(node->getChild1()->getChild0());
            node->getChild0()->setParentChild2();

            deleteNode(node->getChild1());

            node->setKeyT1(node->getKeyT2());
            node->setChild1(node->getChild2());
            node->setChild2(NULL);
        }
        else
        {
            node->getChild0()->setKeyT2(node->getKeyT1());
            node->getChild0()->setChild2(node->getChild1()->getChild0());
            node->getChild0()->setParentChild2();

            deleteNode(node->getChild1());

            node->setChild1(NULL);
            node->setChild2(NULL);

            if (node->getParent())
            {
                pull(node->getParent());
            }
            else
            {
                mRoot = node->getChild0();

                mRoot->setParent(NULL);

                deleteNode(node);
            }
        }
    }
    else // node->child2 is empty
    {
        if (node->isFull())
        {
            if (node->getChild1()->isFull())
            {
                node->getChild2()->setKeyT1(node->getKeyT2());
                node->getChild2()->setChild1(node->getChild2()->getChild0());
                node->getChild2()->setChild0(node->getChild1()->getChild2());
                node->getChild2()->setParentChild0();
                node->setKeyT2(node->getChild1()->getKeyT2());
                node->getChild1()->setChild2(NULL);
            }
            else
            {
                node->getChild1()->setKeyT2(node->getKeyT2());
                node->getChild1()->setChild2(node->getChild2()->getChild0());
                node->getChild1()->setParentChild2();

                deleteNode(node->getChild2());

                node->setChild2(NULL);
            }
        }
    }
}

//================================================================================================================
//================================================================================================================

bool Core::TbTreeIntVD::exists(void* key)
{
    TbTreeIntNodeVD* node = mRoot->find(key);

    if (!node)
    {
        return false;
    }

    if (node->getKeyT1() == key)
    {
        return true;
    }

    if (node->getChild2() && node->getKeyT2() == key)
    {
        return true;
    }

    return false;
}

//================================================================================================================
//================================================================================================================

void* Core::TbTreeIntVD::largestBelowEqual(void* key)
{
    void* ret = mRoot->largestBelowEqual(key);

    // for the (hypothetical) case where ( NULL != (vp_t)0 )
    return (ret == 0) ? NULL : ret;
}

//================================================================================================================
//================================================================================================================

int Core::TbTreeIntVD::set(void* key)
{
    if (!mRoot)
    {
        mRoot = newNode();
        mRoot->setChild0(BNUL_VP);
        mRoot->setChild1(BNUL_VP);
        mRoot->setKeyT1(key);

        return 1;
    }

    TbTreeIntNodeVD* node = mRoot->find(key);

    if (!node)
    {
        return -2;
    }

    if (node->getKeyT1() == key)
    {
        return 0;
    }
    else if (node->getChild2() && node->getKeyT2() == key)
    {
        return 0;
    }
    else
    {
        push(node, key, BNUL_VP, BNUL_VP);

        return 1;
    }
}

//================================================================================================================
//================================================================================================================

int Core::TbTreeIntVD::remove(void* key)
{
    if (!mRoot)
    {
        return 0;
    }

    TbTreeIntNodeVD* node = mRoot->find(key);

    if (!node)
    {
        return -1;
    }

    if (node->getKeyT1() == key)
    {
        if (!node->isLeaf())
        {
            TbTreeIntNodeVD* trace = node->getChild0();

            while (!trace->isLeaf())
            {
                trace = (trace->getChild2()) ? trace->getChild2() : trace->getChild1();
            }

            if (trace->isFull())
            {
                node->setKeyT1(trace->getKeyT2());

                trace->setChild2(NULL);
            }
            else
            {
                node->setKeyT1(trace->getKeyT1());

                trace->setChild1(NULL);
                trace->setChild2(NULL);

                pull(trace->getParent());
            }
        }
        else if (node->isFull())
        {
            node->setKeyT1(node->getKeyT2());

            node->setChild2(NULL);
        }
        else
        {
            node->setChild1(NULL);
            node->setChild2(NULL);

            if (node->getParent())
            {
                pull(node->getParent());
            }
            else
            {
                deleteNode(node);

                mRoot = NULL;
            }
        }

        return 1;
    }

    if (node->getKeyT2() == key)
    {
        if (!node->isLeaf())
        {
            TbTreeIntNodeVD* trace = (node->getChild2()) ? node->getChild2() : node->getChild1();

            while (!trace->isLeaf())
            {
                trace = trace->getChild0();
            }

            if (trace->isFull())
            {
                node->setKeyT2(trace->getKeyT1());

                trace->setKeyT1(trace->getKeyT2());

                trace->setChild2(NULL);
            }
            else
            {
                node->setKeyT2(trace->getKeyT1());

                trace->setChild1(NULL);
                trace->setChild2(NULL);

                pull(trace->getParent());
            }
        }
        else if (node->isFull())
        {
            node->setChild2(NULL);
        }
        else
        {
            node->setChild1(NULL);
            node->setChild2(NULL);

            if (node->getParent())
            {
                pull(node->getParent());
            }
            else
            {
                deleteNode(node);

                mRoot = NULL;
            }
        }

        return 1;
    }

    return 0;
}

//================================================================================================================
//================================================================================================================

void Core::TbTreeIntVD::run(void_callback2 func, void* arg)
{
    mRoot->run(func, arg);
}

//================================================================================================================
//================================================================================================================

size_t Core::TbTreeIntVD::count(bool_callback2 func, void* arg)
{
    return mRoot->count(func, arg);
}

//================================================================================================================
//================================================================================================================

size_t Core::TbTreeIntVD::depth()
{
    return mRoot->depth();
}

//================================================================================================================
//================================================================================================================

void Core::TbTreeIntVD::printStatus()
{
    size_t blocks = 0;
    size_t nodes = 0;
    size_t deletedNodes = 0;

    if (mChainBeg)
    {
        TbTreeIntNodeVD* chainBeg = mChainBeg;

        while (chainBeg)
        {
            chainBeg = *(TbTreeIntNodeVD**)(chainBeg + mBlockSize);

            blocks++;
        }

        nodes = blocks * mBlockSize - (mChainEnd - mChainIns);
    }

    if (mDelChain)
    {
        TbTreeIntNodeVD* delChain = mDelChain;

        while (delChain)
        {
            delChain = delChain->getParent();

            deletedNodes++;
        }
    }

    size_t usedNodes = nodes - deletedNodes;

    printf("keys ........... %zu\n", mRoot->keys());
    printf("nodes .......... %zu\n", usedNodes);
    printf("keys/nodes ..... %5.4f\n", usedNodes > 0 ? (double)(mRoot->keys()) / usedNodes : 0);
    printf("depth .......... %zu\n", mRoot->depth());
    printf("block size ..... %zu\n", mBlockSize);
    printf("blocks ......... %zu\n", blocks);
    printf("deleted nodes .. %zu\n", deletedNodes);
}

//================================================================================================================
//================================================================================================================