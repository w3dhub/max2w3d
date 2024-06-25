#ifndef TT_INCLUDE__HASHTEMPLATEKEYCLASS_H
#define TT_INCLUDE__HASHTEMPLATEKEYCLASS_H

#include "engine_string_view.h"

template<typename Key> class HashTemplateKeyClass
{
public:
	static uint Get_Hash_Value(const Key& key);
};

template<> uint SCRIPTS_API HashTemplateKeyClass<uint>::Get_Hash_Value(const uint& key);

template<class T> uint HashTemplateKeyClass<T>::Get_Hash_Value(const T& key)
{
	return key.GetHash();
}

SCRIPTS_API uint32 UInt32HashFunc32(uint32 i);
SCRIPTS_API uint64 UInt64HashFunc64(uint64 i);
SCRIPTS_API size_t SizeTypeHashFunc(size_t i);
SCRIPTS_API uintptr_t PointerHashFunc(void* ptr);

SCRIPTS_API uint32 CombineHash32(uint32 h1, uint32 h2);
SCRIPTS_API size_t CombineHash(size_t h1, size_t h2);

SCRIPTS_API uint32 ByteHashFunc32(const byte* buf, size_t length);
SCRIPTS_API uint64 ByteHashFunc64(const byte* buf, size_t length);
SCRIPTS_API size_t ByteHashFunc(const byte* buf, size_t length);

SCRIPTS_API uint32 ByteHashFunc32_Begin();
SCRIPTS_API uint32 ByteHashFunc32_Add_Bytes(uint32 current_hash, const byte* buf, size_t length);
SCRIPTS_API uint64 ByteHashFunc64_Begin();
SCRIPTS_API uint64 ByteHashFunc64_Add_Bytes(uint64 current_hash, const byte* buf, size_t length);
SCRIPTS_API size_t ByteHashFunc_Begin();
SCRIPTS_API size_t ByteHashFunc_Add_Bytes(size_t current_hash, const byte* buf, size_t length);


template<typename T>
TT_INLINE T ByteHashFunc_Begin()
{
	if constexpr (std::is_same_v<T, uint32>)
	{
		return ByteHashFunc32_Begin(); 
	}
	else if constexpr (std::is_same_v<T, uint64>)
	{
		return ByteHashFunc64_Begin(); 
	}
}

template<typename T>
TT_INLINE T ByteHashFunc_Add_Bytes(T current_hash, const byte* buf, size_t length)
{
	if constexpr (std::is_same_v<T, uint32>)
	{
		return ByteHashFunc32_Add_Bytes(current_hash, buf, length);
	}
	else if constexpr (std::is_same_v<T, uint64>)
	{
		return ByteHashFunc64_Add_Bytes(current_hash, buf, length);
	}
}

template<typename T>
TT_INLINE T ByteHashFunc(const byte* buf, size_t length)
{
	if constexpr (std::is_same_v<T, uint32>)
	{
		return ByteHashFunc32(buf, length);
	}
	else if constexpr (std::is_same_v<T, uint64>)
	{
		return ByteHashFunc64(buf, length);
	}
}

//struct Hash128 {
//	__m128i data;
//	TT_INLINE bool operator == (Hash128 other) const {
//		return _mm_movemask_epi8(_mm_cmpeq_epi8(data, other.data)) == 0xFFFF;
//	}
//};

typedef __m128i Hash128;

TT_INLINE bool __vectorcall operator== (Hash128 a, Hash128 b) {
	return _mm_movemask_epi8(_mm_cmpeq_epi8(a, b)) == 0xFFFF;
}

// Use these for large byte buffers, at least 128 bytes

SCRIPTS_API Hash128 LargeByteHashFunc128(const byte* buf, size_t length);
SCRIPTS_API uint64 LargeByteHashFunc64(const byte* buf, size_t length);
SCRIPTS_API uint32 LargeByteHashFunc32(const byte* buf, size_t length);
SCRIPTS_API size_t LargeByteHashFunc(const byte* buf, size_t length);


SCRIPTS_API uint32 StringHashFunc32(StringView str);
SCRIPTS_API uint32 StringHashFunc32(WideStringView str);
SCRIPTS_API size_t StringHashFunc(StringView str);
SCRIPTS_API size_t StringHashFunc(WideStringView str);
SCRIPTS_API uint32 IStringHashFunc32(StringView str);
SCRIPTS_API uint32 IStringHashFunc32(WideStringView str);
SCRIPTS_API size_t IStringHashFunc(StringView str);
SCRIPTS_API size_t IStringHashFunc(WideStringView str);

#endif