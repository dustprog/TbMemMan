#include "TbMemMan.h"
#include <mutex>

Core::TbMemMan* Core::TbMemMan::gDefaultTbman = NULL;

//================================================================================================================
//================================================================================================================

Core::TbMemMan::TbMemMan()
{
    // This original code did a memset
    /*Core::TbManUtility::reset(mData);
    Core::TbManUtility::reset(mSize);
    Core::TbManUtility::reset(mPoolSize);
    Core::TbManUtility::reset(mMinBlockSize);
    Core::TbManUtility::reset(mMaxBlockSize);
    Core::TbManUtility::reset(mAligned);
    Core::TbManUtility::reset(mBlockSizeArray);
    Core::TbManUtility::reset(mInternalBTree);
    Core::TbManUtility::reset(mExternalBTree);
    Core::TbManUtility::reset(mMutex);
    Core::TbManUtility::reset(mIArg);
    Core::TbManUtility::reset(mMNode);
    Core::TbManUtility::reset(mMNodeArr);
    Core::TbManUtility::reset(mDiag);*/

    mData = NULL;
    mSize = 0;
    mPoolSize = 0;
    mMinBlockSize = 0;
    mMaxBlockSize = 0;
    mAligned = false;
    mBlockSizeArray = NULL;
    mInternalBTree = NULL;
    mExternalBTree = NULL;
    
    memset(&mMutex, 0, sizeof(mMutex));
    memset(&mIArg, 0, sizeof(mIArg));
    memset(&mMNode, 0, sizeof(mMNode));
    memset(&mMNodeArr, 0, sizeof(mMNodeArr));
    memset(&mDiag, 0, sizeof(mDiag));
}

//================================================================================================================
//================================================================================================================

Core::TbMemMan::~TbMemMan()
{
}

//================================================================================================================
//================================================================================================================

void Core::TbMemMan::init(size_t poolSize, size_t minBlockSize, size_t maxBlockSize, size_t steppingMethod, bool fullAlign)
{
    mData = NULL;
    mSize = 0;
    mPoolSize = 0;
    mMinBlockSize = 0;
    mMaxBlockSize = 0;
    mAligned = false;
    mBlockSizeArray = NULL;
    mInternalBTree = NULL;
    mExternalBTree = NULL;

    memset(&mMutex, 0, sizeof(mMutex));
    memset(&mIArg, 0, sizeof(mIArg));
    memset(&mMNode, 0, sizeof(mMNode));
    memset(&mMNodeArr, 0, sizeof(mMNodeArr));
    memset(&mDiag, 0, sizeof(mDiag));

    // Initialize the NULL nodes
    //Core::btree_node_ps_s_null_g = (TbTreeExtNodePS*) malloc(sizeof(TbTreeExtNodePS*));
    //Core::btree_node_vd_s_null_g = (TbTreeIntNodeVD*) malloc(sizeof(TbTreeIntNodeVD*));

    // Initialize the NULL nodes

    TbTreeExtNodePS extNullNode(NULL, 0, NULL, 0, NULL, NULL, NULL, NULL);
    TbTreeExtNodePS::btree_node_ps_s_null_g = extNullNode;

    TbTreeIntNodeVD intNullNode(NULL, NULL, NULL, NULL, NULL, NULL);
    TbTreeIntNodeVD::btree_node_vd_s_null_g = intNullNode;

    mtx_init(&mMutex, NULL);

    mInternalBTree = TbTreeIntVD::create(stdlib_alloc);
    mExternalBTree = TbTreeExtPS::create(stdlib_alloc);

    /// The following three values are configurable parameters of memory manager
    mPoolSize = poolSize;
    mMinBlockSize = minBlockSize;
    mMaxBlockSize = maxBlockSize;

    size_t maskBxp = steppingMethod;
    size_t sizeMask = (1 << maskBxp) - 1;
    size_t sizeInc = mMinBlockSize;

    while ((sizeMask < mMinBlockSize) || ((sizeMask << 1) & mMinBlockSize) != 0)
    {
        // multiplies by 2 to the power of 1
        sizeMask <<= 1;
    }

    size_t space = 0;

    for (size_t blockSize = mMinBlockSize; blockSize <= mMaxBlockSize; blockSize += sizeInc)
    {
        if (mSize == space)
        {
            space = space > 0 ? space * 2 : 16;

            if (mData)
            {
                // o->data = (block_manager_s**)realloc( o->data, sizeof( block_manager_s* ) * space );
                TbBlockManager** data = (TbBlockManager**)realloc(mData, sizeof(TbBlockManager*) * space);

                if (data == NULL)
                {
                    ERR("Memory reallocation failed, the program will terminate.");
                    //cout << "Memory reallocation failed, the program will terminate." << endl;
                    // free( this->digits ); provided that the destructor does not call free itself
                    exit(0);
                }

                mData = data;
            }
            else
            {
                mData = (TbBlockManager**) malloc(sizeof(TbBlockManager*) * space);
            }

            if (!mData)
            {
                ERR("Failed allocating %zu bytes", sizeof(TbBlockManager*) * space);
            }
        }

        if (mData == NULL)
        {
            ERR("Memory reallocation failed, the program will terminate.");
            //cout << "Memory reallocation failed, the program will terminate." << endl;
            // free( this->digits ); provided that the destructor does not call free itself
            exit(0);
        }

        mData[mSize] = TbBlockManager::create(mPoolSize, blockSize, fullAlign);
        mData[mSize]->setInternalBTree(mInternalBTree);
        mData[mSize]->setParent(this);

        mSize++;

        if (blockSize > sizeMask)
        {
            // multiplies by 2 to the power of 1
            sizeMask <<= 1;
            sizeInc <<= 1;
        }
    }

    mBlockSizeArray = (size_t*) malloc(mSize * sizeof(size_t));

    if (!mBlockSizeArray)
    {
        ERR("Failed allocating %zu bytes", mSize * sizeof(size_t));
    }

    mAligned = true;

    for (size_t i = 0; i < mSize; i++)
    {
        mAligned = mAligned && mData[i]->isAligned();

        mBlockSizeArray[i] = mData[i]->getBlockSize();
    }
}

