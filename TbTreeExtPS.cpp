#include "TbTreeExtPS.h"

//================================================================================================================
//================================================================================================================

void Core::TbTreeExtPS::init()
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

Core::TbTreeExtPS* Core::TbTreeExtPS::create(alloc_callback alloc)
{
    TbTreeExtPS* o = NULL;

    o = (TbTreeExtPS*) alloc(NULL, sizeof(TbTreeExtPS));

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

void Core::TbTreeExtPS::discard()
{
    TbTreeExtPS* o = this;

    mRoot = NULL;

    TbTreeExtNodePS* chainBeg = mChainBeg;

    while (chainBeg)
    {
        TbTreeExtNodePS* newBeg = *(TbTreeExtNodePS**)(chainBeg + mBlockSize);

        mAlloc(chainBeg, 0);

        chainBeg = newBeg;
    }

    mAlloc(o, 0);
}

//================================================================================================================
//================================================================================================================

Core::TbTreeExtNodePS* Core::TbTreeExtPS::newNode()
{
    if (mDelChain)
    {
        TbTreeExtNodePS* nn = mDelChain;

        mDelChain = nn->getParent();

        nn->init();

        return nn;
    }
    else
    {
        if (mChainIns == mChainEnd)
        {
            TbTreeExtNodePS* newPtr = (TbTreeExtNodePS*) mAlloc(NULL, mBlockSize * sizeof(TbTreeExtNodePS) + sizeof(TbTreeExtNodePS*));

            if (!mChainBeg)
            {
                mChainBeg = newPtr;
            }
            else
            {
                ((TbTreeExtNodePS**)(mChainEnd))[0] = newPtr;
            }

            mChainIns = newPtr;
            mChainEnd = newPtr + mBlockSize;

            *(TbTreeExtNodePS**)(mChainEnd) = NULL;
        }

        TbTreeExtNodePS* nn = mChainIns;

        nn->init();

        mChainIns++;

        return nn;
    }
}

//================================================================================================================
//================================================================================================================

void Core::TbTreeExtPS::deleteNode(TbTreeExtNodePS* node)
{
    node->setChild0(NULL);
    node->setChild1(NULL);
    node->setChild2(NULL);

    node->setParent(mDelChain);

    mDelChain = node;
}

//================================================================================================================
//================================================================================================================

void Core::TbTreeExtPS::push(TbTreeExtNodePS* node, void* key, size_t val, TbTreeExtNodePS* child0, TbTreeExtNodePS* child1)
{
    if (node->isFull())
    {
        TbTreeExtNodePS* lnode = node;
        TbTreeExtNodePS* rnode = newNode();

        void* root_key;
        size_t root_val;

        if (key < node->getKeyT1())
        {
            root_key = lnode->getKeyT1();
            root_val = lnode->getValT1();

            rnode->setKeyT1(lnode->getKeyT2());
            rnode->setValT1(lnode->getValT2());
            rnode->setChild0(lnode->getChild1());
            rnode->setChild1(lnode->getChild2());

            lnode->setKeyT1(key);
            lnode->setValT1(val);
            lnode->setChild0(child0);
            lnode->setChild1(child1);
        }
        else if (key > node->getKeyT2())
        {
            root_key = lnode->getKeyT2();
            root_val = lnode->getValT2();

            rnode->setKeyT1(key);
            rnode->setValT1(val);
            rnode->setChild0(child0);
            rnode->setChild1(child1);
        }
        else
        {
            root_key = key;
            root_val = val;

            rnode->setKeyT1(lnode->getKeyT2());
            rnode->setValT1(lnode->getValT2());
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
            push(lnode->getParent(), root_key, root_val, lnode, rnode);
        }
        else
        {
            mRoot = newNode();

            mRoot->setKeyT1(root_key);
            mRoot->setValT1(root_val);
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
            node->setValT2(node->getValT1());
            node->setKeyT1(key);
            node->setValT1(val);
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
            node->setValT2(val);
            node->setChild2(child1);
            node->setChild1(child0);
            node->setParentChild1();
            node->setParentChild2();
        }
    }
}

//================================================================================================================
//================================================================================================================

void Core::TbTreeExtPS::pull(TbTreeExtNodePS* node)
{
    if (node->getChild0()->isFull())
    {
        if (node->getChild1()->isFull())
        {
            node->getChild0()->setKeyT1(node->getKeyT1());
            node->getChild0()->setValT1(node->getValT1());

            node->getChild0()->setChild1(node->getChild1()->getChild0());

            node->getChild0()->setParentChild1();

            node->setKeyT1(node->getChild1()->getKeyT1());
            node->setValT1(node->getChild1()->getValT1());

            node->getChild1()->setKeyT1(node->getChild1()->getKeyT2());
            node->getChild1()->setValT1(node->getChild1()->getValT2());

            node->getChild1()->setChild0(node->getChild1()->getChild1());
            node->getChild1()->setChild1(node->getChild1()->getChild2());

            node->getChild1()->setChild2(NULL);
        }
        else if (node->isFull())
        {
            node->getChild1()->setKeyT2(node->getChild1()->getKeyT1());
            node->getChild1()->setValT2(node->getChild1()->getValT1());

            node->getChild1()->setKeyT1(node->getKeyT1());
            node->getChild1()->setValT1(node->getValT1());

            node->getChild1()->setChild2(node->getChild1()->getChild1());
            node->getChild1()->setChild1(node->getChild1()->getChild0());
            node->getChild1()->setChild0(node->getChild0()->getChild0());

            node->getChild1()->setParentChild0();

            deleteNode(node->getChild0());

            node->setKeyT1(node->getKeyT2());
            node->setValT1(node->getValT2());

            node->setChild0(node->getChild1());
            node->setChild1(node->getChild2());

            node->setChild2(NULL);
        }
        else
        {
            node->getChild1()->setKeyT2(node->getChild1()->getKeyT1());
            node->getChild1()->setValT2(node->getChild1()->getValT1());

            node->getChild1()->setKeyT1(node->getKeyT1());
            node->getChild1()->setValT1(node->getValT1());

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
            node->getChild1()->setValT1(node->getValT1());

            node->getChild1()->setChild1(node->getChild1()->getChild0());
            node->getChild1()->setChild0(node->getChild0()->getChild2());

            node->getChild1()->setParentChild0();

            node->setKeyT1(node->getChild0()->getKeyT2());
            node->setValT1(node->getChild0()->getValT2());

            node->getChild0()->setChild2(NULL);
        }
        else if (node->isFull())
        {
            node->getChild0()->setKeyT2(node->getKeyT1());
            node->getChild0()->setValT2(node->getValT1());

            node->getChild0()->setChild2(node->getChild1()->getChild0());

            node->getChild0()->setParentChild2();

            deleteNode(node->getChild1());

            node->setKeyT1(node->getKeyT2());
            node->setValT1(node->getValT2());

            node->setChild1(node->getChild2());

            node->setChild2(NULL);
        }
        else
        {
            node->getChild0()->setKeyT2(node->getKeyT1());
            node->getChild0()->setValT2(node->getValT1());

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
                node->getChild2()->setValT1(node->getValT2());

                node->getChild2()->setChild1(node->getChild2()->getChild0());
                node->getChild2()->setChild0(node->getChild1()->getChild2());

                node->getChild2()->setParentChild0();

                node->setKeyT2(node->getChild1()->getKeyT2());
                node->setValT2(node->getChild1()->getValT2());

                node->getChild1()->setChild2(NULL);
            }
            else
            {
                node->getChild1()->setKeyT2(node->getKeyT2());
                node->getChild1()->setValT2(node->getValT2());

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

size_t* Core::TbTreeExtPS::val(void* key)
{
    TbTreeExtNodePS* node = mRoot->find(key);

    if (!node)
    {
        return NULL;
    }

    if (node->getKeyT1() == key)
    {
        return node->getValT1Ptr();
    }

    if (node->getChild2() && node->getKeyT2() == key)
    {
        return node->getValT2Ptr();
    }

    return NULL;
}

//================================================================================================================
//================================================================================================================

int Core::TbTreeExtPS::set(void* key, size_t val)
{
    if (!mRoot)
    {
        mRoot = newNode();
        mRoot->setChild0(BNUL_PS);
        mRoot->setChild1(BNUL_PS);
        mRoot->setChild2(NULL);
        mRoot->setKeyT1(key);
        mRoot->setValT1(val);

        return 1;
    }

    TbTreeExtNodePS* node = mRoot->find(key);

    if (!node)
    {
        return -2;
    }

    if (node->getKeyT1() == key)
    {
        if (node->getValT1() == val)
        {
            return 0;
        }

        node->setValT1(val);

        return -1;
    }
    else if (node->getChild2() && node->getKeyT2() == key)
    {
        if (node->getValT2() == val)
        {
            return 0;
        }

        node->setValT2(val);

        return -1;
    }
    else
    {
        push(node, key, val, BNUL_PS, BNUL_PS);

        return 1;
    }
}

//================================================================================================================
//================================================================================================================

int Core::TbTreeExtPS::remove(void* key)
{
    if (!mRoot)
    {
        return 0;
    }

    TbTreeExtNodePS* node = mRoot->find(key);

    if (!node)
    {
        return -1;
    }

    if (node->getKeyT1() == key)
    {
        if (!node->isLeaf())
        {
            TbTreeExtNodePS* trace = node->getChild0();

            while (!trace->isLeaf())
            {
                trace = (trace->getChild2()) ? trace->getChild2() : trace->getChild1();
            }

            if (trace->isFull())
            {
                node->setKeyT1(trace->getKeyT2());
                node->setValT1(trace->getValT2());

                trace->setChild2(NULL);
            }
            else
            {
                node->setKeyT1(trace->getKeyT1());
                node->setValT1(trace->getValT1());

                trace->setChild1(NULL);
                trace->setChild2(NULL);

                pull(trace->getParent());
            }
        }
        else if (node->isFull())
        {
            node->setKeyT1(node->getKeyT2());
            node->setValT1(node->getValT2());

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
            TbTreeExtNodePS* trace = (node->getChild2()) ? node->getChild2() : node->getChild1();

            while (!trace->isLeaf())
            {
                trace = trace->getChild0();
            }

            if (trace->isFull())
            {
                node->setKeyT2(trace->getKeyT1());
                node->setValT2(trace->getValT1());

                trace->setKeyT1(trace->getKeyT2());
                trace->setValT1(trace->getValT2());

                trace->setChild2(NULL);
            }
            else
            {
                node->setKeyT2(trace->getKeyT1());
                node->setValT2(trace->getValT1());

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

void Core::TbTreeExtPS::run(void_callback1 func, void* arg)
{
    mRoot->run(func, arg);
}

//================================================================================================================
//================================================================================================================

size_t Core::TbTreeExtPS::count(bool_callback1 func, void* arg)
{
    if (mRoot == NULL) return 0;
    return mRoot->count(func, arg);
}

//================================================================================================================
//================================================================================================================

size_t Core::TbTreeExtPS::sum(bool_callback1 func, void* arg)
{
    if (mRoot == NULL) return 0;
    return mRoot->sum(func, arg);
}

//================================================================================================================
//================================================================================================================

size_t Core::TbTreeExtPS::depth()
{
    if (mRoot == NULL) return 0;
    return mRoot->depth();
}

//================================================================================================================
//================================================================================================================

void Core::TbTreeExtPS::printStatus()
{
    size_t blocks = 0;
    size_t nodes = 0;
    size_t deletedNodes = 0;

    if (mChainBeg)
    {
        TbTreeExtNodePS* chainBeg = mChainBeg;

        while (chainBeg)
        {
            chainBeg = *(TbTreeExtNodePS**)(chainBeg + mBlockSize);

            blocks++;
        }

        nodes = blocks * mBlockSize - (mChainEnd - mChainIns);
    }
    
    if (mDelChain)
    {
        TbTreeExtNodePS* delChain = mDelChain;

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