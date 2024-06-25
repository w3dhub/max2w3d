#pragma once
class GenericList;
class GenericNode {
public:
	GenericNode(void) : NextNode(nullptr), PrevNode(nullptr) {}
	virtual ~GenericNode(void) { Unlink(); }
	GenericNode(GenericNode & node) { node.Link(this); }
	GenericNode & operator = (GenericNode & node)
	{
		if (&node != this)
		{
			node.Link(this);
		}
		return(*this);
	}
	void Unlink(void)
	{
		if (Is_Valid())
		{
			PrevNode->NextNode = NextNode;
			NextNode->PrevNode = PrevNode;
			PrevNode = nullptr;
			NextNode = nullptr;
		}
	}
	GenericList * Main_List(void) const
	{
		GenericNode const * node = this;
		while (node->PrevNode)
		{
			node = PrevNode;
		}
		return((GenericList *)this);
	}
	void Link(GenericNode * node)
	{
		TT_ASSERT(node != (GenericNode *)nullptr);
		node->Unlink();
		node->NextNode = NextNode;
		node->PrevNode = this;
		if (NextNode) NextNode->PrevNode = node;
		NextNode = node;
	}
	GenericNode * Next(void) const { return(NextNode); }
	GenericNode * Next_Valid(void) const
	{
		return ((NextNode && NextNode->NextNode) ? NextNode : (GenericNode *)nullptr);
	}
	GenericNode * Prev(void) const { return(PrevNode); }
	GenericNode * Prev_Valid(void) const
	{
		return ((PrevNode && PrevNode->PrevNode) ? PrevNode : (GenericNode *)nullptr);
	}
	bool Is_Valid(void) const { return(this != (GenericNode *)nullptr && NextNode != (GenericNode *)nullptr && PrevNode != (GenericNode *)nullptr); }
protected:
	GenericNode * NextNode; // 0004
	GenericNode * PrevNode; // 0008
}; // 000C
class GenericList {
public:
	GenericList(void)
	{
		FirstNode.Link(&LastNode);
	}
	virtual ~GenericList(void)
	{
		while (FirstNode.Next()->Is_Valid())
		{
			FirstNode.Next()->Unlink();
		}
	}
	GenericNode * First(void) const { return(FirstNode.Next()); }
	GenericNode * First_Valid(void) const
	{
		GenericNode *node = FirstNode.Next();
		return (node->Next() ? node : (GenericNode *)nullptr);
	}
	GenericNode * Last(void) const { return(LastNode.Prev()); }
	GenericNode * Last_Valid(void) const
	{
		GenericNode *node = LastNode.Prev();
		return (node->Prev() ? node : (GenericNode *)nullptr);
	}
	bool Is_Empty(void) const { return(!FirstNode.Next()->Is_Valid()); }
	void Add_Head(GenericNode * node) { FirstNode.Link(node); }
	void Add_Tail(GenericNode * node) { LastNode.Prev()->Link(node); }
	int Get_Valid_Count(void) const
	{
		GenericNode * node = First_Valid();
		int counter = 0;
		while (node)
		{
			counter++;
			node = node->Next_Valid();
		}
		return counter;
	}
protected:
	GenericNode FirstNode; // 0004
	GenericNode LastNode; // 0010
private:
	GenericList(GenericList & list);
	GenericList & operator = (GenericList const &);
}; // 001C
template<class T> class List;
template<class T>
class Node : public GenericNode
{
public:
	List<T> * Main_List(void) const { return((List<T> *)GenericNode::Main_List()); }
	T Next(void) const { return((T)GenericNode::Next()); }
	T Next_Valid(void) const { return((T)GenericNode::Next_Valid()); }
	T Prev(void) const { return((T)GenericNode::Prev()); }
	T Prev_Valid(void) const { return((T)GenericNode::Prev_Valid()); }
	bool Is_Valid(void) const { return(GenericNode::Is_Valid()); }
}; // 000C
template<class T>
class List : public GenericList
{
public:
	List(void) {};
	T First(void) const { return((T)GenericList::First()); }
	T First_Valid(void) const { return((T)GenericList::First_Valid()); }
	T Last(void) const { return((T)GenericList::Last()); }
	T Last_Valid(void) const { return((T)GenericList::Last_Valid()); }
	void Delete(void) { while (First()->Is_Valid()) delete First(); }
private:
	List(List<T> const & rvalue) = delete;
	List<T> operator = (List<T> const & rvalue) = delete;
}; // 001C
template<class T>
class DataNode : public GenericNode
{
	T Value;
public:
	DataNode() {};
	DataNode(T value) { Set(value); };
	void Set(T value) { Value = value; };
	T Get() const { return Value; };
	DataNode<T> * Next(void) const { return (DataNode<T> *)GenericNode::Next(); }
	DataNode<T> * Next_Valid(void) const { return (DataNode<T> *)GenericNode::Next_Valid(); }
	DataNode<T> * Prev(void) const { return (DataNode<T> *)GenericNode::Prev(); }
	DataNode<T> * Prev_Valid(void) const { return (DataNode<T> *)GenericNode::Prev_Valid(); }
};
template<class C, class D>
class ContextDataNode : public DataNode<D>
{
	C Context;
public:
	ContextDataNode() {};
	ContextDataNode(C context, D data) { Set_Context(context); Set(data); }
	void Set_Context(C context) { Context = context; };
	C Get_Context() { return Context; };
};
template<class C, class D>
class SafeContextDataNode : public ContextDataNode<C, D>
{
public:
	SafeContextDataNode(C context, D data) : ContextDataNode<C, D>(context, data) { }
private:
	SafeContextDataNode() = delete;
};
template<class PRIMARY, class SECONDARY>
class DoubleNode
{
	void Initialize() { Primary.Set(this); Secondary.Set(this); };
	PRIMARY PrimaryValue;
	SECONDARY SecondaryValue;
public:
	typedef DoubleNode<PRIMARY, SECONDARY> Type;
	DataNode<Type *> Primary;
	DataNode<Type *> Secondary;
	DoubleNode() { Initialize(); };
	DoubleNode(PRIMARY primary, SECONDARY secondary) { Initialize(); Set_Primary(primary); Set_Secondary(secondary); };
	void Set_Primary(PRIMARY value) { PrimaryValue = value; };
	void Set_Secondary(SECONDARY value) { SecondaryValue = value; };
	PRIMARY Get_Primary() { return PrimaryValue; };
	SECONDARY Get_Secondary() { return SecondaryValue; };
	void Unlink() { Primary.Unlink(); Secondary.Unlink(); };
};
