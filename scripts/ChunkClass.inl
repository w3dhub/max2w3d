#pragma once
#include "EnumUtilities.h"

template <typename T>
std::enable_if_t<std::is_enum_v<T>, bool> ChunkLoadClass::Is_Cur_Chunk_ID(T id)
{
	return Cur_Chunk_ID() == enum_to_value(id);
}

template <typename T>
std::enable_if_t<std::is_enum_v<T>, bool> ChunkSaveClass::Begin_Chunk(T id)
{
	return Begin_Chunk(enum_to_value(id));
}