//================================================================================================================
//================================================================================================================

Core::TbMemMan* Core::TbMemMan::create(size_t poolSize, size_t minBlockSize, size_t maxBlockSize, size_t steppingMethod, bool fullAlign)
{
    TbMemMan* o = (TbMemMan*)malloc(sizeof(TbMemMan));

    if (!o)
    {
        ERR("Failed allocating %zu bytes", sizeof(TbMemMan));
    }

    o->init(poolSize, minBlockSize, maxBlockSize, steppingMethod, fullAlign);

    return o;
}

//================================================================================================================
//================================================================================================================

void Core::TbMemMan::open()
{
    ////static pthread_once_t flag = PTHREAD_ONCE_INIT;
    //static std::once_flag flag = ONCE_FLAG_INIT;
    ////int ern = pthread_once( &flag, create_tbman );
    //call_once(&flag, create_tbman);
    ////if( ern ) ERR( "function returned error %i", ern );

    gDefaultTbman = TbMemMan::createDefault();

    static std::once_flag flag;
    std::call_once(flag, [&]() { gDefaultTbman; });
}

//================================================================================================================
//================================================================================================================

//Core::TbMemMan* Core::TbMemMan::instanceOpen()
//{
//    return createDefault();
//}

//================================================================================================================
//================================================================================================================

Core::TbMemMan* Core::TbMemMan::createDefault()
{
    return TbMemMan::create
    (
        mDefaultPoolSize,
        mDefaultMinBlockSize,
        mDefaultMaxBlockSize,
        mDefaultSteppingMethod,
        mDefaultFullAlign
    );
}

//================================================================================================================
//================================================================================================================

void Core::TbMemMan::discard(TbMemMan* value)
{
    if (!value)
    {
        return;
    }

    value->down();

    //free(this);
    _aligned_free(value);
}

//================================================================================================================
//================================================================================================================

//void* Core::TbMemMan::memSAlloc(void* currentPtr, size_t requestedSize, size_t* grantedSize)
//{
//    if (!gDefaultTbman) return NULL;
//
//    //pthread_mutex_lock( &gDefaultTbman->getMutex() );
//    mtx_lock(&gDefaultTbman->getMutex());
//
//    void* ret = NULL;
//
//    if (requestedSize == 0)
//    {
//        if (currentPtr)
//        {
//            gDefaultTbman->memFree(currentPtr, NULL);
//        }
//
//        if (grantedSize)
//        {
//            *grantedSize = 0;
//        }
//    }
//    else
//    {
//        if (currentPtr)
//        {
//            ret = gDefaultTbman->memRealloc(currentPtr, NULL, requestedSize, grantedSize);
//        }
//        else
//        {
//            ret = gDefaultTbman->memAlloc(requestedSize, grantedSize);
//        }
//    }
//
//    //pthread_mutex_unlock( &gDefaultTbman->getMutex() );
//    mtx_unlock(&gDefaultTbman->getMutex());
//
//    return ret;
//}

//================================================================================================================
//================================================================================================================

void* Core::TbMemMan::memNalloc(void* currentPtr, size_t currentSize, size_t requestedSize, size_t* grantedSize)
{
    if (!gDefaultTbman) return NULL;

    mtx_t mutex = gDefaultTbman->getMutex();

    //pthread_mutex_lock( &gDefaultTbman->getMutex() );
    mtx_lock(&mutex);

    void* ret = NULL;

    if (requestedSize == 0)
    {
        if (currentSize) // 0 means currentPtr may not be used for free or realloc
        {
            gDefaultTbman->memFree(currentPtr, &currentSize);
        }

        if (grantedSize)
        {
            *grantedSize = 0;
        }
    }
    else
    {
        if (currentSize) // 0 means currentPtr may not be used for free or realloc
        {
            ret = gDefaultTbman->memRealloc(currentPtr, &currentSize, requestedSize, grantedSize);
        }
        else
        {
            ret = gDefaultTbman->memAlloc(requestedSize, grantedSize);
        }
    }

    //pthread_mutex_unlock( &gDefaultTbman->getMutex() );
    mtx_unlock(&mutex);

    return ret;
}

