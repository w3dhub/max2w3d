#pragma once

#ifndef SCRIPTS_INCLUDE__ENUM_UTILITIES_H
#define SCRIPTS_INCLUDE__ENUM_UTILITIES_H

template<typename T>
constexpr std::enable_if_t<std::is_enum_v<T>, std::underlying_type_t<T>> enum_to_value(T e)
{
	return static_cast<std::underlying_type_t<T>>(e);
}

//Bitwise operations. Specialise enable struct on std::true_type to use. Disabled by default
template <typename T>
struct EnableEnumClassBitWiseOperators
	: public std::false_type
{ };

template<typename T>
constexpr typename std::enable_if_t<std::is_enum_v<T> && EnableEnumClassBitWiseOperators<T>::value, T> operator|(T lhs, T rhs)
{
	using BaseType = std::underlying_type_t<T>;
	return static_cast<T>(static_cast<BaseType>(lhs) | static_cast<BaseType>(rhs));
}

template<typename T>
constexpr typename std::enable_if_t<std::is_enum_v<T> && EnableEnumClassBitWiseOperators<T>::value, T>& operator|=(T& lhs, T rhs)
{
	lhs = lhs | rhs;
	return lhs;
}

template<typename T>
constexpr typename std::enable_if_t<std::is_enum_v<T> && EnableEnumClassBitWiseOperators<T>::value, T> operator&(T lhs, T rhs)
{
	using BaseType = std::underlying_type_t<T>;
	return static_cast<T>(static_cast<BaseType>(lhs) & static_cast<BaseType>(rhs));
}

template<typename T>
constexpr typename std::enable_if_t<std::is_enum_v<T> && EnableEnumClassBitWiseOperators<T>::value, T>& operator&=(T& lhs, T rhs)
{
	lhs = lhs & rhs;
	return lhs;
}

template<typename T>
constexpr typename std::enable_if_t<std::is_enum_v<T> && EnableEnumClassBitWiseOperators<T>::value, T> operator^(T lhs, T rhs)
{
	using BaseType = std::underlying_type_t<T>;
	return static_cast<T>(static_cast<BaseType>(lhs) ^ static_cast<BaseType>(rhs));
}

template<typename T>
constexpr typename std::enable_if_t<std::is_enum_v<T> && EnableEnumClassBitWiseOperators<T>::value, T>& operator^=(T& lhs, T rhs)
{
	lhs = lhs ^ rhs;
	return lhs;
}

template<typename T>
constexpr typename std::enable_if_t<std::is_enum_v<T> && EnableEnumClassBitWiseOperators<T>::value, T> operator~(T val)
{
	using BaseType = std::underlying_type_t<T>;
	return static_cast<T>(~static_cast<BaseType>(val));
}

template<typename T>
constexpr typename std::enable_if_t<std::is_enum_v<T> && EnableEnumClassBitWiseOperators<T>::value, T> operator<<(T val, int shift)
{
	using BaseType = std::underlying_type_t<T>;
	return static_cast<T>(static_cast<BaseType>(val) << shift);
}

template<typename T>
constexpr typename std::enable_if_t<std::is_enum_v<T> && EnableEnumClassBitWiseOperators<T>::value, T>& operator<<=(T& val, int shift)
{
	val = val << shift;
	return val;
}

template<typename T>
constexpr typename std::enable_if_t<std::is_enum_v<T> && EnableEnumClassBitWiseOperators<T>::value, T> operator>>(T val, int shift)
{
	using BaseType = std::underlying_type_t<T>;
	return static_cast<T>(static_cast<BaseType>(val) >> shift);
}

template<typename T>
constexpr typename std::enable_if_t<std::is_enum_v<T> && EnableEnumClassBitWiseOperators<T>::value, T>& operator>>=(T& val, int shift)
{
	val = val >> shift;
	return val;
}

template<typename T>
constexpr typename std::enable_if_t<std::is_enum_v<T> && EnableEnumClassBitWiseOperators<T>::value, bool> enum_has_flags(T val, T flags)
{
	return (val & flags) == flags;
}

#endif //SCRIPTS_INCLUDE__ENUM_UTILITIES_H
