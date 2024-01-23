#include "TbTokenManager.h"
#include "TbBlockManager.h"

//================================================================================================================
//================================================================================================================

void Core::TbTokenManager::init()
{
	// This original code did a memset
	/*Core::TbManUtility::reset(mPoolSize);
	Core::TbManUtility::reset(mBlockSize);
	Core::TbManUtility::reset(mStackSize);
	Core::TbManUtility::reset(mStackIndex);
	Core::TbManUtility::reset(mAligned);
	Core::TbManUtility::reset(mParent);
	Core::TbManUtility::reset(mParentIndex);
	Core::TbManUtility::reset(mTokenStack);*/

    mPoolSize = 0;
    mBlockSize = 0;
    mStackSize = 0;
    mStackIndex = 0;
    mAligned = false;
    mParent = NULL;
    mParentIndex = 0;

    memset(&mTokenStack, 0, sizeof(*mTokenStack));
}

//================================================================================================================
//================================================================================================================

void Core::TbTokenManager::down()
{
	// This method is empty
}

//================================================================================================================
//================================================================================================================

Core::TbTokenManager* Core::TbTokenManager::create(size_t pool_size, size_t block_size, bool align)
{
	if ((pool_size & (pool_size - 1)) != 0)
	{
		ERR("pool_size %zu is not a power of two", pool_size);
	}

    size_t stack_size = pool_size / block_size;

    if (stack_size > 0x10000)
    {
        ERR("stack_size %zu exceeds 0x10000", stack_size);
    }
    
    size_t reserved_size = sizeof(Core::TbTokenManager) + sizeof(uint16_t) * stack_size;
    size_t reserved_blocks = reserved_size / block_size + ((reserved_size % block_size) > 0);

    if (stack_size < (reserved_blocks + 1))
    {
        ERR("pool_size %zu is too small", pool_size);
    }

    Core::TbTokenManager* o;
    if (align)
    {
        o = (Core::TbTokenManager*)_aligned_malloc(pool_size, pool_size);

        if (!o)
        {
            ERR("Failed aligned allocating %zu bytes", pool_size);
        }
    }
    else
    {
        o = (Core::TbTokenManager*)_aligned_malloc(TBMAN_ALIGN, pool_size);

        if (!o)
        {
            ERR("Failed allocating %zu bytes", pool_size);
        }
    }

    o->init();
    bool oAligned = ((intptr_t)o & (intptr_t)(pool_size - 1)) == 0;
    o->setIsAligned(oAligned);
    o->setPoolSize(pool_size);
    o->setBlockSize(block_size);
    o->setStackSize(stack_size);
    o->setStackIndex(0);
    
    for (size_t i = 0; i < stack_size; i++)
    {
        uint16_t ts = (i + reserved_blocks) < stack_size ? (i + reserved_blocks) : 0;
        o->setTokenStack(ts, i);
    }

    return o;
}

//================================================================================================================
//================================================================================================================

void Core::TbTokenManager::discard(Core::TbTokenManager* tm)
{
    if (!tm) return;

    tm->down();

    //free(tm);
    _aligned_free(tm);
}

//================================================================================================================
//================================================================================================================

bool Core::TbTokenManager::isFull()
{
    return mTokenStack[mStackIndex] == 0;
}

//================================================================================================================
//================================================================================================================

bool Core::TbTokenManager::isEmpty()
{
    return mStackIndex == 0;
}

//================================================================================================================
//================================================================================================================

void* Core::TbTokenManager::alloc()
{
    assert(!isFull());
    
    void *ret = (uint8_t*)this + mTokenStack[mStackIndex] * mBlockSize;

    assert((uint8_t*)ret >= (uint8_t*)this + sizeof(Core::TbTokenManager));

    mStackIndex++;

    return ret;
}

//================================================================================================================
//================================================================================================================

void Core::TbTokenManager::free(void* ptr)
{
#ifdef RTCHECKS
    if (mStackIndex == 0) ERR("Block manager is empty.");
    if ((size_t)((ptrdiff_t)((uint8_t*)ptr - (uint8_t*)this)) > mPoolSize) ERR("Attempt to free memory outside pool.");
#endif

    uint16_t token = ((ptrdiff_t)((uint8_t*)ptr - (uint8_t*)this)) / mBlockSize;

#ifdef RTCHECKS
    if (token * mBlockSize < sizeof(Core::TbTokenManager)) ERR("Attempt to free reserved memory.");
    for (size_t i = mStackIndex; i < mStackSize; i++) if (mTokenStack[i] == token) ERR("Attempt to free memory that is declared free.");
#endif // RTCHECKS

    if (mTokenStack[mStackIndex] == 0)
    {
        mParent->fullToFree(this);
    }

    mStackIndex--;

    mTokenStack[mStackIndex] = token;

    if (mStackIndex == 0)
    {
        mParent->freeToEmpty(this);
    }
}

//================================================================================================================
//================================================================================================================

size_t Core::TbTokenManager::totalAlloc()
{
    return mBlockSize * mStackIndex;
}

//================================================================================================================
//================================================================================================================

size_t Core::TbTokenManager::totalInstances()
{
    return mStackIndex;
}

//================================================================================================================
//================================================================================================================

size_t Core::TbTokenManager::totalSpace()
{
    return mPoolSize + mStackSize * sizeof(uint16_t);
}

//================================================================================================================
//================================================================================================================

void Core::TbTokenManager::forEachInstance(void_callback1 cb, void* arg)
{
    if (!cb) return;

    for (size_t i = 0; i < mStackIndex; i++)
    {
        size_t token = mTokenStack[i];

        cb(arg, (uint8_t*)this + token * mBlockSize, mBlockSize);
    }
}

//================================================================================================================
//================================================================================================================

void Core::TbTokenManager::printStatus(int detail_level)
{
    if (detail_level <= 0) return;

    printf("    pool_size:   %zu\n", mPoolSize);
    printf("    block_size:  %zu\n", mBlockSize);
    printf("    stack_size:  %u\n", mStackSize);
    printf("    aligned:     %s\n", mAligned ? "true" : "false");
    printf("    stack_index: %zu\n", (size_t)mStackIndex);
    printf("    total alloc: %zu\n", totalAlloc());
    printf("    total space: %zu\n", totalSpace());
}

//================================================================================================================
//================================================================================================================