//================================================================================================================
//================================================================================================================

void* Core::TbMemMan::nallocNoCurrentBytes(void* currentPtr, size_t currentBytes, size_t requestedBytes, size_t* grantedBytes)
{
    if (!gDefaultTbman) return NULL;

    return gDefaultTbman->memAlloc(currentPtr, requestedBytes, grantedBytes);
}

//================================================================================================================
//================================================================================================================

void Core::TbMemMan::diagnosticTestCallback(void* arg, void* ptr, size_t space)
{
    diagnostic* d = (diagnostic*)arg;

    int found = false;

    for (size_t i = 0; i < d->size; i++)
    {
        if (ptr == d->ptr_arr[i])
        {
            found = true;

            ASSERT(space == d->spc_arr[i]);
        }
    }

    ASSERT(found);

    d->man->memAlloc(ptr, 0, NULL);
}

//================================================================================================================
//================================================================================================================

void Core::TbMemMan::diagnosticTest()
{
    diagnostic diag;

    diag.man = TbMemMan::createDefault();
    diag.size = 1000;
    diag.ptr_arr = (void**)malloc(sizeof(void*) * diag.size);
    diag.spc_arr = (size_t*)malloc(sizeof(size_t) * diag.size);

    uint32_t rval = 1234;

    for (size_t i = 0; i < diag.size; i++)
    {
        rval = TbManUtility::xsg_u2(rval);
        size_t size = rval % 20000;

        diag.ptr_arr[i] = diag.man->memAlloc(NULL, size, &diag.spc_arr[i]);
    }

    ASSERT(diag.man->totalInstances() == diag.size);

    // the callback function frees memory
    diag.man->forEachInstance(diagnosticTestCallback, &diag);

    ASSERT(diag.man->totalGrantedSpace() == 0);
    ASSERT(diag.man->totalInstances() == 0);

    free(diag.ptr_arr);
    free(diag.spc_arr);

    diag.man->close();
}

//================================================================================================================
//================================================================================================================

void Core::TbMemMan::test()
{
    size_t table_size = 100000;
    size_t cycles = 10;
    size_t max_alloc = 65536;
    size_t seed = 1237;

    bool verbose = true; // set 'true' for more expressive test results

    // perform a memset of tbman variables before each test

    printf("Memory Manager Evaluation:\n");
    {
        printf("\nmalloc, free, realloc (stdlib) ...\n");
        TbMemMan::allocChallenge(external_nalloc, table_size, cycles, max_alloc, seed, true, verbose);
    }

    {
        printf("\ntbman_malloc, tbman_free, tbman_realloc ...\n");
        TbMemMan::allocChallenge(nallocNoCurrentBytes, table_size, cycles, max_alloc, seed, true, verbose);
    }

    {
        printf("\ntbman_malloc, tbman_nfree, tbman_nrealloc ...\n");
        TbMemMan::allocChallenge(TbMemMan::memNalloc, table_size, cycles, max_alloc, seed, true, verbose);
    }

    printf("\ndiagnostic test ... ");
    {
        TbMemMan::diagnosticTest();
    }
    printf("success!\n");
}

//================================================================================================================
//================================================================================================================

