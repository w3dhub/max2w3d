#pragma once
template <class T> class SimpleVecClass {
protected:
	T *Vector; // 0004
	int VectorMax; // 0008
public:
	explicit SimpleVecClass(int size = 0)
	{
		Vector = 0;
		VectorMax = 0;
		if (size > 0)
		{
			SimpleVecClass<T>::Resize(size);
		}
	}
	virtual ~SimpleVecClass()
	{
		if (Vector)
		{
			delete[] Vector;
			Vector = 0;
			VectorMax = 0;
		}
	}

	SimpleVecClass(const SimpleVecClass<T>& vector)
	{
		Vector = new T[vector.VectorMax];
		VectorMax = vector.VectorMax;
		memcpy(Vector, vector.Vector, VectorMax * sizeof(T));
	}

	SimpleVecClass<T>& operator =(const SimpleVecClass<T>& vector)
	{
		if (this != &vector)
		{
			delete[] Vector;
			Vector = new T[vector.VectorMax];
			VectorMax = vector.VectorMax;
			memcpy(Vector, vector.Vector, VectorMax * sizeof(T));
		}
		return *this;
	}

#if _MSC_VER >= 1600
	/* more move semantics, yay! */
	SimpleVecClass(SimpleVecClass<T>&& vector) noexcept : Vector(vector.Vector), VectorMax(vector.VectorMax)
	{
		vector.Vector = 0;
	}

	SimpleVecClass<T>& operator =(SimpleVecClass<T>&& vector) noexcept
	{
		if (this != &vector)
		{
			delete[] Vector;
			Vector = vector.Vector;
			VectorMax = vector.VectorMax;
			vector.Vector = 0;
		}
		return *this;
	}
#endif

	virtual bool Resize(int newsize)
	{
		if (VectorMax == newsize)
		{
			return true;
		}
		if (newsize > 0)
		{
			T *vec = new T[newsize];
			if (Vector)
			{
				int count = VectorMax;
				if (newsize < count)
				{
					count = newsize;
				}
				memcpy(vec, Vector, count * sizeof(T));
				delete[] Vector;
				Vector = 0;
			}
			Vector = vec;
			VectorMax = newsize;
		}
		else
		{
			VectorMax = 0;
			if (Vector)
			{
				delete[] Vector;
				Vector = 0;
			}
		}
		return true;
	}
	virtual bool Uninitialised_Grow(int newsize)
	{
		if (newsize <= VectorMax)
		{
			return true;
		}
		if (newsize > 0)
		{
			if (Vector)
			{
				delete[] Vector;
			}
			Vector = new T[newsize];
			VectorMax = newsize;
		}
		return true;
	}

	void Uninitialized_Resize(int newsize)
	{
		TT_ASSERT(newsize > 0);
		delete[] Vector;
		Vector = new T[newsize];
		VectorMax = newsize;
	}

	int Length() const
	{
		return VectorMax;
	}
	T &operator[](int index)
	{
#if ENABLE_VECTOR_RANGE_CHECKS
		TT_ASSERT(index >= 0 && index < VectorMax);
#endif
		return Vector[index];
	}
	T const &operator[](int index) const
	{
#if ENABLE_VECTOR_RANGE_CHECKS
		TT_ASSERT(index >= 0 && index < VectorMax);
#endif
		return Vector[index];
	}

	// C++11 iterator/range-for support

	T* begin()
	{
		return Vector;
	}

	const T* begin() const
	{
		return Vector;
	}

	virtual T* end()
	{
		return Vector + VectorMax;
	}
	virtual const T* end() const
	{
		return Vector + VectorMax;
	}

	void Zero_Memory()
	{
		if (Vector != nullptr)
		{
			memset(Vector, 0, VectorMax * sizeof(T));
		}
	}
}; // 000C



