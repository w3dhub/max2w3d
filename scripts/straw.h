#ifndef TT_INCLUDE__STRAW_H
#define TT_INCLUDE__STRAW_H
#include "fileclass.h"
class SCRIPTS_API Straw {
	Straw* ChainTo;
	Straw* ChainFrom;
public:
	Straw();
	virtual ~Straw();
	virtual void Get_From(Straw* straw);
	virtual int Get(void* source,int slen);
};
class SCRIPTS_API Buffer {
	char* BufferPtr;
	long Size;
	bool IsAllocated;
public:
	Buffer(void* buffer,long size);
	Buffer(long size) : BufferPtr(nullptr), Size(size), IsAllocated(false)
	{
		if (size > 0)
		{
			BufferPtr = new char[size];
			IsAllocated = true;
		}
	}
	void *Get_Buffer()
	{
		return BufferPtr;
	}
	long Get_Size()
	{
		return Size;
	}
	~Buffer();
};

class SCRIPTS_API BufferStraw : public Straw  {
	Buffer BufferPtr;
	int Index;
public:
	BufferStraw(void* buffer, int size);
	~BufferStraw();
	int Get(void* source,int slen);
};

class FileClass;
class SCRIPTS_API FileStraw : public Straw {
	FileClass* File;
	bool HasOpened;
public:
	FileStraw(FileClass&);
	~FileStraw();
	int Get(void* source,int slen);
};

class SCRIPTS_API CacheStraw : public Straw {
	Buffer BufferPtr;
	int Index;
	int Length;
public:
	CacheStraw(int size) : BufferPtr(size), Index(0), Length(0)
	{
	}
	bool Is_Valid()
	{
		return BufferPtr.Get_Buffer() != nullptr;
	}
	~CacheStraw() {}
	int Get(void* source,int slen)
	{
		char *src = (char *)source;
		int len = slen;
		int ret = 0;
		int len2;
		if (BufferPtr.Get_Buffer())
		{
			for (int i = source == nullptr;!i && len > 0;i = len2 == 0)
			{
				if (Length > 0 )
				{
					int sz = len;
					if (len > this->Length)
					{
						sz = Length;
					}
					memmove(src,(char *)BufferPtr.Get_Buffer() + Index,sz);
					len -= sz;
					Index += sz;
					ret += sz;
					Length -= sz;
					src += sz;
				}
				if (!len)
				{
					break;
				}
				len2 = Straw::Get(BufferPtr.Get_Buffer(),BufferPtr.Get_Size());
				Length = len2;
				Index = 0;
			}
		}
		return ret;
	}
};

class SCRIPTS_API Base64Straw : public Straw
{
public:
	enum CodeControl {
		ENCODE,
		DECODE,
	};
	Base64Straw(CodeControl control) : Control(control), Counter(0), CBuffer{}, PBuffer{}
	{
	}

	int Get(void* source, int slen);
	~Base64Straw() {};
private:
	CodeControl Control;
	int Counter;
	char CBuffer[4];
	char PBuffer[3];
};

class SCRIPTS_API Pipe
{
	Pipe* ChainTo;
	Pipe* ChainFrom;
public:
	Pipe() : ChainTo(nullptr), ChainFrom(nullptr)
	{
	}
	virtual ~Pipe()
	{
		if (ChainTo)
		{
			ChainTo->ChainFrom = ChainFrom;
		}
		if (ChainFrom)
		{
			ChainFrom->Put_To(ChainTo);
		}
		ChainFrom = nullptr;
		ChainTo = nullptr;
	}
	virtual int Flush()
	{
		if (ChainTo)
		{
			return ChainTo->Flush();
		}
		else
		{
			return 0;
		}
	}
	virtual int End()
	{
		return Flush();
	}
	virtual void Put_To(Pipe* pipe)
	{
		if (ChainTo != pipe)
		{
			if (pipe && pipe->ChainFrom)
			{
				pipe->ChainFrom->Put_To(nullptr);
				pipe->ChainFrom = nullptr;
			}
			if (ChainTo)
			{
				ChainTo->ChainFrom = nullptr;
				ChainTo->Flush();
			}
			ChainTo = pipe;
			if (pipe)
			{
				pipe->ChainFrom = this;
			}
		}
	}
	virtual int Put(const void *source,int length)
	{
		if (ChainTo)
		{
			return ChainTo->Put(source,length);
		}
		return length;
	}
};

class BufferPipe : public Pipe
{
	Buffer BufferPtr;
	int Index;
public:
	BufferPipe(void *data,int size) : BufferPtr(data,size), Index(0)
	{
	}
	virtual int Put(const void *source,int length)
	{
		if (BufferPtr.Get_Buffer() && source && length > 0)
		{
			int len = length;
			int size = BufferPtr.Get_Size();
			if (size)
			{
				len = size - Index;
				if (len > length)
				{
					len = length;
				}
			}
			if (len > 0)
			{
				memmove((char *)(BufferPtr.Get_Buffer()) + Index,source,len);
			}
			Index += len;
			return len;
		}
		return 0;
	}
};

class FilePipe : public Pipe
{
	FileClass* File;
	bool HasOpened;
public:
	FilePipe(FileClass *file) : File(file), HasOpened(false)
	{
	}
	virtual ~FilePipe()
	{
		if (File && HasOpened)
		{
			HasOpened = false;
			File->Close();
			File = nullptr;
		}
	}
	virtual int End()
	{
		int ret = Flush();
		if (File && HasOpened)
		{
			HasOpened = false;
			File->Close();
		}
		return ret;
	}
	virtual int Put(const void *source,int length)
	{
		if (File && source && length > 0)
		{
			if (!File->Is_Open())
			{
				HasOpened = true;
				File->Open(2);
			}
			return File->Write((void *)source,length);
		}
		else
		{
			return 0;
		}
	}
};

class SCRIPTS_API Base64Pipe : public Pipe
{
public:
	enum CodeControl {
		ENCODE,
		DECODE,
	};
	Base64Pipe(CodeControl control) : Control(control), Counter(0), CBuffer{}, PBuffer{}
	{
	}

	int Put(const void *source, int slen);
	int Flush();
	~Base64Pipe() {};
private:
	CodeControl Control;
	int Counter;
	char CBuffer[4];
	char PBuffer[3];
};
#endif
