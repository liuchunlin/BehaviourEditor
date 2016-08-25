//
#pragma once

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif


class BlockAllocator
{
public:
	/**
	* Ctor
	* 
	* @param	InitialBlockSize		- number of allocations to warm the cache with
	*/
	BlockAllocator(DWORD nBlockSize, DWORD nBlocksPerBatch = 64, DWORD nInitialBatches = 0, const wstring& kDebugAllocTypeName = wstring(_T("")))
		:FreeList(NULL)
		,NumAllocated(0)
		,NumLive(0)
		,BlocksPerBatch(nBlocksPerBatch)
		,DebugAllocTypeName(kDebugAllocTypeName)
	{
		// need enough memory to store pointers for the free list
		BlockSize = max(nBlockSize, (DWORD)sizeof(FreeListNode));

		// warm the cache
		Grow(nInitialBatches);
	}

	/**
	* Destructor. Can't free memory, so only checks that allocations have been returned.
	*/
	~BlockAllocator()
	{
		// by now all block better have been returned to me
		assert(NumLive == 0); 
		// WRH - 2007/09/14 - Note we are stranding memory here.
		// These pools are meant to be global and never deleted. 

		// Chain the block into a list of free list nodes
		
		//modify-begin
		vector<BYTE*>::iterator iter(toFrees.begin());
		for( ; iter != toFrees.end(); ++iter )
		{
			free( (*iter) );
		}
		//modify-end
	}

	/**
	* Allocates one element from the free list. Return it by calling Free.
	*/
	void *Allocate()
	{
		CheckInvariants();
		if (!FreeList)
		{
			assert(NumLive == NumAllocated);
			Grow(BlocksPerBatch);
		}
		// grab next free element, updating FreeList pointer
		void *rawMem = (void *)FreeList;
		FreeList = FreeList->NextFreeAllocation;
		++NumLive;
		CheckInvariants();
		return rawMem;
	}

	/**
	* Returns one element from the free list.
	* Must have been acquired previously by Allocate.
	*/
	void Free(void *Element)
	{
		CheckInvariants();
		--NumLive;
		FreeListNode* NewFreeElement = (FreeListNode*)Element;
		NewFreeElement->NextFreeAllocation = FreeList;
		FreeList = NewFreeElement;
		CheckInvariants();
	}

	/**
	* Get total memory allocated
	*/
	DWORD GetAllocatedSize() const
	{
		CheckInvariants();
		return NumAllocated * BlockSize;
	}

	/**
	* Get size of one allocation
	*/
	DWORD GetBlockSize()
	{
		return BlockSize;
	}

	/**
	* Grows the free list by a specific number of elements. Does one allocation for all elements.
	* Safe to call at any time to warm the cache with a single block allocation.
	*/
	void Grow(DWORD NumElements)
	{
		if (NumElements == 0) return;
		// need enough memory to store pointers for the free list
		assert(BlockSize*NumElements >= sizeof(FreeListNode));

		// allocate a block of memory
		BYTE* RawMem = (BYTE*)malloc(BlockSize * NumElements);
		//modify-begin
		toFrees.push_back( RawMem );
		//modify-end
		FreeListNode* NewFreeList = (FreeListNode*)RawMem;

		// Chain the block into a list of free list nodes
		for (UINT i=0;i<NumElements-1;++i,NewFreeList=NewFreeList->NextFreeAllocation)
		{
			NewFreeList->NextFreeAllocation = (FreeListNode*)(RawMem + (i+1)*BlockSize);
		}

		// link the last Node to the previous FreeList
		NewFreeList->NextFreeAllocation = FreeList;
		FreeList = (FreeListNode*)RawMem;

		NumAllocated += NumElements;
	}

private:
	struct FreeListNode
	{
		FreeListNode* NextFreeAllocation;
	};

	void CheckInvariants() const
	{
		assert(NumAllocated >= NumLive);
	}

	/** Linked List of free memory blocks. */
	FreeListNode* FreeList;
	/** The number of objects that have been allocated. */
	DWORD NumAllocated;
	/** The number of objects that are constructed and "out in the wild". */
	DWORD NumLive;
	/** Fixed size of one allocation. */
	DWORD BlockSize;
	/** number of blocks to allocate at one time when free memory runs out. */
	DWORD BlocksPerBatch;
	/** The data struct type we are allocating for, for debug use only. */
	wstring DebugAllocTypeName;
	//modify-begin
	vector<BYTE*>	toFrees;
	//modify-end
};

#define USE_BLOCK_ALLOCATOR 1

#if USE_BLOCK_ALLOCATOR	
#define DECLARE_BLOCK_ALLOCATE \
	public: \
	void* operator new(size_t sz); \
	void operator delete(void* pBlock); \
	private: \
	static BlockAllocator s_BlockAllocator; 
#else
#define DECLARE_BLOCK_ALLOCATE 
#endif

#if USE_BLOCK_ALLOCATOR
#define IMPLEMENT_BLOCK_ALLOCATE(Class, BlocksPerBatch) \
	BlockAllocator Class::s_BlockAllocator(sizeof(Class), BlocksPerBatch, 0, TEXT(#Class)); \
	void* Class::operator new(size_t sz) \
	{ \
		assert(s_BlockAllocator.GetBlockSize() >= sz); \
		return s_BlockAllocator.Allocate(); \
	} \
	void Class::operator delete(void* pBlock) \
	{ \
		s_BlockAllocator.Free(pBlock); \
	} 
#else
#define IMPLEMENT_BLOCK_ALLOCATE(Class, BlocksPerBatch)
#endif