void Core::TbMemMan::allocChallenge(
    fp_alloc alloc,
    size_t table_size,
    size_t cycles,
    size_t max_alloc,
    uint32_t seed,
    bool cleanup,
    bool verbose)
{
    void** data_table = (void**)malloc(table_size * sizeof(void*));
    size_t* size_table = (size_t*)malloc(table_size * sizeof(size_t));

    for (size_t i = 0; i < table_size; i++)
    {
        data_table[i] = NULL;
    }

    for (size_t i = 0; i < table_size; i++)
    {
        size_table[i] = 0;
    }

    uint32_t rval = seed;
    size_t alloc_attempts = 0;
    size_t realloc_attempts = 0;
    size_t free_attempts = 0;
    size_t alloc_failures = 0;
    size_t realloc_failures = 0;
    size_t free_failures = 0;

    // Functionality test: Mix of malloc, free, realloc
    for (size_t j = 0; j < cycles; j++)
    {
        for (size_t i = 0; i < table_size; i++)
        {
            rval = TbManUtility::xsg_u2(rval);
            size_t idx = rval % table_size;

            // verify table content
            if (size_table[idx] > 0)
            {
                uint32_t rv = TbManUtility::xsg_u2(idx + 1);
                uint8_t* data = (uint8_t*)data_table[idx];
                size_t sz = size_table[idx];

                for (size_t i = 0; i < sz; i++)
                {
                    rv = TbManUtility::xsg_u2(rv);

                    if (data[i] != (rv & 255))
                    {
                        fprintf(stderr, "data failure [%u vs %u].", (uint32_t)data[i], (uint32_t)(rv & 255));

                        abort();
                    }
                }
            }

            if (data_table[idx] == NULL)
            {
                rval = TbManUtility::xsg_u2(rval);
                size_t size = pow((double)max_alloc, (rval * pow(2.0, -32)));

                // This line is causing an error with 2nd set of Core tests at j=0,i=224 in the loops, IS_2_POW_N(alignment) issue
                // _aligned_malloc is throwing the error in the tbman_s_mem_alloc method
                data_table[idx] = alloc(data_table[idx], 0, size, &size_table[idx]);
                alloc_attempts++;
                alloc_failures += (size > 0) && (data_table[idx] == NULL);

                // set new content
                if (size_table[idx] > 0)
                {
                    uint32_t rv = TbManUtility::xsg_u2(idx + 1);
                    uint8_t* data = (uint8_t*)data_table[idx];
                    size_t sz = size_table[idx];

                    for (size_t i = 0; i < sz; i++)
                    {
                        data[i] = ((rv = TbManUtility::xsg_u2(rv)) & 255);
                    }
                }
            }
            else
            {
                rval = TbManUtility::xsg_u2(rval);
                if (rval & 32)
                {
                    data_table[idx] = alloc(data_table[idx], size_table[idx], 0, &size_table[idx]); // free
                    free_attempts++;
                    free_failures += (data_table[idx] != NULL);
                }
                else
                {
                    rval = TbManUtility::xsg_u2(rval);
                    size_t size = pow((double)max_alloc, rval * pow(2.0, -32));

                    size_t new_size = 0;

                    data_table[idx] = alloc(data_table[idx], size_table[idx], size, &new_size); // realloc

                    // verify old table content (only when size > sz - stdlib realloc does not seem to retain data otherwise)
                    if (size > size_table[idx])
                    {
                        if (data_table[idx] != NULL && size_table[idx] > 0)
                        {
                            uint32_t rv = TbManUtility::xsg_u2(idx + 1);
                            uint8_t* data = (uint8_t*)data_table[idx];
                            size_t sz = size_table[idx];

                            for (size_t i = 0; i < sz; i++)
                            {
                                rv = TbManUtility::xsg_u2(rv);

                                if (data[i] != (rv & 255))
                                {
                                    fprintf(stderr, "data failure [%u vs %u].", (uint32_t)data[i], (uint32_t)(rv & 255));

                                    abort();
                                }
                            }
                        }
                    }

                    size_table[idx] = new_size; //( data_table[ idx ] != NULL ) ? size : 0;
                    realloc_attempts++;
                    realloc_failures += (size > 0) && (data_table[idx] == NULL);

                    // set new content
                    if (size_table[idx] > 0)
                    {
                        uint32_t rv = TbManUtility::xsg_u2(idx + 1);
                        uint8_t* data = (uint8_t*)data_table[idx];
                        size_t sz = size_table[idx];

                        for (size_t i = 0; i < sz; i++)
                        {
                            data[i] = ((rv = TbManUtility::xsg_u2(rv)) & 255);
                        }
                    }
                }
            }
        }
    }

    size_t allocated_table_size = 0;

    for (size_t i = 0; i < table_size; i++)
    {
        allocated_table_size += (data_table[i] != NULL);
    }

    if (verbose)
    {
        printf("cycles ............... %zu\n", cycles);
        printf("max alloc size ....... %zu\n", max_alloc);
        printf("Instances\n");
        printf("  total .............. %zu\n", table_size);
        printf("  allocated .......... %zu\n", allocated_table_size);
        printf("Alloc\n");
        printf("  attempts  .......... %zu\n", alloc_attempts);
        printf("  failures  .......... %zu\n", alloc_failures);
        printf("Realloc\n");
        printf("  attempts  .......... %zu\n", realloc_attempts);
        printf("  failures  .......... %zu\n", realloc_failures);
        printf("Free\n");
        printf("  attempts  .......... %zu\n", free_attempts);
        printf("  failures  .......... %zu\n", free_failures);
    }

    size_t local_table_size = 10 < table_size ? 10 : table_size;
    size_t local_cycles = table_size / local_table_size;

    // Dummy loops: Assessment of overhead time, which is to be
    // subtracted from time needed for the principal loop
    clock_t overhead_time = 0;
    {
        size_t* size_buf = (size_t*)malloc(table_size * sizeof(size_t));
        clock_t time = clock();

        for (size_t j = 0; j < cycles; j++)
        {
            for (size_t i = 0; i < table_size; i++)
            {
                rval = TbManUtility::xsg_u2(rval);
                size_t idx = rval % table_size;
                rval = TbManUtility::xsg_u2(rval);
                size_t size = pow((double)max_alloc, rval * pow(2.0, -32));

                if (data_table[idx] == NULL)
                {
                    size_buf[idx] = size;
                }
                else
                {
                    size_buf[idx] = 0;
                }
            }
        }

        free(size_buf);

        overhead_time = clock() - time;
    }

    clock_t local_overhead_time = 0;
    {
        size_t* size_buf = (size_t*)malloc(table_size * sizeof(size_t));
        clock_t time = clock();

        for (size_t k = 0; k < cycles; k++)
        {
            size_t local_seed = (rval = TbManUtility::xsg_u2(rval));

            for (size_t j = 0; j < local_cycles; j++)
            {
                rval = local_seed;

                for (size_t i = 0; i < local_table_size; i++)
                {
                    rval = TbManUtility::xsg_u2(rval);
                    size_t idx = rval % table_size;
                    rval = TbManUtility::xsg_u2(rval);
                    size_t size = pow((double)max_alloc, rval * pow(2.0, -32));

                    if (data_table[idx] == NULL)
                    {
                        size_buf[idx] = size;
                    }
                    else
                    {
                        size_buf[idx] = 0;
                    }
                }
            }
        }

        free(size_buf);
        local_overhead_time = clock() - time;
    }

    // Equilibrium speed test: malloc, free
    {
        clock_t time = clock();

        for (size_t j = 0; j < cycles; j++)
        {
            for (size_t i = 0; i < table_size; i++)
            {
                rval = TbManUtility::xsg_u2(rval);
                size_t idx = rval % table_size;
                rval = TbManUtility::xsg_u2(rval);
                size_t size = pow((double)max_alloc, rval * pow(2.0, -32));

                if (data_table[idx] == NULL)
                {
                    data_table[idx] = alloc(data_table[idx], 0, size, &size_table[idx]); // malloc
                }
                else
                {
                    data_table[idx] = alloc(data_table[idx], size_table[idx], 0, &size_table[idx]); // free
                }
            }
        }

        time = clock() - time - overhead_time;
        size_t ns = (1E9 * time) / (CLOCKS_PER_SEC * cycles * table_size);
        printf("speed test alloc-free (general): %6zuns per call\n", ns);
    }

    // Equilibrium speed test: realloc
    {
        clock_t time = clock();
        for (size_t j = 0; j < cycles; j++)
        {
            for (size_t i = 0; i < table_size; i++)
            {
                rval = TbManUtility::xsg_u2(rval);
                size_t idx = rval % table_size;
                rval = TbManUtility::xsg_u2(rval);
                size_t size = pow((double)max_alloc, rval * pow(2.0, -32));

                data_table[idx] = alloc(data_table[idx], size_table[idx], size, &size_table[idx]); // realloc
            }
        }

        time = clock() - time - overhead_time;
        size_t ns = (1E9 * time) / (CLOCKS_PER_SEC * cycles * table_size);
        printf("speed test realloc (general)   : %6zuns per call\n", ns);
    }

    // Local speed test: malloc, free
    {
        clock_t time = clock();

        for (size_t k = 0; k < cycles; k++)
        {
            size_t local_seed = (rval = TbManUtility::xsg_u2(rval));

            for (size_t j = 0; j < local_cycles; j++)
            {
                rval = local_seed;

                for (size_t i = 0; i < local_table_size; i++)
                {
                    rval = TbManUtility::xsg_u2(rval);
                    size_t idx = rval % table_size;
                    rval = TbManUtility::xsg_u2(rval);
                    size_t size = pow((double)max_alloc, rval * pow(2.0, -32));

                    if (data_table[idx] == NULL)
                    {
                        data_table[idx] = alloc(data_table[idx], 0, size, &size_table[idx]); // malloc
                    }
                    else
                    {
                        data_table[idx] = alloc(data_table[idx], size_table[idx], 0, &size_table[idx]); // free
                    }
                }
            }
        }

        time = clock() - time - local_overhead_time;
        size_t total_cycles = cycles * local_cycles * local_table_size;
        size_t ns = (1E9 * time) / (CLOCKS_PER_SEC * total_cycles);
        printf("speed test alloc-free (local)  : %6zuns per call\n", ns);
    }

    // cleanup
    if (cleanup)
    {
        for (size_t i = 0; i < table_size; i++)
        {
            data_table[i] = alloc(data_table[i], size_table[i], 0, NULL);
        }
    }

    free(size_table);
    free(data_table);
}

