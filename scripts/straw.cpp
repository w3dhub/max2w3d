#include "general.h"
#include "straw.h"
#include "base64.h"

Straw::Straw() : ChainTo(nullptr), ChainFrom(nullptr)
{
}

Straw::~Straw()
{
	if (ChainTo)
	{
		ChainTo->ChainFrom = ChainFrom;
	}
	if (ChainFrom)
	{
		ChainFrom->Get_From(ChainFrom);
	}
	ChainTo = nullptr;
	ChainFrom = nullptr;
}

void Straw::Get_From(Straw* straw)
{
	if (ChainTo != straw)
	{
		if ((straw) && (ChainFrom))
		{
			ChainFrom->Get_From(nullptr);
			ChainFrom = nullptr;
		}
		if (ChainTo)
		{
			ChainFrom = nullptr;
		}
		ChainTo = straw;
		if (straw)
		{
			straw->ChainFrom = this;
		}
	}
}

int Straw::Get(void* source,int slen)
{
	if (!ChainTo)
	{
		return 0;
	}
	return ChainTo->Get(source,slen);
}

Buffer::Buffer(void* buffer,long size)
{
	BufferPtr = (char *)buffer;
	Size = size;
	IsAllocated = false;
	if ((!buffer) && (size > 0))
	{
		BufferPtr = new char[size];
		IsAllocated = true;
	}
}

Buffer::~Buffer()
{
	if (IsAllocated)
	{
		delete[] BufferPtr;
	}
	BufferPtr = nullptr;
	Size = 0;
	IsAllocated = false;
}

BufferStraw::BufferStraw(void* buffer, int size) : BufferPtr(buffer,size), Index(0)
{
}

BufferStraw::~BufferStraw()
{
}

int BufferStraw::Get(void* source,int slen)
{
	if ((!BufferPtr.Get_Buffer()) || (!source) || (slen <= 0))
	{
		return 0;
	}
	int bsz = BufferPtr.Get_Size();
	int sz = slen;
	if (bsz)
	{
		bsz -= Index;
		if (slen > bsz)
		{
			sz = bsz;
		}
	}
	if (sz)
	{
		memmove(source,((char *)BufferPtr.Get_Buffer())+Index,sz);
	}
	Index += sz;
	return sz;
}

FileStraw::FileStraw(FileClass& f) : Straw()
{
	File = &f;
	HasOpened = false;
}

FileStraw::~FileStraw()
{
	if (File)
	{
		if (HasOpened)
		{
			File->Close();
			HasOpened = false;
			File = nullptr;
		}
	}
}

int FileStraw::Get(void* source,int slen)
{
	if (File && source && slen)
	{
		if (!File->Is_Open())
		{
			HasOpened = true;
			if (!File->Is_Available() || !File->Open())
			{
				return 0;
			}
		}
		return File->Read(source,slen);
	}
	else
	{
		return 0;
	}
}

int Base64Straw::Get(void* source, int slen)
{
	int total = 0;
	int fromsize;
	char *from;
	char *to;
	int tosize;
	if (Control == DECODE)
	{
		fromsize = 4;
		from = CBuffer;
		to = PBuffer;
		tosize = 3;
	}
	else
	{
		fromsize = 3;
		from = PBuffer;
		to = CBuffer;
		tosize = 4;
	}
	while (slen > 0)
	{
		if (Counter > 0)
		{
			int tocopy = slen;
			if (slen >= Counter)
			{
				tocopy = Counter;
			}
			memmove(source, &to[tosize - Counter], tocopy);
			Counter -= tocopy;
			slen -= tocopy;
			source = (char *)source + tocopy;
			total += tocopy;
		}
		if (!slen)
		{
			break;
		}
		int fromsize2 = Straw::Get(from, fromsize);
		Counter = Control ? Base64_Decode(from, fromsize2, to, tosize) : Base64_Encode(from, fromsize2, to, tosize);
		if (!Counter)
		{
			break;
		}
	}
	return total;
}

int Base64Pipe::Put(const void *source, int slen)
{
	int total = 0;
	int fromsize;
	char *to;
	int outlen;
	char *from;
	int tosize;
	if (source && slen > 0)
	{
		if (Control == DECODE)
		{
			fromsize = 4;
			from = CBuffer;
			to = PBuffer;
			tosize = 3;
		}
		else
		{
			fromsize = 3;
			from = PBuffer;
			to = CBuffer;
			tosize = 4;
		}
		if (Counter > 0)
		{
			int len = slen;
			if (slen >= fromsize - Counter)
			{
				len = fromsize - Counter;
			}
			memmove(&from[Counter], source, len);
			Counter = len + Counter;
			source = (char *)source + len;
			slen -= len;
			if (Counter == fromsize)
			{
				if (Control == DECODE)
				{
					outlen = Base64_Decode(from, fromsize, to, tosize);
				}
				else
				{
					outlen = Base64_Encode(from, fromsize, to, tosize);
				}
				total = Pipe::Put(to, outlen);
				Counter = 0;
			}
		}
		for (; slen >= fromsize; slen -= fromsize)
		{
			if (Control == DECODE)
			{
				outlen = Base64_Decode(source, fromsize, to, tosize);
			}
			else
			{
				outlen = Base64_Encode(source, fromsize, to, tosize);
			}
			source = (char *)source + fromsize;
			total += Pipe::Put(to, outlen);
		}
		if (slen > 0)
		{
			memmove(from, source, slen);
			Counter = slen;
		}
	}
	return total;
}

int Base64Pipe::Flush()
{
	int len = 0;
	char *buffer;
	int b64len;
	if (Counter)
	{
		if (Control == DECODE)
		{
			buffer = PBuffer;
			b64len = Base64_Decode(CBuffer, Counter, PBuffer, 3);
		}
		else
		{
			buffer = CBuffer;
			b64len = Base64_Encode(PBuffer, Counter, CBuffer, 4);
		}
		len = Pipe::Put(buffer, b64len);
		Counter = 0;
	}
	return len + Pipe::Flush();
}
