#ifndef TT_INCLUDE_CHUNKCLASS_H
#define TT_INCLUDE_CHUNKCLASS_H
#include "iostruct.h"

class FileClass;
class StringClass;

struct ChunkHeader {
	unsigned long ChunkType;
	unsigned long ChunkSize;
};

struct MicroChunkHeader {
	unsigned char ChunkType;
	unsigned char ChunkSize;
};

class SCRIPTS_API ChunkLoadClass
{
	FileClass* File;
	int StackIndex;
	unsigned long PositionStack[256];
	ChunkHeader HeaderStack[256];
	bool InMicroChunk;
	int MicroChunkPosition;
	MicroChunkHeader MCHeader;
public:
	ChunkLoadClass(FileClass *file);
	bool Open_Chunk();
	bool Peek_Next_Chunk(unsigned int *id, unsigned int *length);
	bool Close_Chunk();
	unsigned long Cur_Chunk_ID();
	unsigned long Cur_Chunk_Length();
	int Cur_Chunk_Depth();
	int Contains_Chunks();
	bool Open_Micro_Chunk();
	bool Close_Micro_Chunk();
	unsigned long Cur_Micro_Chunk_ID();
	unsigned long Cur_Micro_Chunk_Length();
	long Seek(unsigned long nbytes);
	long Read(void *buf, unsigned long nbytes);
	long Read(IOVector2Struct *v);
	long Read(IOVector3Struct *v);
	long Read(IOVector4Struct *v);
	long Read(IOQuaternionStruct *q);
	long Read(StringClass& string);

	template <typename T>
	TT_INLINE std::enable_if_t<std::is_enum_v<T>, bool> Is_Cur_Chunk_ID(T id);

	template<typename T> TT_INLINE long SimpleRead(T& buf)
	{
		int length = Read(&buf, sizeof(T));
		TT_ASSERT(length == sizeof(T))
		return length;
	}
};

class SCRIPTS_API ChunkSaveClass {
	FileClass* File;
	int StackIndex;
	int PositionStack[256];
	ChunkHeader HeaderStack[256];
	bool InMicroChunk;
	int MicroChunkPosition;
	MicroChunkHeader MCHeader;
public:
	ChunkSaveClass(FileClass *file);
	void Set_Contains_Chunks()
	{
		HeaderStack[StackIndex-1].ChunkSize |= 0x80000000;
	}

	template <typename T>
	TT_INLINE std::enable_if_t<std::is_enum_v<T>, bool> Begin_Chunk(T id);

	bool Begin_Chunk(unsigned long id);
	bool End_Chunk();
	int Cur_Chunk_Depth();
    unsigned int Cur_Chunk_Length();
	bool Begin_Micro_Chunk(unsigned long id);
	bool End_Micro_Chunk();

	template<typename T, typename = std::enable_if_t<!std::is_pointer_v<T>>> // NOTE: never write pointers straight to disk! always use the pointer remapper!
	unsigned long Write(const T* buf, unsigned long nbytes)
	{
		return Write_Internal(buf, nbytes);
	}
	unsigned long Write(const IOVector2Struct& v);
	unsigned long Write(const IOVector3Struct& v);
	unsigned long Write(const IOVector4Struct& v);
	unsigned long Write(const IOQuaternionStruct& q);
	unsigned long Write(const StringClass& sting);

	template<typename T, typename = std::enable_if_t<!std::is_pointer_v<T>>> // NOTE: never write pointers straight to disk! always use the pointer remapper!
	TT_INLINE long SimpleWrite(const T& buf)
	{
		int length = Write_Internal((void*)&buf, sizeof(T));
		TT_ASSERT(length == sizeof(T))
		return length;
	}

private:
	unsigned long Write_Internal(const void* buf, unsigned long nbytes);
};

#include "ChunkClass.inl"
#define WRITE_MICRO_CHUNK(csave,id,value) \
csave.Begin_Micro_Chunk(id); \
csave.Write(&value,(unsigned long)sizeof(value)); \
csave.End_Micro_Chunk();
#define WRITE_MICRO_CHUNK_STRING(cload,id,string) \
csave.Begin_Micro_Chunk(id); \
csave.Write((void *)string,(unsigned long)strlen(string) + 1ul); \
csave.End_Micro_Chunk();
#define WRITE_MICRO_CHUNK_WWSTRING(csave,id,string) \
csave.Begin_Micro_Chunk(id); \
csave.Write(string.Peek_Buffer(),(int)sizeof(*string.Peek_Buffer())*(string.Get_Length() + 1)); \
csave.End_Micro_Chunk();
#define WRITE_MICRO_CHUNK_VECTOR(csave,id,vector) \
csave.Begin_Micro_Chunk(id); \
csave.Write(vector.begin(), (int)sizeof(vector[0]) * vector.Count()); \
csave.End_Micro_Chunk();
#define WRITE_WWSTRING_CHUNK(csave,id,string) \
csave.Begin_Chunk(id); \
csave.Write(string.Peek_Buffer(), (int)sizeof(*string.Peek_Buffer())*(string.Get_Length() + 1)); \
csave.End_Chunk();

#define LOAD_MICRO_CHUNK(cload,value) \
cload.Read(&value,(unsigned long)sizeof(value));

#define READ_MICRO_CHUNK(cload,id,value) \
case id: \
LOAD_MICRO_CHUNK(cload,value) \
break;

#define READ_MICRO_CHUNK_STRING(cload,id,string,size) \
case id: \
TT_ASSERT(size >= cload.Cur_Micro_Chunk_Length()); \
cload.Read(string,cload.Cur_Micro_Chunk_Length()); \
break;

#define LOAD_MICRO_CHUNK_WWSTRING(cload,string) \
if constexpr (sizeof(*string.Peek_Buffer()) == 1_sz) \
	cload.Read(string.Get_Buffer(cload.Cur_Micro_Chunk_Length()),cload.Cur_Micro_Chunk_Length()); \
else if constexpr (sizeof(*string.Peek_Buffer()) == 2_sz) \
	cload.Read(string.Get_Buffer((cload.Cur_Micro_Chunk_Length() + 1ul) / 2ul),cload.Cur_Micro_Chunk_Length());

#define READ_MICRO_CHUNK_WWSTRING(cload,id,string) \
case id: \
LOAD_MICRO_CHUNK_WWSTRING(cload, string) \
break;

#define LOAD_WWSTRING_CHUNK(cload,string) \
if constexpr (sizeof(*string.Peek_Buffer()) == 1_sz) \
	cload.Read(string.Get_Buffer(cload.Cur_Chunk_Length()), cload.Cur_Chunk_Length()); \
else if constexpr (sizeof(*string.Peek_Buffer()) == 2_sz) \
	cload.Read(string.Get_Buffer((cload.Cur_Chunk_Length() + 1ul) / 2ul), cload.Cur_Chunk_Length());

#define READ_WWSTRING_CHUNK(cload,id,string) \
case id: \
LOAD_WWSTRING_CHUNK(cload, string) \
break;

#define READ_MICRO_CHUNK_DYNAMIC_VECTOR(cload,id,vector) \
case id: { \
	unsigned long size = cload.Cur_Micro_Chunk_Length(); \
	if (size > 0) { \
		int count = int(size / (unsigned long) sizeof(vector[0])); \
		vector.Resize(count); \
		cload.Read(vector.begin(), size); \
		vector.Set_Active(count); \
	} \
} break;

#endif