//================================================================================================================
//================================================================================================================

void Core::TbMemMan::eval()
{
    TbMemMan::open();
    {
        TbMemMan::test();
    }
    gDefaultTbman->close();
}

//================================================================================================================
//================================================================================================================

void Core::TbMemMan::down()
{
    size_t leaking_bytes = totalGrantedSpace();

    if (leaking_bytes > 0)
    {
        size_t leaking_instances = totalInstances();

        fprintf
        (
            stderr,
            "TBMAN WARNING: Detected %zu instances with a total of %zu bytes leaking space.\n",
            leaking_instances,
            leaking_bytes
        );
    }

    //pthread_mutex_lock( &mMutex );
    mtx_lock(&mMutex);
    
    if (mData)
    {
        for (size_t i = 0; i < mSize; i++)
        {
            TbBlockManager::discard(mData[i]);
        }

        memFree(mData);
    }

    mInternalBTree->discard();
    mExternalBTree->discard();

    if (mBlockSizeArray)
    {
        memFree(mBlockSizeArray);
    }

    //pthread_mutex_unlock( &mMutex );
    mtx_unlock(&mMutex);
    
    //pthread_mutex_destroy( &mMutex );
    mtx_destroy(&mMutex);
}

//================================================================================================================
//================================================================================================================