template <class T> class SimpleDynVecClass :
	public SimpleVecClass<T>
{

protected:

	int ActiveCount; // 000C

	bool Grow(int new_size_hint)
	{
		int new_size = max(SimpleVecClass<T>::VectorMax + SimpleVecClass<T>::VectorMax / 4, SimpleVecClass<T>::VectorMax + 4);
		new_size = max(new_size, new_size_hint);
		return Resize(new_size);
	}
	bool Shrink(void)
	{
		if (ActiveCount < SimpleVecClass<T>::VectorMax / 4)
		{
			return Resize(ActiveCount);
		}
		return true;
	}

public:
	virtual ~SimpleDynVecClass()
	{
		ActiveCount = 0;
	}
	explicit SimpleDynVecClass(int size = 0) : SimpleVecClass<T>(size)
	{
		ActiveCount = 0;
	}

	SimpleDynVecClass(const SimpleDynVecClass<T>& vector) : SimpleVecClass(vector), ActiveCount(vector.ActiveCount)
	{
		/* nothing */
	}

	SimpleDynVecClass<T>& operator =(const SimpleDynVecClass<T>& vector)
	{
		if (this != &vector)
		{
			SimpleVecClass<T>::operator =(vector);
			ActiveCount = vector.ActiveCount;
		}
		return *this;
	}

#if _MSC_VER >= 1600
	/* move semantics ftw */
	SimpleDynVecClass(SimpleDynVecClass<T>&& vector) noexcept : SimpleVecClass(std::move(vector)), ActiveCount(vector.ActiveCount)
	{
		/* nothing */
	};

	SimpleDynVecClass<T>& operator =(SimpleDynVecClass<T>&& vector) noexcept
	{
		if (this != &vector)
		{
			SimpleVecClass<T>::operator =(std::move(vector));
			ActiveCount = vector.ActiveCount;
		}
		return *this;
	}
#endif

	int Find_Index(T const & object)
	{
		for (int index = 0; index < Count(); index++)
		{
			if (SimpleVecClass<T>::Vector[index] == object)
			{
				return index;
			}
		}
		return -1;
	}

	int Count() const
	{
		return ActiveCount;
	}

#if ENABLE_VECTOR_RANGE_CHECKS
	T& operator[](int index)
	{
		TT_ASSERT(index >= 0 && index < ActiveCount);
		return(Vector[index]);
	}

	T const& operator[](int index) const
	{
		TT_ASSERT(index >= 0 && index < ActiveCount);
		return(Vector[index]);
	}
#endif

	bool Resize(int newsize)
	{
		if (SimpleVecClass<T>::Resize(newsize))
		{
			if (SimpleVecClass<T>::VectorMax < ActiveCount)
			{
				ActiveCount = SimpleVecClass<T>::VectorMax;
			}
			return true;
		}
		return false;
	}
	bool Add(T const& data, int new_size_hint = 0)
	{
		if (ActiveCount >= SimpleVecClass<T>::VectorMax)
		{
			if (!Grow(new_size_hint))
			{
				return false;
			}
		}
		SimpleVecClass<T>::Vector[ActiveCount++] = data;
		return true;
	}
	T *Add_Multiple(int number_to_add)
	{
		int index = ActiveCount;
		ActiveCount += number_to_add;
		if (ActiveCount > SimpleVecClass<T>::VectorMax)
		{
			Grow(ActiveCount);
		}
		return &SimpleVecClass<T>::Vector[index];
	}
	void Add_Multiple(const SimpleDynVecClass<T>& elements)
	{
		Add_Multiple(elements.begin(), elements.Count());
	}
	void Add_Multiple(const T* elements, int count)
	{
		int newcount = ActiveCount + count;
		if (newcount > VectorMax) Grow(newcount);
		memcpy(Vector + ActiveCount, elements, count * sizeof(T));
		ActiveCount = newcount;
	}
	bool Add_Head(const T& object)
	{
		return Insert(0, object);
	}
	bool Insert(int index, const T& object)
	{
		TT_ASSERT(index >= 0 && index <= ActiveCount);
		if (ActiveCount >= SimpleVecClass<T>::VectorMax)
		{
			if (!Grow(0))
			{
				return false;
			}
		}
		if (index < ActiveCount)
		{
			memmove(&SimpleVecClass<T>::Vector[index + 1], &SimpleVecClass<T>::Vector[index], (ActiveCount - index) * sizeof(T));
		}
		SimpleVecClass<T>::Vector[index] = object;
		++ActiveCount;
		return true;
	}
	T& Pop_Back() {
		return Vector[--ActiveCount];
	}
	bool Delete(int index, bool allow_shrink = true)
	{
		TT_ASSERT(index >= 0 && index < ActiveCount);
		if (index < ActiveCount - 1)
		{
			memmove(&(SimpleVecClass<T>::Vector[index]), &(SimpleVecClass<T>::Vector[index + 1]), (ActiveCount - index - 1) * sizeof(T));
		}
		ActiveCount--;
		if (allow_shrink)
		{
			Shrink();
		}
		return true;
	}
	bool Delete_Unordered(int index, bool allow_shrink = true)
	{
		TT_ASSERT(index >= 0 && index < ActiveCount);
		if (index < ActiveCount - 1)
		{
			SimpleVecClass<T>::Vector[index] = SimpleVecClass<T>::Vector[ActiveCount - 1];
		}
		ActiveCount--;
		if (allow_shrink)
		{
			Shrink();
		}
		return true;
	}
	bool Delete(T const & object, bool allow_shrink = true)
	{
		int id = Find_Index(object);
		if (id != -1)
		{
			return Delete(id, allow_shrink);
		}
		return false;
	}
	bool Delete_Unordered(T const & object, bool allow_shrink = true)
	{
		int id = Find_Index(object);
		if (id != -1)
		{
			return Delete_Unordered(id, allow_shrink);
		}
		return false;
	}
	bool Delete_Range(int start, int count, bool allow_shrink = true)
	{
		TT_ASSERT(start >= 0 && (start + count) <= ActiveCount);
		if (start < ActiveCount - count)
		{
			memmove(&(SimpleVecClass<T>::Vector[start]), &(SimpleVecClass<T>::Vector[start + count]), (ActiveCount - start - count) * sizeof(T));
		}
		ActiveCount -= count;
		if (allow_shrink)
		{
			Shrink();
		}
		return true;
	}
	void Delete_All(bool allow_shrink = true)
	{
		ActiveCount = 0;
		if (allow_shrink)
		{
			Shrink();
		}
	}

	void qsort(int(*compareCallback)(const void*, const void*))
	{
		::qsort(SimpleVecClass<T>::Vector, ActiveCount, sizeof(T), compareCallback);
	}
	void qsort(int(*compareCallback)(const T&, const T&))
	{
		::qsort(SimpleVecClass<T>::Vector, ActiveCount, sizeof(T), (int(*)(const void*, const void*))compareCallback);
	}

	bool isEmpty() const { return ActiveCount == 0; }

	// C++11 iterator/range-for support

	virtual T* end() final
	{
		return Vector + ActiveCount;
	}
	virtual const T* end() const final
	{
		return Vector + ActiveCount;
	}

}; // 0010
