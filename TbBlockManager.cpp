#include "TbBlockManager.h"

//================================================================================================================
//================================================================================================================

void Core::TbBlockManager::init()
{
	// This original code did a memset
	/*Core::TbManUtility::reset(mPoolSize);
	Core::TbManUtility::reset(mBlockSize);
	Core::TbManUtility::reset(mAlign);
	Core::TbManUtility::reset(mData);
	Core::TbManUtility::reset(mSize);
	Core::TbManUtility::reset(mSpace);
	Core::TbManUtility::reset(mFreeIndex);
	Core::TbManUtility::reset(mSweepHysteresis);
	Core::TbManUtility::reset(mAligned);
	Core::TbManUtility::reset(mParent);
	Core::TbManUtility::reset(mInternalBTree);*/

	mAligned = true;
	mSweepHysteresis = 0.125;

	mPoolSize = 0;
	mBlockSize = 0;
	mAlign = false;
	mData = NULL;
	mSize = 0;
	mSpace = 0;
	mFreeIndex = 0;
	mParent = NULL;
	mInternalBTree = NULL;
}

//================================================================================================================
//================================================================================================================

void Core::TbBlockManager::down()
{
	if (mData)
	{
		for (size_t i = 0; i < mSize; i++)
		{
			TbTokenManager::discard(mData[i]);
		}

		//free(mData);
		_aligned_free(mData);

		mData = NULL;
		
		mSize = mSpace = 0;
	}
}

//================================================================================================================
//================================================================================================================

Core::TbBlockManager* Core::TbBlockManager::create(size_t pool_size, size_t block_size, bool align)
{
	Core::TbBlockManager* o = (Core::TbBlockManager*) malloc(sizeof(Core::TbBlockManager));

	if (!o) ERR("Failed allocating %zu bytes", sizeof(Core::TbBlockManager));
	
	o->init();
	o->setPoolSize(pool_size);
	o->setBlockSize(block_size);
	o->setAlign(align);
	
	return o;
}

//================================================================================================================
//================================================================================================================

void Core::TbBlockManager::discard(TbBlockManager* bm)
{
	if (!bm) return;

	bm->down();

	//free(bm);
	_aligned_free(bm);
}

//================================================================================================================
//================================================================================================================

void* Core::TbBlockManager::alloc()
{
	if (mFreeIndex == mSize)
	{
		if (mSize == mSpace)
		{
			mSpace = (mSpace > 0) ? mSpace * 2 : 1;

			if (mData)
			{
				TbTokenManager** data = (TbTokenManager**) realloc(mData, sizeof(TbTokenManager*) * mSpace);

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
				mData = (TbTokenManager**) malloc(sizeof(TbTokenManager*) * mSpace);
			}

			if (!mData)
			{
				ERR("Failed allocating %zu bytes", sizeof(TbTokenManager*) * mSpace);
			}
		}
		
		mData[mSize] = TbTokenManager::create(mPoolSize, mBlockSize, mAlign);
		mData[mSize]->setParentIndex(mSize);
		mData[mSize]->setParent(this);
		
		if (mAligned && !mData[mSize]->isAligned())
		{
			mAligned = false;

			mParent->lostAlignment();
		}
		
		if (mInternalBTree->set(mData[mSize]) != 1)
		{
			ERR("Failed registering block address.");
		}
		
		mSize++;
	}

	TbTokenManager* child = mData[mFreeIndex];

	void* ret = child->alloc();

	if (child->isFull())
	{
		mFreeIndex++;
	}
	
	return ret;
}

//================================================================================================================
//================================================================================================================

void Core::TbBlockManager::fullToFree(TbTokenManager* child)
{
	assert(mFreeIndex > 0);

	mFreeIndex--;

	// swap child with current free position
	size_t child_index = child->getParentIndex();
	size_t swapc_index = mFreeIndex;

	TbTokenManager* swapc = mData[swapc_index];
	mData[swapc_index] = child;
	mData[child_index] = swapc;
	child->setParentIndex(swapc_index);
	swapc->setParentIndex(child_index);
}

//================================================================================================================
//================================================================================================================

size_t Core::TbBlockManager::emptyTail()
{
	if (mSize == 0) return 0;
	
	size_t empty_index = mSize;
	
	while (empty_index > 0 && mData[empty_index - 1]->isEmpty())
	{
		empty_index--;
	}
	
	return mSize - empty_index;
}

//================================================================================================================
//================================================================================================================

void Core::TbBlockManager::freeToEmpty(TbTokenManager* child)
{
	// move empty manager to tail (if not already there)
	size_t child_index = child->getParentIndex();
	size_t empty_tail = emptyTail();

	if (empty_tail < mSize)
	{
		size_t swapc_index = mSize - empty_tail - 1;
		if (child_index < swapc_index)
		{
			TbTokenManager* swapc = mData[swapc_index];
			mData[child_index] = swapc;
			mData[swapc_index] = child;
			child->setParentIndex(swapc_index);
			swapc->setParentIndex(child_index);
			empty_tail++;
		}
	}

	if (empty_tail > (mSize - empty_tail) * mSweepHysteresis) // discard empty managers when enough accumulated
	{
		while (mSize > 0 && mData[mSize - 1]->isEmpty())
		{
			mSize--;

			if (mInternalBTree->remove(mData[mSize]) != 1)
			{
				ERR("Failed removing block address.");
			}

#ifdef RTCHECKS
			if (mInternalBTree->exists(mData[mSize]))
			{
				ERR("Removed block address still exists");
			}
#endif

			TbTokenManager::discard(mData[mSize]);

			mData[mSize] = NULL;
		}
	}
}

//================================================================================================================
//================================================================================================================

size_t Core::TbBlockManager::totalAlloc()
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

size_t Core::TbBlockManager::totalInstances()
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

size_t Core::TbBlockManager::totalSpace()
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

void Core::TbBlockManager::forEachInstance(void_callback1 cb, void* arg)
{
	for (size_t i = 0; i < mSize; i++)
	{
		mData[i]->forEachInstance(cb, arg);
	}
}

//================================================================================================================
//================================================================================================================

void Core::TbBlockManager::printStatus(int detail_level)
{
	if (detail_level <= 0) return;

	printf("  pool_size:        %zu\n", mPoolSize);
	printf("  block_size:       %zu\n", mBlockSize);
	printf("  sweep_hysteresis: %g\n", mSweepHysteresis);
	printf("  aligned:          %s\n", mAligned ? "true" : "false");
	printf("  token_managers:   %zu\n", mSize);
	printf("      full:         %zu\n", mFreeIndex);
	printf("      empty:        %zu\n", emptyTail());
	printf("  total alloc:      %zu\n", totalAlloc());
	printf("  total space:      %zu\n", totalSpace());
	
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