void Core::TbMemMan::close()
{
    discard(this);
}

//================================================================================================================
//================================================================================================================

#include <stdio.h>
#include <stdlib.h>

void* Core::TbMemMan::memAlloc(size_t requestedSize, size_t* grantedSize)
{
    TbBlockManager* blockManager = NULL;

    for (size_t i = 0; i < mSize; i++)
    {
        if (requestedSize <= mBlockSizeArray[i])
        {
            blockManager = mData[i];

            break;
        }
    }

    void* reservedPtr = NULL;

    if (blockManager)
    {
        reservedPtr = blockManager->alloc();
        
        if (grantedSize)
        {
            *grantedSize = blockManager->getBlockSize();
        }
    }
    else
    {
        reservedPtr = TbManUtility::aligned_alloc(TBMAN_ALIGN, requestedSize);
        
        if (!reservedPtr)
        {
            ERR("Failed allocating %zu bytes.", requestedSize);
        }
        
        if (grantedSize)
        {
            *grantedSize = requestedSize;
        }
        
        if (mExternalBTree->set(reservedPtr, requestedSize) != 1)
        {
            ERR("Registering new address failed");
        }
    }

    return reservedPtr;
}

//================================================================================================================
//================================================================================================================

void Core::TbMemMan::memFree(void* currentPtr, const size_t* currentSize)
{
    if (currentSize && *currentSize <= mMaxBlockSize && mAligned)
    {
        TbTokenManager* tokenManager = (TbTokenManager*)((intptr_t)currentPtr & ~(intptr_t)(mPoolSize - 1));

        tokenManager->free(currentPtr);
    }
    else
    {
        void* blockPtr = mInternalBTree->largestBelowEqual(currentPtr);

        if (blockPtr && (((uint8_t*)currentPtr - (uint8_t*)blockPtr) < mPoolSize))
        {
            ((TbTokenManager*)blockPtr)->free(currentPtr);
        }
        else
        {
            if (mExternalBTree->remove(currentPtr) != 1) ERR("Attempt to free invalid memory");
            
            //free( currentPtr );
            TbManUtility::aligned_free(currentPtr);
        }
    }
}

//================================================================================================================
//================================================================================================================

