#ifndef _TSLIST_H_
#define _TSLIST_H_

#include <assert.h>
#include "platform.h"

#pragma warning(push)
#pragma warning(disable: 4201) // warning C4201: nonstandard extension used : nameless struct/union <-- nice, but anonymous structs/unions make my code look prettier so it's staying
#pragma warning(disable: 4324) // warning C4324: 'TSLNode' : structure was padded due to __declspec(align()) <-- sorta why I put it in there? 

#if PLATFORM_32_BIT
struct _TSLNode32;
struct _TSLHead32;
class _TSListBase32;
typedef _TSLNode32 TSLNode;
typedef _TSLHead32 TSLHead;
typedef _TSListBase32 TSListBase;
#elif PLATFORM_64_BIT
struct _TSLNode64;
struct _TSLHead64;
class _TSListBase64;
typedef _TSLNode64 TSLNode;
typedef _TSLHead64 TSLHead;
typedef _TSListBase64 TSListBase;
#endif


#if PLATFORM_32_BIT
struct DECLSPEC_ALIGN(PLATFORM_MEMORY_ALIGNMENT) _TSLNode32
{
	TSLNode* Next;
	_TSLNode32()
	{
		Next = nullptr;
	};
};

struct DECLSPEC_ALIGN(PLATFORM_MEMORY_ALIGNMENT) _TSLHead32
{
	union
	{
		struct
		{
			TSLNode*	Next;
			int16		Depth;
			int16		Sequence;  
		};
		int64			Value;
	};
	_TSLHead32()
	{
		Value = 0;
		assert(((size_t) this % PLATFORM_MEMORY_ALIGNMENT) == 0); // make sure we are aligned!
	};
};

class _TSListBase32
{
	TSLHead head;
public:
	_TSListBase32(): head()
	{

	};

	~_TSListBase32()
	{

	};

	int32 Count()
	{
		return head.Depth;
	};

	TSLNode* Push(TSLNode* node)
	{
		TSLHead old_head;
		TSLHead new_head;

		for (;;)
		{
			old_head.Value = head.Value;
			node->Next = old_head.Next;
			new_head.Next = node;
			*((unsigned long *)&new_head.Depth) = *((unsigned long *)&old_head.Depth) + 0x10001; // increment depth + sequence at the same time
			
			if (_InterlockedCompareExchange64(&head.Value, new_head.Value, old_head.Value) == old_head.Value) 
			{
				break;
			};

			YieldThread();
		};

		return old_head.Next;
	};

	TSLNode* Pop()
	{
		TSLHead old_head;
		TSLHead new_head;

		for (;;)
		{
			old_head.Value = head.Value;
			if (!old_head.Next) 
			{
				return nullptr;
			}

			new_head.Next = old_head.Next->Next;
			*((unsigned long *)&new_head.Depth) = *((unsigned long *)&old_head.Depth) - 1;
			
			if (_InterlockedCompareExchange64(&head.Value, new_head.Value, old_head.Value) == old_head.Value) 
			{
				break;
			};

			YieldThread();
		};

		return old_head.Next;
	};

	TSLNode* Flush() // aka destroy all humans, err I mean pop all items
	{
		TSLHead old_head;
		TSLHead new_head;

		for (;;)
		{
			old_head.Value = head.Value;
			if (!old_head.Next) 
			{
				return nullptr;
			}
			new_head.Next = nullptr;
			*((unsigned long *)&new_head.Depth) = *((unsigned long *)&old_head.Depth) - 1;
			
			if (_InterlockedCompareExchange64(&head.Value, new_head.Value, old_head.Value) == old_head.Value) 
			{
				break;
			};

			YieldThread();
		};
		return old_head.Next;
	};

	void ResetHead()
	{
		head = TSLHead();
	}
};

#elif PLATFORM_64_BIT

struct DECLSPEC_ALIGN(PLATFORM_MEMORY_ALIGNMENT) _TSLNode64
{
	TSLNode* Next;
	TSLNode()
	{
		Next = 0;
	};
};

struct DECLSPEC_ALIGN(PLATFORM_MEMORY_ALIGNMENT) _TSLHead64
{
	union
	{
		struct
		{
			TSLNode*	Next;
			int32		Depth;
			int32		Sequence;
		};
		int64			Value[2];
	};
	TSLHead()
	{
		Value[0] = 0;
		Value[1] = 0;
		assert(((size_t) this % PLATFORM_MEMORY_ALIGNMENT) == 0); // make sure we are aligned!
	};
};

class _TSListBase64
{
private:
	TSLHead head;
public:
	_TSListBase64(): head()
	{

	};

	~_TSListBase64()
	{

	};

	int32 Count()
	{
		return head.Depth;
	};

	TSLNode* Push(TSLNode* node)
	{
		TSLHead old_head;
		TSLHead new_head;

		for (;;)
		{
			old_head.Value[0] = head.Value[0];
			old_head.Value[1] = head.Value[1];
			node->Next = old_head.Next;
			new_head.Next = node;
			*((int64 *)&new_head.Depth) = *((int64 *)&old_head.Depth) + 0x100000001; // increment depth + sequence at the same time
			
			if (_InterlockedCompareExchange128(&head.Value[0], new_head.Value[1], new_head.Value[0], old_head.Value))
			{
				break;
			};

			YieldThread();
		};

		return old_head.Next;
	};

	TSLNode* Pop()
	{
		TSLHead old_head;
		TSLHead new_head;

		for (;;)
		{
			old_head.Value[0] = head.Value[0];
			old_head.Value[1] = head.Value[1];
			if (!old_head.Next) 
			{
				return nullptr;
			}

			new_head.Next = old_head.Next->Next;
			*((int64 *)&new_head.Depth) = *((int64 *)&old_head.Depth) - 1;
			
			if (_InterlockedCompareExchange128(&head.Value[0], new_head.Value[1], new_head.Value[0], old_head.Value))
			{
				break;
			};

			YieldThread();
		};

		return old_head.Next;
	};

	TSLNode* Flush() // aka destroy all humans, err I mean pop all items
	{
		TSLHead old_head;
		TSLHead new_head;

		for (;;)
		{
			old_head.Value[0] = head.Value[0];
			old_head.Value[1] = head.Value[1];
			if (!old_head.Next) 
			{
				return nullptr;
			}

			new_head.Next = nullptr;
			*((int64 *)&new_head.Depth) = (*((int64 *)&old_head.Depth) - 0x100000000) & 0xffffffff00000000;
			
			if (_InterlockedCompareExchange128(&head.Value[0], new_head.Value[1], new_head.Value[0], old_head.Value))
			{
				break;
			};

			YieldThread();
		};

		return old_head.Next;
	};

	void ResetHead()
	{
		head = TSLHead();
	}
};
#endif


template <typename T> class TSList: public TSListBase
{
public:
	void Push(T* item)
	{
		assert(sizeof(T) >= sizeof(TSLNode));
		TSListBase::Push((TSLNode*) item); 
	};
	
	T* Pop()
	{
		return (T*) TSListBase::Pop();
	};

	T* Flush()
	{
		return (T*) TSListBase::Flush();
	};
};

#pragma warning(pop)

#endif
