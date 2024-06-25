#include "General.h"
#include "HashTemplateKeyClass.h"
#include "engine_string.h"

// optimize even in debug mode
#ifdef DEBUG
#pragma optimize("gt", on)
#endif

#define XXH_INLINE_ALL
#include "xxhash.h"


template<> uint SCRIPTS_API HashTemplateKeyClass<uint>::Get_Hash_Value(const uint& key)
{
	return UInt32HashFunc32(key);
}

// ------------------------------
// --- Integer Hash Functions ---
// ------------------------------

uint32 UInt32HashFunc32(uint32 i)
{
	uint32 result = i;
	result ^= result >> 16;
	result *= 0x7feb352du;
	result ^= result >> 15;
	result *= 0x846ca68bu;
	result ^= result >> 16;
	return result;
}

uint64 UInt64HashFunc64(uint64 i)
{
	uint64 result = i;
	result ^= result >> 30;
	result *= UINT64_C(0xbf58476d1ce4e5b9);
	result ^= result >> 27;
	result *= UINT64_C(0x94d049bb133111eb);
	result ^= result >> 31;
	return result;
}

size_t SizeTypeHashFunc(size_t i)
{
#if PLATFORM_32_BIT
	return (size_t) UInt32HashFunc32(i);
#elif PLATFORM_64_BIT
	return (size_t) UInt64HashFunc64(i);
#endif
}

uintptr_t PointerHashFunc(void* ptr)
{
	return (uintptr_t) SizeTypeHashFunc((size_t) ptr);
}


uint32 CombineHash32(uint32 h1, uint32 h2)
{
	// magic number is 2^32 / phi
	return h1 ^ (h2 + 0x9e3779b9u + (h1 << 6) + (h1 >> 2));
}

uint64 CombineHash64(uint64 h1, uint64 h2)
{
	// magic number is 2^64 / phi
	return h1 ^ (h2 + 11400714819323198485ull + (h1 << 6) + (h1 >> 2));
}

size_t CombineHash(size_t h1, size_t h2)
{
#if PLATFORM_32_BIT
	return (size_t)CombineHash32(h1, h2);
#elif PLATFORM_64_BIT
	return (size_t)CombineHash64(h1, h2);
#endif
}

// ----------------------------------
// --- Byte Buffer Hash Functions ---
// ----------------------------------

// Notes on hash functions used:
// FNV1a is very small, has excellent performance in debug mode and is very fast on small inputs (<16-32b). Use for hash tables.
// XXH3_* pollutes cache for smaller/infrequent inputs (~200b of internal state, though it accesses less of it on small inputs).
// XXH3_64 is extremely fast for large inputs (>128b), but the x86 optimization isn't great on small inputs (x64 is fine except in debug).
// XXH3_128 is similar speed to the 64-bit version on larger inputs, a bit slower on smaller.
// XXH32 is faster than XXH3 on x86 for <512 byte inputs and has minimal cache pollution, though debug performance tanks hard on larger inputs.
// XXH64 is extremely slow on x86 and has only one advantage over XXH3 on x64 (smaller state).
// I've tried CityHash, but it's strictly worse than XXH except for its smaller state.
// I've tried RobinHood, but it has absolute garbage performance on x86.

static constexpr uint32 FNV_OFFSET_BASIS_32 = 2166136261u;
static constexpr uint32 FNV_PRIME_32 = 16777619u;
static constexpr uint64 FNV_OFFSET_BASIS_64 = 14695981039346656037ull;
static constexpr uint64 FNV_PRIME_64 = 1099511628211ull;

uint32 ByteHashFunc32_Begin()
{
	return FNV_OFFSET_BASIS_32;
}
uint64 ByteHashFunc64_Begin()
{
	return FNV_OFFSET_BASIS_64;
}
size_t ByteHashFunc_Begin()
{
#if PLATFORM_32_BIT
	return FNV_OFFSET_BASIS_32;
#elif PLATFORM_64_BIT
	return FNV_OFFSET_BASIS_64;
#endif
}

uint32 ByteHashFunc32_Add_Bytes(uint32 current_hash, const byte* buf, size_t length)
{
	for (size_t i = 0; i < length; i++) {
		current_hash ^= buf[i];
		current_hash *= FNV_PRIME_32;
	}
	return current_hash;
}

uint64 ByteHashFunc64_Add_Bytes(uint64 current_hash, const byte* buf, size_t length)
{
	for (size_t i = 0; i < length; i++) {
		current_hash ^= buf[i];
		current_hash *= FNV_PRIME_64;
	}
	return current_hash;
}

size_t ByteHashFunc_Add_Bytes(size_t current_hash, const byte* buf, size_t length)
{
	for (size_t i = 0; i < length; i++) {
		current_hash ^= buf[i];
#if PLATFORM_32_BIT
		current_hash *= FNV_PRIME_32;
#elif PLATFORM_64_BIT
		current_hash *= FNV_PRIME_64;
#endif
	}
	return current_hash;
}