void* Core::TbMemMan::memRealloc(void* currentPtr, const size_t* currentSize, size_t requestedSize, size_t* grantedSize)
{
    TbTokenManager* tokenManager = NULL;

    if (currentSize && *currentSize <= mMaxBlockSize && mAligned)
    {
        tokenManager = (TbTokenManager*)((intptr_t)currentPtr & ~(intptr_t)(mPoolSize - 1));
    }
    else
    {
        void* blockPtr = mInternalBTree->largestBelowEqual(currentPtr);

        if (blockPtr && (((uint8_t*)currentPtr - (uint8_t*)blockPtr) < mPoolSize))
        {
            tokenManager = (TbTokenManager*)blockPtr;
        }
    }

    if (tokenManager)
    {
        if (requestedSize > tokenManager->getBlockSize())
        {
            void* reservedPtr = memAlloc(requestedSize, grantedSize);

            memcpy(reservedPtr, currentPtr, tokenManager->getBlockSize());
            
            tokenManager->free(currentPtr);
            
            return reservedPtr;
        }
        else // size reduction
        {
            TbBlockManager* blockManager = NULL;

            for (size_t i = 0; i < mSize; i++)
            {
                if (requestedSize <= mBlockSizeArray[i])
                {
                    blockManager = mData[i];

                    break;
                }
            }

            if (blockManager->getBlockSize() != tokenManager->getBlockSize())
            {
                void* reservedPtr = blockManager->alloc();

                memcpy(reservedPtr, currentPtr, requestedSize);
                
                tokenManager->free(currentPtr);
                
                if (grantedSize)
                {
                    *grantedSize = blockManager->getBlockSize();
                }
                
                return reservedPtr;
            }
            else
            {
                // same block-size: keep current location
                if (grantedSize)
                {
                    *grantedSize = tokenManager->getBlockSize();
                }

                return currentPtr;
            }
        }
    }
    else
    {
        if (requestedSize <= mMaxBlockSize) // new size fits into manager, old size was outside manager
        {
            void* reservedPtr = memAlloc(requestedSize, grantedSize);

            memcpy(reservedPtr, currentPtr, requestedSize);

            if (mExternalBTree->remove(currentPtr) != 1)
            {
                ERR("Attempt to free invalid memory");
            }
            
            //free( currentPtr );
            _aligned_free(currentPtr);
            
            return reservedPtr;
        }
        else // neither old nor new size handled by this manager
        {
            size_t* pCurrentSize = mExternalBTree->val(currentPtr);

            if (!pCurrentSize)
            {
                ERR("Could not retrieve current external memory");
            }
            
            size_t currentExtBytes = *pCurrentSize;

            // is requested bytes is less but not significantly less than current bytes, keep current memory
            if ((requestedSize < currentExtBytes) && (requestedSize >= (currentExtBytes >> 1)))
            {
                if (grantedSize)
                {
                    *grantedSize = currentExtBytes;
                }

                return currentPtr;
            }

            void* reservedPtr = _aligned_malloc(TBMAN_ALIGN, requestedSize);
            
            if (!reservedPtr)
            {
                ERR("Failed allocating %zu bytes.", requestedSize);
            }
            
            if (grantedSize)
            {
                *grantedSize = requestedSize;
            }
            
            if (mExternalBTree->set(reservedPtr, requestedSize) != 1)
            {
                ERR("Registering new address failed");
            }

            size_t copy_bytes = (requestedSize < currentExtBytes) ? requestedSize : currentExtBytes;
            
            memcpy(reservedPtr, currentPtr, copy_bytes);

            if (mExternalBTree->remove(currentPtr) != 1)
            {
                ERR("Attempt to free invalid memory");
            }

            //free( currentPtr );
            _aligned_free(currentPtr);
            
            return reservedPtr;
        }
    }
}

//================================================================================================================
//================================================================================================================

void* Core::TbMemMan::memAlloc(void* currentPtr, size_t requestedSize, size_t* grantedSize)
{
    //pthread_mutex_lock( &mMutex );
    mtx_lock(&mMutex);
    
    void* ret = NULL;
    
    if (requestedSize == 0)
    {
        if (currentPtr)
        {
            memFree(currentPtr, NULL);
        }

        if (grantedSize)
        {
            *grantedSize = 0;
        }
    }
    else
    {
        if (currentPtr)
        {
            ret = memRealloc(currentPtr, NULL, requestedSize, grantedSize);
        }
        else
        {
            ret = memAlloc(requestedSize, grantedSize);
        }

        if (!ret)
        {
            fprintf(stderr, "Failed allocating %zu bytes", requestedSize);
            abort();
        }
    }
    
    //pthread_mutex_unlock( &mMutex );
    mtx_unlock(&mMutex);
    
    return ret;
}

//================================================================================================================
//================================================================================================================

void* Core::TbMemMan::memMalloc(size_t size)
{
    return memAlloc(NULL, size, NULL);
}

//================================================================================================================
//================================================================================================================

void* Core::TbMemMan::memRealloc(void* ptr, size_t size)
{
    return memAlloc(ptr, size, NULL);
}

//================================================================================================================
//================================================================================================================

void Core::TbMemMan::memFree(void* ptr)
{
    memAlloc(ptr, 0, NULL);
}

//================================================================================================================
//================================================================================================================

void* Core::TbMemMan::memNrealloc(void* currentPtr, size_t currentSize, size_t newSize)
{
    return Core::TbMemMan::memNalloc(currentPtr, currentSize, newSize, NULL);
}

//================================================================================================================
//================================================================================================================

void Core::TbMemMan::memNfree(void* currentPtr, size_t currentSize)
{
    Core::TbMemMan::memNalloc(currentPtr, currentSize, 0, NULL);
}

//================================================================================================================
//================================================================================================================

size_t Core::TbMemMan::grantedSpace(const void* currentPtr)
{
    TbTokenManager* tokenManager = NULL;
    {
        void* blockPtr = mInternalBTree->largestBelowEqual((void*)currentPtr);
        
        if (blockPtr && (((uint8_t*)currentPtr - (uint8_t*)blockPtr) < mPoolSize))
        {
            tokenManager = (TbTokenManager*)blockPtr;
        }
    }

    if (tokenManager)
    {
        return tokenManager->getBlockSize();
    }
    else
    {
        size_t* pCurrentSize = mExternalBTree->val((void*)currentPtr);
        
        if (!pCurrentSize)
        {
            return 0;
        }
        
        return *pCurrentSize;
    }
}

//================================================================================================================
//================================================================================================================

size_t Core::TbMemMan::totalGrantedSpace()
{
    //pthread_mutex_lock( &mMutex );
    mtx_lock(&mMutex);

    size_t space = totalAlloc();
    
    //pthread_mutex_unlock( &mMutex );
    mtx_unlock(&mMutex);
    
    return space;
}

