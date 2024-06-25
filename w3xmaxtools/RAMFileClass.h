#pragma once
#include "fileclass.h"
class RAMFileClass : public FileClass
{
private:
	char* Buffer;
	int MaxLength;
	int Length;
	int Offset;
	int Access;
	bool IsOpen;
	bool IsAllocated;
	bool Reallocate;
public:
	RAMFileClass(void* buffer, int len) : Buffer((char*)buffer), MaxLength(len), Length(len), Offset(0), Access(1), IsOpen(false), IsAllocated(false), Reallocate(false)
	{
		if (!buffer && len > 0)
		{
			Buffer = (char*)malloc(len);
			IsAllocated = true;
		}
	}
	virtual ~RAMFileClass() override
	{
		IsOpen = false;
		if (IsAllocated)
		{
			free(Buffer);
			Buffer = nullptr;
			IsAllocated = false;
		}
	}
	virtual char const* File_Name() const override
	{
		return("UNKNOWN");
	}
	virtual char const* Set_Name(char const*) override
	{
		return File_Name();
	}
	virtual bool Create() override
	{
		if (Is_Open())
		{
			return false;
		}
		Length = 0;
		return true;
	}
	virtual bool Delete() override
	{
		{
			if (Is_Open())
			{
				return false;
			}
			Length = 0;
			return true;
		}
	}
	virtual bool Is_Available(int forced) override
	{
		return true;
	}
	virtual bool Is_Open() const override
	{
		return IsOpen;
	}
	virtual int Open(char const* filename, int access) override
	{
		return Open(access);
	}
	virtual int Open(int access) override
	{
		if (!Buffer || Is_Open())
		{
			return 0;
		}
		Offset = 0;
		Access = access;
		IsOpen = true;
		if (access == 2)
		{
			Length = 0;
		}
		return Is_Open();
	}
	virtual int Read(void* buffer, int size) override
	{
		if (!Buffer || !buffer || !size)
		{
			return 0;
		}
		bool close = false;
		if (Is_Open())
		{
			if (Access & 1)
			{
				goto l1;
			}
			return 0;
		}
		close = true;
		Open(1);
	l1:
		int len = Length - Offset;
		if (size < len)
		{
			len = size;
		}
		memmove(buffer, &Buffer[Offset], len);
		Offset += len;
		if (close)
		{
			Close();
		}
		return len;
	}
	virtual int Seek(int pos, int dir) override
	{
		if (!Buffer || !Is_Open())
		{
			return Offset;
		}
		int len = Length;
		if (Access & 2)
		{
			len = MaxLength;
		}
		switch (dir)
		{
		case 0:
			Offset = pos;
			break;
		case 1:
			Offset += pos;
			break;
		case 2:
			Offset = len + pos;
			break;
		}
		if (Offset < 0)
		{
			Offset = 0;
		}
		if (Offset > len)
		{
			Offset = len;
		}
		if (Offset > Length)
		{
			Length = Offset;
		}
		return Offset;
	}
	virtual int Size() override
	{
		return Length;
	}
	virtual int Write(void const* buffer, int size) override
	{
		if (!Buffer || !buffer || !size)
		{
			return 0;
		}
		bool close = false;
		if (!Is_Open())
		{
			Open(2);
			close = true;
			goto l1;
		}
		if (!(Access & 2))
		{
			return 0;
		}
	l1:
		int len;
		for (;;)
		{
			int mlen = MaxLength;
			len = MaxLength - Offset;
			if (size <= len || !Reallocate)
			{
				break;
			}
			MaxLength = 2 * mlen;
			Buffer = (char *)realloc(Buffer, 2 * mlen);
		}
		if (size >= len)
		{
			size = MaxLength - Offset;
		}
		memmove(&Buffer[Offset], buffer, size);
		Offset += size;
		if (Offset > Length)
		{
			Length = Offset;
		}
		if (close)
		{
			Close();
		}
		return size;
	}
	virtual void Close() override
	{
		IsOpen = false;
	}
	virtual unsigned long Get_Date_Time() override
	{
		return 0;
	}
	virtual bool Set_Date_Time(unsigned long datetime) override
	{
		return true;
	}
	virtual void Error(int a, int b, const char* c) override
	{
	}
	virtual void Bias(int start, int length) override
	{
		Buffer += start;
		int len = Length;
		if (len >= start + length)
		{
			len = start + length;
		}
		Length = len - start;
		int mlen = MaxLength;
		if (mlen >= start + length)
		{
			mlen = start + length;
		}
		MaxLength = mlen - start;
		if (Is_Open())
		{
			Seek(0, 0);
		}
	}
	virtual bool Is_Hash_Checked() const override
	{
		return false;
	}
	void Set_Reallocate(bool reallocate)
	{
		Reallocate = reallocate;
	}
	char* Get_Buffer()
	{
		return Buffer;
	}
	int Get_Length()
	{
		return Length;
	}
};