#pragma once
template <typename T1, class T2> class IndexClass {
	struct NodeElement {
		T1 ID;
		T2 Data;
		NodeElement(T1 const &id, T2 const &data) : ID(id), Data(data)
		{
		}
		NodeElement() : ID(0), Data(0)
		{
		}
		bool operator==(NodeElement const &elem)
		{
			return ID == elem.ID;
		}
		bool operator<(NodeElement const &elem)
		{
			return ID < elem.ID;
		}
	};
	NodeElement* IndexTable;
	int IndexCount;
	int IndexSize;
	unsigned char IsSorted;
	const NodeElement* Archive;
public:
	IndexClass() : IndexTable(nullptr), IndexCount(0), IndexSize(0), IsSorted(false), Archive(nullptr)
	{
		Invalidate_Archive();
	}
	~IndexClass()
	{
		Clear();
	}
	bool Remove_Index(const T1 &ID)
	{
		int pos = -1;
		for (int i = 0; i < IndexCount; i++)
		{
			if (IndexTable[i].ID == ID)
			{
				pos = i;
				break;
			}
		}
		if (pos == -1)
		{
			return false;
		}
		else
		{
			for (int i = pos; i < IndexCount; i++)
			{
				IndexTable[i] = IndexTable[i + 1];
			}
		}
		IndexCount--;
		IndexTable[IndexCount].ID = 0;
		IndexTable[IndexCount].Data = 0;
		Invalidate_Archive();
		return true;
	}
	bool Is_Present(const T1 &ID)
	{
		if (IndexCount)
		{
			if (Is_Archive_Same(ID))
			{
				return true;
			}
			else
			{
				NodeElement *node = Search_For_Node(ID);
				if (node)
				{
					Set_Archive(node);
					return true;
				}
				else
				{
					return false;
				}
			}
		}
		return false;
	}
	bool Add_Index(const T1 &ID, const T2 &Data)
	{
		if (IndexCount + 1 <= IndexSize)
		{
			IndexTable[IndexCount].ID = ID;
			IndexTable[IndexCount++].Data = Data;
			IsSorted = false;
			return true;
		}
		int size = IndexSize;
		if (!size)
		{
			size = 10;
		}
		if (Increase_Table_Size(size))
		{
			IndexTable[IndexCount].ID = ID;
			IndexTable[IndexCount++].Data = Data;
			IsSorted = false;
			return true;
		}
		else
		{
			return false;
		}
	}
	int Count() const
	{
		return IndexCount;
	}
	const T2 &operator[](T1 const &ID)
	{
		static const T2 x = nullptr;
		if (Is_Present(ID))
		{
			return Archive->Data;
		}
		return x;
	}
	void Invalidate_Archive()
	{
		Archive = 0;
	}
	void Clear()
	{
		if (IndexTable)
		{
			delete[] IndexTable;
		}
		IndexTable = 0;
		IndexCount = 0;
		IndexSize = 0;
		IsSorted = 0;
		Invalidate_Archive();
	}
	bool Is_Archive_Same(const T1 &ID)
	{
		return Archive && Archive->ID == ID;
	}
	NodeElement *Search_For_Node(const T1 &ID)
	{
		if (IndexCount)
		{
			if (!IsSorted)
			{
				qsort(IndexTable, IndexCount, sizeof(NodeElement), search_compfunc);
				Invalidate_Archive();
				IsSorted = true;
			}
			NodeElement elem(ID, 0);
			return Binary_Search<NodeElement>(IndexTable, IndexCount, elem);
		}
		return nullptr;
	}
	void Set_Archive(NodeElement const *archive)
	{
		Archive = archive;
	}
	bool Increase_Table_Size(int amount)
	{
		if (amount >= 0)
		{
			int newsize = IndexSize + amount;
			NodeElement *newindex = new NodeElement[newsize];
			if (newindex)
			{
				TT_ASSERT(IndexCount < newsize);
				for (int i = 0; i < this->IndexCount; i++)
				{
					newindex[i].ID = IndexTable[i].ID;
					newindex[i].Data = IndexTable[i].Data;
				}
				if (IndexTable)
					delete[] IndexTable;
				IndexTable = newindex;
				IndexSize += amount;
				Invalidate_Archive();
				return true;
			}
		}
		return false;
	}
	T1 Fetch_ID_By_Position(int position)
	{
		return IndexTable[position].ID;
	}
	T2 Fetch_By_Position(int position)
	{
		return IndexTable[position].Data;
	}
	static int search_compfunc(void  const *ptr2, void  const *ptr1)
	{
		if (*(NodeElement *)ptr2 == *(NodeElement *)ptr1)
		{
			return 0;
		}
		else
		{
			if (*(NodeElement *)ptr1 < *(NodeElement *)ptr2)
			{
				return 1;
			}
			else
			{
				return -1;
			}
		}
	}
}; // 0014
template <typename T> T *Binary_Search(T *list, int count, T &var)
{
	T *list2 = list;
	int pos = count;
	while (pos > 0)
	{
		T *list3 = &list2[pos / 2];
		if (var.ID >= list3->ID)
		{
			if (list3->ID == var.ID)
			{
				return &list2[pos / 2];
			}
			list2 = list3 + 1;
			pos = pos - pos / 2 - 1;
		}
		else
		{
			pos /= 2;
		}
	}
	return nullptr;
}
