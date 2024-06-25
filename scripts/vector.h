#pragma once
template <typename T> class NoEqualsClass
{
public:
	bool operator== (const T &src)
	{
		return false;
	};
	bool operator!= (const T &src)
	{
		return true;
	};
};

class NoInitClass {
public:
	void operator () (void) const {};
};

//CFE Note: It seems like we'd be better off replacing this with std::vector. We can control allocation through an allocator and just a typedef/using statement if we're concerned about initialisation costs
template <class T> class VectorClass {
protected:
	T *Vector; // 0004
	int VectorMax; // 0008
	bool IsValid; // 000C
	bool IsAllocated; // 000D
	bool VectorClassPad[2]; // 000E
public:
	VectorClass(NoInitClass const &) : Vector(nullptr), VectorMax(0), IsValid(false), IsAllocated(false), VectorClassPad{}
	{
	}

	explicit VectorClass(int size = 0, T const * array = nullptr) : Vector(nullptr), VectorMax(size), IsValid(true),
		IsAllocated(false), VectorClassPad{}
	{
		if (size)
		{
			if (array)
			{
				Vector = new((void*)array) T[size];
			}
			else
			{
				Vector = new T[size];
				IsAllocated = true;
			}
		}
	}

	VectorClass(VectorClass<T> const & vector) : Vector(nullptr), VectorMax(0), IsValid(true), IsAllocated(false)
	{
		*this = vector;
	}
	VectorClass<T> &operator= (VectorClass<T> const &vector)
	{
		if (this != &vector)
		{
			Clear();
			VectorMax = vector.Length();
			if (VectorMax)
			{
				Vector = new T[VectorMax];
				if (Vector)
				{
					IsAllocated = true;
					for (int index = 0; index < VectorMax; index++)
					{
						Vector[index] = vector[index];
					}
				}
			}
			else
			{
				Vector = 0;
				IsAllocated = false;
			}
		}
		return *this;
	}

#if _MSC_VER >= 1600
	/* id theft and destruction of evidence */
	VectorClass(VectorClass<T>&& vector) noexcept : Vector(vector.Vector), VectorMax(vector.VectorMax),
		IsValid(vector.IsValid), IsAllocated(vector.IsAllocated),
		VectorClassPad{}
	{
		vector.Vector = 0;
	}

	VectorClass<T>& operator=(VectorClass<T>&& vector) noexcept
	{
		if (this != &vector)
		{
			delete[] Vector;
			Vector = vector.Vector;
			VectorMax = vector.VectorMax;
			IsValid = vector.IsValid;
			IsAllocated = vector.IsAllocated;
			vector.Vector = 0;
		}
		return *this;
	}
#endif

	virtual ~VectorClass()
	{
		VectorClass<T>::Clear();
	}

	T & operator[](int index)
	{
#if ENABLE_VECTOR_RANGE_CHECKS
		TT_ASSERT(index >= 0 && index < VectorMax);
#endif
		return(Vector[index]);
	}

	T const & operator[](int index) const
	{
#if ENABLE_VECTOR_RANGE_CHECKS
		TT_ASSERT(index >= 0 && index < VectorMax);
#endif
		return(Vector[index]);
	}

	virtual bool operator== (VectorClass<T> const &vector) const
	{
		if (VectorMax == vector.Length())
		{
			for (int index = 0; index < VectorMax; index++)
			{
				if (Vector[index] != vector[index])
				{
					return false;
				}
			}
			return true;
		}
		return false;
	}
	virtual bool Resize(int newsize, T const * array = nullptr)
	{
		if (newsize)
		{
			T *newptr;
			IsValid = false;
			if (!array)
			{
				newptr = new T[newsize];
			}
			else
			{
				newptr = new((void*)array) T[newsize];
			}
			IsValid = true;
			if (!newptr)
			{
				return false;
			}
			if (Vector != nullptr)
			{
				int copycount = (newsize < VectorMax) ? newsize : VectorMax;
				for (int index = 0; index < copycount; index++)
				{
					newptr[index] = std::move(Vector[index]);
				}
				if (IsAllocated)
				{
					delete[] Vector;
					Vector = 0;
				}
			}
			Vector = newptr;
			VectorMax = newsize;
			IsAllocated = (Vector && !array);
		}
		else
		{
			Clear();
		}
		return true;
	}
	virtual void Clear(void)
	{
		if (Vector)
		{
			if (IsAllocated) delete[] Vector;
			Vector = 0;
			VectorMax = 0;
			IsAllocated = false;
		}
	}
	int Length(void) const
	{
		return VectorMax;
	}
	virtual int ID(T const *ptr)
	{
		if (!IsValid)
		{
			return 0;
		}
		return static_cast<int>(((ptr - Vector) / sizeof(T)));
	}
	virtual int ID(T const &object)
	{
		if (!IsValid)
		{
			return 0;
		}
		for (int index = 0; index < VectorMax; index++)
		{
			if (Vector[index] == object)
			{
				return index;
			}
		}
		return -1;
	}

	// C++11 iterator/range-for support

	T* begin() noexcept
	{
		return Vector;
	}

	const T* begin() const noexcept
	{
		return Vector;
	}

	virtual T* end() noexcept
	{
		return Vector + VectorMax;
	}
	virtual const T* end() const noexcept
	{
		return Vector + VectorMax;
	}

}; // 0010