//================================================================================================================
//================================================================================================================

size_t Core::TbMemMan::totalInstances()
{
    //pthread_mutex_lock( &mMutex );
    mtx_lock(&mMutex);

    size_t count = 0;
    count += externalTotalInstances();
    count += internalTotalInstances();
    
    //pthread_mutex_unlock( &mMutex );
    mtx_unlock(&mMutex);
    
    return count;
}

//================================================================================================================
//================================================================================================================

void Core::TbMemMan::forEachInstance(void_callback1 cb, void* arg)
{
    if (!cb)
    {
        return;
    }
    
    size_t size = totalInstances();

    if (!size)
    {
        return;
    }

    mnode_arr arr;
    arr.data = (mnode*)malloc(sizeof(mnode) * size);
    arr.space = size;
    arr.size = 0;

    //pthread_mutex_lock( &mMutex );
    mtx_lock(&mMutex);

    externalForEachInstance(forEachInstanceCollectCallback, &arr);
    internalForEachInstance(forEachInstanceCollectCallback, &arr);
    
    //pthread_mutex_unlock( &mMutex );
    mtx_unlock(&mMutex);

    assert(arr.size == arr.space);

    for (size_t i = 0; i < size; i++)
    {
        cb(arg, arr.data[i].p, arr.data[i].s);
    }

    free(arr.data);
}

//================================================================================================================
//================================================================================================================

void Core::TbMemMan::printStatus(int detail_level)
{
    if (detail_level <= 0) return;

    printf("pool_size:              %zu\n", mPoolSize);
    printf("block managers:         %zu\n", mSize);
    printf("token managers:         %zu\n", mInternalBTree->count(NULL, NULL));
    printf("external allocs:        %zu\n", mExternalBTree->count(NULL, NULL));
    printf("internal_btree depth:   %zu\n", mInternalBTree->depth());
    printf("external_btree depth:   %zu\n", mExternalBTree->depth());
    printf("min_block_size:         %zu\n", mSize > 0 ? mData[0]->getBlockSize() : 0);
    printf("max_block_size:         %zu\n", mSize > 0 ? mData[mSize - 1]->getBlockSize() : 0);
    printf("aligned:                %s\n", mAligned ? "true" : "false");
    printf("total external granted: %zu\n", externalTotalAlloc());
    printf("total internal granted: %zu\n", internalTotalAlloc());
    printf("total internal used:    %zu\n", totalSpace());

    if (detail_level > 1)
    {
        for (size_t i = 0; i < mSize; i++)
        {
            printf("\nblock manager %zu:\n", i);

            mData[i]->printStatus(detail_level - 1);
        }
    }
}

//================================================================================================================
//================================================================================================================

void Core::TbMemMan::lostAlignment()
{
    mAligned = false;
}

//================================================================================================================
//================================================================================================================

size_t Core::TbMemMan::externalTotalAlloc()
{
    return mExternalBTree->sum(NULL, NULL);
}

//================================================================================================================
//================================================================================================================

size_t Core::TbMemMan::externalTotalInstances()
{
    size_t size = 0;

    mExternalBTree->run(ext_count, &size);
    
    return size;
}

//================================================================================================================
//================================================================================================================

void Core::TbMemMan::externalForEachInstance(void_callback1 cb, void* arg)
{
    ext_for_instance_arg iarg;// = { .cb = cb, .arg = arg };
    iarg.cb = cb;
    iarg.arg = arg;
    
    mExternalBTree->run(ext_for_instance, &iarg);
}

//================================================================================================================
//================================================================================================================

size_t Core::TbMemMan::internalTotalAlloc()
{
    size_t sum = 0;

    for (size_t i = 0; i < mSize; i++)
    {
        sum += mData[i]->totalAlloc();
    }
    
    return sum;
}

//================================================================================================================
//================================================================================================================

size_t Core::TbMemMan::internalTotalInstances()
{
    size_t sum = 0;
    
    for (size_t i = 0; i < mSize; i++)
    {
        sum += mData[i]->totalInstances();
    }
    
    return sum;
}

//================================================================================================================
//================================================================================================================

void Core::TbMemMan::internalForEachInstance(void_callback1 cb, void* arg)
{
    for (size_t i = 0; i < mSize; i++)
    {
        mData[i]->forEachInstance(cb, arg);
    }
}

//================================================================================================================
//================================================================================================================

size_t Core::TbMemMan::totalAlloc()
{
    return externalTotalAlloc() + totalAlloc();
}

//================================================================================================================
//================================================================================================================

size_t Core::TbMemMan::totalSpace()
{
    size_t sum = 0;

    for (size_t i = 0; i < mSize; i++)
    {
        sum += mData[i]->totalSpace();
    }
    
    return sum;
}

//================================================================================================================
//================================================================================================================