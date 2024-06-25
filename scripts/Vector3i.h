#ifndef TT_INCLUDE_VECTOR3I_H
#define TT_INCLUDE_VECTOR3I_H
class Vector3i
{
public:
	int		I;
	int		J;
	int		K;
	constexpr Vector3i(): I(0), J(0), K(0)
	{
	}

	inline constexpr Vector3i(int i,int j,int k)
		: I(i), J(j), K(k)
	{
		
	}
	inline bool			operator== (const Vector3i & v) const
	{
		return (I == v.I && J == v.J && K == v.K);
	}
	inline bool			operator!= (const Vector3i& v) const
	{
		return !(I == v.I && J == v.J && K == v.K);
	}
	inline const	int&	operator[] (int n) const
	{
		return ((int*)this)[n];
	}
	inline int&			operator[] (int n)
	{
		return ((int*)this)[n];
	}
};
class Vector3i16 {
public:
	unsigned short I;
	unsigned short J;
	unsigned short K;
	inline Vector3i16(): I(0), J(0), K(0)
	{
	}

	inline Vector3i16(unsigned short i,unsigned short j,unsigned short k)
	{
		I = i; J = j; K = k;
	}
	inline const unsigned short &	operator[] (int n) const
	{
		return ((unsigned short *)this)[n]; 
	}
	inline unsigned short & operator[] (int n)
	{
		return ((unsigned short *)this)[n]; 
	}

	unsigned short* begin() { return reinterpret_cast<unsigned short*>(this); }
	const unsigned short* begin() const { return reinterpret_cast<const unsigned short*>(this); }

	unsigned short* end()   { return reinterpret_cast<unsigned short*>(this) + 3; }
	const unsigned short* end() const { return reinterpret_cast<const unsigned short*>(this) + 3; }
};

struct Vector3i16Hash
{
	size_t operator()(const Vector3i16& vec) const
	{
		std::hash<unsigned short> shrt_hash;
		return shrt_hash(vec.I) ^ shrt_hash(vec.J) << 2 ^ shrt_hash(vec.K) >> 2;
	}
};

#endif