template <class T> class DynamicVectorClass : public VectorClass<T> {
protected:
	int ActiveCount = 0; // 0010
	int GrowthStep = 0; // 0014
public:

	explicit DynamicVectorClass(unsigned size = 0, T const *array = nullptr) : VectorClass<T>(size, array)
	{

	}

	DynamicVectorClass(const DynamicVectorClass<T>& vector) : VectorClass<T>(vector), ActiveCount(vector.ActiveCount), GrowthStep(vector.GrowthStep)
	{
		/* nothing */
	}

	template <int size>
	explicit DynamicVectorClass(T(&arr)[size]) : VectorClass<T>(size, arr)
	{

	}

	DynamicVectorClass<T> & operator =(DynamicVectorClass<T> const &rvalue)
	{
		VectorClass<T>::operator =(rvalue);
		ActiveCount = rvalue.ActiveCount;
		GrowthStep = rvalue.GrowthStep;
		return *this;
	}

#if _MSC_VER >= 1600
	/* stealing candy from babies */
	DynamicVectorClass(DynamicVectorClass<T>&& vector) noexcept : VectorClass<T>(std::move(vector)), ActiveCount(vector.ActiveCount), GrowthStep(vector.GrowthStep)
	{
		/* nothing */
	}

	DynamicVectorClass<T>& operator=(DynamicVectorClass<T>&& vector) noexcept
	{
		if (this != &vector)
		{
			VectorClass<T>::operator =(std::move(vector));
			ActiveCount = vector.ActiveCount;
			GrowthStep = vector.GrowthStep;
		}
		return *this;
	}
#endif

	bool operator== (const DynamicVectorClass &src)
	{
		return false;
	}
	bool operator!= (const DynamicVectorClass &src)
	{
		return true;
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

	bool Resize(int newsize, T const *array = nullptr)
	{
		if (VectorClass<T>::Resize(newsize, array))
		{
			if (VectorClass<T>::Length() < ActiveCount)
			{
				ActiveCount = VectorClass<T>::Length();
			}
			return true;
		}
		return false;
	}
	void Clear(void)
	{
		ActiveCount = 0;
		VectorClass<T>::Clear();
	}
	void Reset_Active(void)
	{
		ActiveCount = 0;
	}
	void Set_Active(int count)
	{
		ActiveCount = count;
	}
	int Count(void) const
	{
		return(ActiveCount);
	}
	// Grows by 1.5x if growth step is 0, otherwise grows according to step size.
	// Returns false if the resize fails.
	bool Grow()
	{
		if (GrowthStep == 0)
		{
			int len = VectorClass<T>::Length();
			int newlen = len + (len >> 1); // 1.5x
			if (newlen < 10) newlen = 10; // minimum length 10
			return Grow(newlen);
		}
		else if (GrowthStep > 0)
		{
			return Grow(VectorClass<T>::Length() + GrowthStep);
		}
		TT_UNREACHABLE; // somebody set the growth step to negative
	}
	// Returns false if the resize fails or we don't need to grow (current length is greater than newlen).
	bool Grow(int newlen)
	{
		if (newlen > Length() && (VectorClass<T>::IsAllocated || !VectorClass<T>::VectorMax))
		{
			return Resize(newlen);
		}
		return false;
	}
	bool Add(T const &object)
	{
		if (ActiveCount >= VectorClass<T>::Length())
		{
			if (!Grow())
				return false;
		}
		Vector[ActiveCount++] = object;
		return true;
	}
	bool Add_Head(T const &object)
	{
		return Insert(0, object);
	}
	bool Insert(int index, T const &object)
	{
		if (index < 0)
		{
			return false;
		}
		if (index > ActiveCount)
		{
			return false;
		}
		if (ActiveCount >= VectorClass<T>::Length())
		{
			if (!Grow())
				return false;
		}
		for (int i = ActiveCount; i > index; --i)
		{
			Vector[i] = std::move(Vector[i - 1]);
		}
		Vector[index] = object;
		ActiveCount++;
		return true;
	}

#if _MSC_VER >= 1600
	/* these versions carry move semantics for capable objects */
	bool Add(T&& object)
	{
		if (ActiveCount >= VectorClass<T>::Length())
		{
			if (!Grow())
				return false;
		}
		Vector[ActiveCount++] = std::move(object);
		return true;
	}

	bool Add_Head(T&& object)
	{
		return Insert(0, std::move(object));
	}

	bool Insert(int index, T&& object)
	{
		if (index < 0)
		{
			return false;
		}
		if (index > ActiveCount)
		{
			return false;
		}
		if (ActiveCount >= VectorClass<T>::Length())
		{
			if (!Grow())
				return false;
		}
		for (int i = ActiveCount; i > index; --i)
		{
			Vector[i] = std::move(Vector[i - 1]);
		}
		Vector[index] = std::move(object);
		ActiveCount++;
		return true;
	}
#endif

	bool DeleteObj(T const &object)
	{
		int id = ID(object);
		if (id != -1)
		{
			return Delete(id);
		}
		return false;
	}
	bool Delete(int index)
	{
		TT_ASSERT(index >= 0);
		if (index < ActiveCount)
		{
			ActiveCount--;
			for (int i = index; i < ActiveCount; i++)
			{
				Vector[i] = std::move(Vector[i + 1]);
			}
			return true;
		}
		return false;
	}

	bool DeleteObj_Unordered(T const& object)
	{
		int id = ID(object);
		if (id != -1)
		{
			return Delete_Unordered(id);
		}
		return false;
	}
	bool Delete_Unordered(int index)
	{
		TT_ASSERT(index >= 0);
		if (index < ActiveCount)
		{
			ActiveCount--;
			if (index < ActiveCount)
			{
				Vector[index] = std::move(Vector[ActiveCount]);
			}
			return true;
		}
		return false;
	}
	void Delete_All(void)
	{
		int len = VectorClass<T>::VectorMax;
		Clear();
		Resize(len);
	}
	int Set_Growth_Step(int step)
	{
		return(GrowthStep = step);
	}
	int Growth_Step(void)
	{
		return GrowthStep;
	}
	virtual int ID(T const *ptr)
	{
		return(VectorClass<T>::ID(ptr));
	}
	virtual int ID(T const &object)
	{
		for (int index = 0; index < Count(); index++)
		{
			if (Vector[index] == object)
			{
				return(index);
			}
		}
		return -1;
	}
	T *Uninitialized_Add(void)
	{
		if (ActiveCount >= VectorClass<T>::Length())
		{
			if (!Grow())
				return nullptr;
		}
		return &(Vector[ActiveCount++]);
	}
	void Add_Multiple(const DynamicVectorClass<T>& elements)
	{
		Add_Multiple(elements.begin(), elements.Count());
	}
	void Add_Multiple(const T* elements, int count)
	{
		int newcount = ActiveCount + count;
		Grow(newcount);

		for (int i = 0; i < count; i++)
		{
			Vector[ActiveCount + i] = elements[i];
		}
		ActiveCount = newcount;
	}
	void Add_Multiple(int number_to_add)
	{
		int newcount = ActiveCount + number_to_add;
		Grow(newcount);
		ActiveCount = newcount;
	}

	void qsort(int(*compareCallback)(const void*, const void*))
	{
		::qsort(Vector, ActiveCount, sizeof(T), compareCallback);
	}
	void qsort(int(*compareCallback)(const T&, const T&))
	{
		::qsort(Vector, ActiveCount, sizeof(T), (int(*)(const void*, const void*))compareCallback);
	}

	// C++11 iterator/range-for support

	T* end() noexcept final
	{
		return Vector + ActiveCount;
	}
	const T* end() const noexcept final
	{
		return Vector + ActiveCount;
	}
}; // 0018