uint32 ByteHashFunc32(const byte* buf, size_t length)
{
	uint32 result = ByteHashFunc32_Begin();
	return ByteHashFunc32_Add_Bytes(result, buf, length);
}

uint64 ByteHashFunc64(const byte* buf, size_t length)
{
	uint64 result = ByteHashFunc64_Begin();
	return ByteHashFunc64_Add_Bytes(result, buf, length);
}

size_t ByteHashFunc(const byte* buf, size_t length)
{
#if PLATFORM_32_BIT
	return (size_t)ByteHashFunc32(buf, length);
#elif PLATFORM_64_BIT
	return (size_t)ByteHashFunc64(buf, length);
#endif
}

Hash128 LargeByteHashFunc128(const byte* buf, size_t length)
{
	alignas(16) XXH128_hash_t a = XXH3_128bits((void*)buf, length);
	return *(Hash128*)&a;
}

uint64 LargeByteHashFunc64(const byte* buf, size_t length)
{
	return XXH3_64bits((void*)buf, length);
}

uint32 LargeByteHashFunc32(const byte* buf, size_t length)
{
	return XXH32((void*)buf, length, FNV_OFFSET_BASIS_32);
}

size_t LargeByteHashFunc(const byte* buf, size_t length)
{
	return (size_t) XXH3_64bits((void*)buf, length);
}



// -----------------------------
// --- String Hash Functions ---
// -----------------------------


template<typename CharType, bool CaseInsensitive, typename HashType>
HashType StringHashFunc_NoLength(const CharType* str)
{
	HashType result = ByteHashFunc_Begin<HashType>();
	while (*str)
	{
		if constexpr (CaseInsensitive)
		{
			// NOTE(Mara): This is generally slower than converting the entire string ahead of time, even with extra allocations. I'll leave it for now.
			if constexpr (std::is_same_v<CharType, wchar_t>) {
				wint_t c = towlower(wint_t(*str));
				result = ByteHashFunc_Add_Bytes<HashType>(result, (const byte*)&c, sizeof(*str));
			}
			else {
				int c = tolower(int(*str));
				result = ByteHashFunc_Add_Bytes<HashType>(result, (const byte*)&c, sizeof(*str));
			}
		}
		else
		{
			result = ByteHashFunc_Add_Bytes<HashType>(result, (const byte*)str, sizeof(*str));
		}
		str++;
	}
	return result;
}

template<typename CharType, bool CaseInsensitive, typename HashType>
HashType StringHashFunc_Length(const CharType* str, size_t length)
{
	if constexpr (CaseInsensitive == false)
	{
		return ByteHashFunc<HashType>((const byte*)str, length * sizeof(*str));
	}
	else
	{
		HashType result = ByteHashFunc_Begin<HashType>();
		for (size_t i = 0; i < length; i++)
		{
			// NOTE(Mara): This is generally slower than converting the entire string ahead of time, even with extra allocations. I'll leave it for now.
			if constexpr (std::is_same_v<CharType, wchar_t>) {
				wint_t c = towlower(wint_t(str[i]));
				result = ByteHashFunc_Add_Bytes<HashType>(result, (const byte*)&c, sizeof(*str));
			}
			else {
				int c = tolower(int(str[i]));
				result = ByteHashFunc_Add_Bytes<HashType>(result, (const byte*)&c, sizeof(*str));
			}
		}
		return result;
	}
}


uint32 StringHashFunc32(StringView str)
{
	return StringHashFunc_Length<char, false, uint32>(str.data(), str.length());
}

size_t StringHashFunc(StringView str)
{
	return StringHashFunc_Length<char, false, size_t>(str.data(), str.length());
}

uint32 StringHashFunc32(WideStringView str)
{
	return StringHashFunc_Length<wchar_t, false, uint32>(str.data(), str.length());
}

size_t StringHashFunc(WideStringView str)
{
	return StringHashFunc_Length<wchar_t, false, size_t>(str.data(), str.length());
}

// --- Case Insensitive ---

uint32 IStringHashFunc32(StringView str)
{
	return StringHashFunc_Length<char, true, uint32>(str.data(), str.length());
}

size_t IStringHashFunc(StringView str)
{
	return StringHashFunc_Length<char, true, size_t>(str.data(), str.length());
}

uint32 IStringHashFunc32(WideStringView str)
{
	return StringHashFunc_Length<wchar_t, true, uint32>(str.data(), str.length());
}

size_t IStringHashFunc(WideStringView str)
{
	return StringHashFunc_Length<wchar_t, true, size_t>(str.data(), str.length());
}

#ifdef DEBUG
#pragma optimize("", on)
#endif