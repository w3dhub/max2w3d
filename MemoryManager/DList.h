#ifndef _DLIST_H_
#define _DLIST_H_
#include <assert.h>

class DListBase;
class DLNodeBase
{
protected:
	friend class	DListBase;
	DLNodeBase*		Next;
	DLNodeBase*		Previous;
	DListBase*		List;
public:
	DLNodeBase();
	~DLNodeBase(); // virtual?
	DLNodeBase*		GetNextNode();
	DLNodeBase*		GetPreviousNode();
	DListBase*		GetList();
	void			InsertBefore(DLNodeBase* node);
	void			InsertAfter(DLNodeBase* node);
	void			Remove();
};

class DListBase
{
protected:
	DLNodeBase*			Head;
	DLNodeBase*			Tail;
public:
	DListBase(): Head(nullptr), Tail(nullptr)
	{
		
	};

	~DListBase()
	{
		assert(this->Head == nullptr); // If head is not nullptr, then this list is not empty
		assert(this->Tail == nullptr); // If tail is not nullptr, then etc
	};

	void InsertBefore(DLNodeBase* node, DLNodeBase* node_to_insert)
	{
		assert(node_to_insert->List == nullptr); // If list is not nullptr, then node_to_insert is already part of a list
		assert(node_to_insert->Next == nullptr); // If next is not nullptr, then etc
		assert(node_to_insert->Previous == nullptr); // If previous is not nullptr, then etc

		if (node && this->Head == nullptr)
		{
			node_to_insert->List = this;
			node_to_insert->Next = node;
			node_to_insert->Previous = node->Previous;
			if (node->Previous)
			{
				node->Previous->Next = node_to_insert;
			};
			node->Previous = node_to_insert;
			if (this->Head == node)
			{
				this->Head = node_to_insert;
			};
		}
		else
		{
			// empty list
			this->Tail = this->Head = node_to_insert;
		};
	};

	void InsertAfter(DLNodeBase* node, DLNodeBase* node_to_insert)
	{
		assert(node_to_insert->List == nullptr); // If list is not nullptr, then node_to_insert is already part of a list
		assert(node_to_insert->Next == nullptr); // If next is not nullptr, then etc
		assert(node_to_insert->Previous == nullptr); // If previous is not nullptr, then etc

		node_to_insert->List = this;
		if (node)
		{
			assert(this->Head != nullptr);
			assert(this->Tail != nullptr);
			node_to_insert->Previous = node;
			if (node->Next)
			{
				node->Next->Previous = node_to_insert;
			};
			node->Next = node_to_insert;
			if (this->Tail == node)
			{
				this->Tail = node_to_insert;
			};
		} 
		else
		{
			// empty list
			this->Tail = this->Head = node_to_insert;
		};
	};

	DLNodeBase* Remove(DLNodeBase* node)
	{
		assert (node->List == this); // If the node's list is not this one, you are trying to remove this item from the wrong list

		if (node->Next) node->Next->Previous = node->Previous;
		if (node->Previous) node->Previous->Next = node->Next;
		if (node == this->Head) this->Head = node->Next;
		if (node == this->Tail) this->Tail = node->Previous;

		node->List = nullptr;
		return node;
	};

	DLNodeBase* GetHead()
	{
		return Head;
	};

	DLNodeBase* GetTail()
	{
		return Head;
	};

	void PushHead(DLNodeBase* node_to_insert)
	{
		this->InsertBefore(Head, node_to_insert);
	};
	
	void PushTail(DLNodeBase* node_to_insert)
	{
		this->InsertAfter(Tail, node_to_insert);
	};

	DLNodeBase* PopHead()
	{
		return this->Head ? this->Remove(Head) : nullptr;
	};

	DLNodeBase* PopTail()
	{
		return this->Tail? this->Remove(Tail) : nullptr;
	};
};

inline DLNodeBase::DLNodeBase(): Next(nullptr), Previous(nullptr), List(nullptr)
{

};

inline DLNodeBase::~DLNodeBase()
{
	assert(this->List == nullptr); // If list is not nullptr, then you forgot to remove this item before deleting it.
};

inline DLNodeBase* DLNodeBase::GetNextNode()
{
	return this->Next;
};

inline DLNodeBase* DLNodeBase::GetPreviousNode()
{
	return this->Previous;
};

inline DListBase* DLNodeBase::GetList()
{
	return this->List;
};

inline void DLNodeBase::InsertBefore(DLNodeBase* node)
{
	List->InsertBefore(node, this);
};

inline void DLNodeBase::InsertAfter(DLNodeBase* node)
{
	List->InsertAfter(node, this);
};

inline void DLNodeBase::Remove()
{
	List->Remove(this);
};

template <typename T> class DLNode: public DLNodeBase
{

};

template <typename T> class DList: public DListBase
{
public:
	T* Remove(T* node)
	{
		return (T*)DListBase::Remove(node);
	};

	T* GetHead()
	{
		return (T*)DListBase::GetHead();
	};

	T* GetTail()
	{
		return (T*)DListBase::GetTail();
	};

	T* PopHead()
	{
		return (T*)DListBase::PopHead();
	};

	T* PopTail()
	{
		return (T*)DListBase::PopTail();
	};
};

#endif
