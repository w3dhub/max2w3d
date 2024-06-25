#ifndef TT_INCLUDE_RAWFILECLASS_H
#define TT_INCLUDE_RAWFILECLASS_H



#include "fileclass.h"
#include "engine_math.h"
#include "engine_string.h"

class RawFileClass : public FileClass {
	int Mode = 0; // 0004
	int BiasStart = 0; // 0008
	int BiasLength = -1; // 000C
	void* Handle = INVALID_HANDLE_VALUE; // 0010
	StringClass Filename; // 0014
	unsigned short Date = 0; // 0018
	unsigned short Time = 0; // 001C
	int CurrentOffset = 0; // unbiased offset in whole file
	int CachedSize = -1; // never use this directly, always call Size()

protected:

	RawFileClass() {}
	RawFileClass(const char* filename) : Filename(filename) {}

	int Raw_Seek(int offset, int origin)
	{
		if (offset == 0 && origin == ORIGIN_CURRENT)
			return CurrentOffset;

		if (offset == CurrentOffset && origin == ORIGIN_START)
			return CurrentOffset;

		if (!Is_Open())
			Error(9, 0, Filename);

		CurrentOffset = SetFilePointer(Handle, offset, nullptr, origin);
		if (CurrentOffset == -1)
			Error(GetLastError(), 0, Filename);

		return CurrentOffset;
	}

	void Error(int error_code, int b /* ??? */, const char* filename) final
	{
#ifdef DEBUG
		char buffer[256];
		int pos = snprintf(buffer, sizeof(buffer) - 1, "File Error: filename '%s', code %d\nError Message: ", filename ? filename : "<NULL>", error_code);

		pos += FormatMessageA(
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			nullptr,
			error_code,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			buffer + pos,
			ARRAYSIZE(buffer) - 1 - pos,
			nullptr);

		buffer[pos] = '\n';
		buffer[pos + 1] = '\0';

		OutputDebugStringA(buffer);
#endif
	}

	int Get_Mode() {
		return Mode & 3;
	}

public:

	virtual bool Is_Hash_Checked() const override
	{
		return Is_Biased();
	}

	bool Is_Biased() const
	{
		return BiasLength != -1;
	}

	int Transfer_Block_Size()
	{
		return -11; // ???
	}

	void Reset()
	{
		Close();
		Filename.Free_String();
	}

	~RawFileClass()
	{
		Reset();
	}

	virtual const char * File_Name() const override
	{
		return Filename;
	}

	const char *Set_Name(const char* name) override
	{
		Reset();
		if (name)
		{
			Bias(0,-1);
			Filename = name;
		}
		return nullptr;
	}

	bool Create() override
	{
		Close();
		if (!Open(OPEN_WRITE))
		{
			return false;
		}
		if (BiasLength != -1)
		{
			Seek(0, ORIGIN_START);
		}
		Close();
		return true;
	}

	bool Delete() override
	{
		Close();
		if (Filename.Is_Empty())
		{
			Error(2,0,nullptr);
		}
		if (!Is_Available(0))
		{
			return false;
		}
		if (!DeleteFileA(Filename))
		{
			Error(GetLastError(),0,Filename);
			return false;
		}
		return true;
	}

	virtual bool Is_Available(int _handle) override
	{
		if (Filename.Is_Empty())
			return false;
		
		if (Is_Open())
			return true;
		
		if (_handle)
		{
			Open(OPEN_READ);
			Close();
			return true;
		}

		int attr = GetFileAttributesA(Filename);
		return attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY);
	}

	bool Is_Open() const override
	{
		if (Handle != INVALID_HANDLE_VALUE)
		{
			return true;
		}
		return false;
	}
	int Open(const char* name, int mode) override
	{
		Set_Name(name);
		return Open(mode);
	}
	int Open(int mode) override
	{
		Close();
		if (Filename.Is_Empty())
		{
			Error(2,0,nullptr);
		}
		Mode = mode;
        DWORD flags = FILE_ATTRIBUTE_NORMAL;
        if (mode & OPEN_OVERLAPPED_FLAG) flags |= FILE_FLAG_OVERLAPPED;
		switch (mode & 3)
		{
		case OPEN_WRITE:
			Handle = CreateFileA(Filename,GENERIC_WRITE,0,nullptr,CREATE_ALWAYS, flags,nullptr);
			break;
		case OPEN_READ:
			Handle = CreateFileA(Filename,GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING, flags,nullptr);
			break;
		case OPEN_READ_WRITE:
			Handle = CreateFileA(Filename,GENERIC_READ|GENERIC_WRITE,0,nullptr,OPEN_ALWAYS, flags,nullptr);
			break;
		default:
			errno = EINVAL;
		}

		if (Handle != INVALID_HANDLE_VALUE)
		{
			if (!(mode & OPEN_OVERLAPPED_FLAG) && (BiasStart) && (BiasLength != -1))
			{
				Seek(0, ORIGIN_START);
				CurrentOffset = BiasStart;
			}
			return true;
		}
		return false;
	}


	int Read(void* buffer, int bytesToRead) override
	{
		DWORD bytesRead = 0;
		if (!Is_Open())
		{
			if (Open(OPEN_READ))
			{
				bytesRead = Read(buffer, bytesToRead);
				Close();
			}
		}
		else
		{
			if (BiasLength != -1)
			{
				int maxBytesToRead = BiasLength - Tell();
				if (bytesToRead > maxBytesToRead)
					bytesToRead = maxBytesToRead;
			}
			
			if (!ReadFile(Handle, buffer, bytesToRead, &bytesRead, nullptr))
				Error(GetLastError(), 1, Filename);

			CurrentOffset += bytesRead;
			TT_ASSERT(BiasLength == -1 || (CurrentOffset >= BiasStart && CurrentOffset <= BiasStart + BiasLength));
		}
		
		return bytesRead;
	}


	int Seek(int offset, int origin) override
	{
		if (BiasLength != -1)
		{
			if (origin == ORIGIN_CURRENT && offset == 0)
				return CurrentOffset - BiasStart;

			switch (origin)
			{
			case ORIGIN_CURRENT: offset += CurrentOffset - BiasStart; break;
			case ORIGIN_END:     offset += BiasLength; break;
			}

			int result = Raw_Seek(BiasStart + clamp(offset, 0, BiasLength), ORIGIN_START);
			if (result != -1)
				result -= BiasStart;

			TT_ASSERT(BiasLength == -1 || (CurrentOffset >= BiasStart && CurrentOffset <= BiasStart + BiasLength));
			TT_ASSERT(result <= BiasLength);
			return result;
		}
		else
			return Raw_Seek(offset, origin);
	}

	int Tell() override
	{
		return CurrentOffset - BiasStart;
	}

	virtual int Size() override
	{
		if (BiasLength != -1)
			return BiasLength;
		else if (CachedSize != -1)
			return CachedSize;
		else
		{
			int size = -1;
			if (!Is_Open())
			{
				if (Open(OPEN_READ))
				{
					size = Size();
					Close();
				}
			}
			else
			{
				size = CachedSize = GetFileSize(Handle,nullptr);
				if (size == -1)
					Error(GetLastError(), 0, Filename);
			}
			return size;
		}
	}


	int Write(const void* buffer, int size)
	{
		DWORD bytesWritten = 0;
		if (!Is_Open())
		{
			if (Open(OPEN_WRITE))
			{
				bytesWritten = Write(buffer, size);
				Close();
			}
		}
		else
		{
			if (!WriteFile(Handle, buffer, size, &bytesWritten, nullptr))
				Error(GetLastError(), 0, Filename);

			CurrentOffset += bytesWritten;

			if (BiasLength != -1)
			{
				if (CurrentOffset > BiasStart + BiasLength)
				{
					BiasLength = CurrentOffset - BiasStart;
					CachedSize = -1;
				}
			}
			if (CachedSize != -1 && CurrentOffset > CachedSize)
				CachedSize = -1;

			TT_ASSERT(BiasLength == -1 || (CurrentOffset >= BiasStart && CurrentOffset <= BiasStart + BiasLength));
		}
		return bytesWritten;
	}


	void Close() override
	{
		if (Is_Open())
		{
			if (!CloseHandle(Handle))
				Error(GetLastError(), 0, Filename);
			
			Handle = INVALID_HANDLE_VALUE;
		}
		CurrentOffset = 0;
		CachedSize = -1;
	}


	unsigned long Get_Date_Time() override
	{
		BY_HANDLE_FILE_INFORMATION info;
		unsigned short dostime;
		unsigned short dosdate;
		if (GetFileInformationByHandle(Handle,&info))
		{
			FileTimeToDosDateTime(&info.ftLastWriteTime, &dosdate, &dostime);
			return dosdate << 0x10 | dostime;
		}
		return 0;
	}
	bool Set_Date_Time(unsigned long datetime) override
	{
		BY_HANDLE_FILE_INFORMATION info;
		FILETIME filetime;
		if (Is_Open())
		{
			if (GetFileInformationByHandle(Handle,&info))
			{
				if (DosDateTimeToFileTime((WORD)(datetime >> 0x10),(WORD)datetime,&filetime))
				{
					if (SetFileTime(Handle,&info.ftCreationTime,&filetime,&filetime))
					{
						return true;
					}
				}
			}
		}
		return false;
	}

	virtual HANDLE Get_File_Handle() const override
	{
		return Handle;
	}

	void Bias(int start, int length)
	{
		if (start == 0)
		{
			TT_ASSERT(length == -1);
			BiasStart = 0;
			BiasLength = -1;
		}
		else
		{
			BiasLength = Size();
			BiasStart += start;
			if (length != -1)
			{
				if (length < BiasLength)
					BiasLength = length;
				
				if (BiasLength < 0)
					BiasLength = 0;
			}
		}
	}
    void Get_Bias(int& start, int& length) override
    { 
        start = BiasStart;
        length = BiasLength;
    }

	virtual void Attach(HANDLE handle,int mode)
	{
		Reset();
		Handle = handle;
		Mode = mode; // TODO: Mode is currently unused and the parameter to Attach is actually wrong in a bunch of places
		BiasStart = 0;
		BiasLength = -1;
		Date = 0;
		Time = 0;
		CachedSize = -1;
	}
	virtual void Detach()
	{
		CurrentOffset = 0;
		Mode = 0;
		Handle = INVALID_HANDLE_VALUE;
		BiasStart = 0;
		BiasLength = -1;
		Date = 0;
		Time = 0;
		CachedSize = -1;
	}
}; 

class TextFileClass : public RawFileClass {
public:
	TextFileClass()
	{
	}
	TextFileClass(const char *filename) : RawFileClass(filename)
	{
	}
	bool Read_Line(StringClass &str)
	{
		str = "";
		char buf[64];
		memset(buf,0,sizeof(buf));
		bool b;
		do
		{
			int sz = Read(buf,63);
			b = (sz == 63);
			if (sz > 0)
			{
				for (int i = 0;i < sz;i++)
				{
					if (buf[i] == '\n')
					{
						buf[i + 1] = 0;
						Seek(i - sz + 1,1);
						b = false;
						break;
					}
				}
				str += buf;
			}
		} while (b);
		bool l = str.Get_Length() > 0;
		strtrim(str.Peek_Buffer());
		return l;
	}
	bool Write_Line(StringClass const &str)
	{
		int len = str.Get_Length();
		if (Write((void *)str.Peek_Buffer(),len) == len)
		{
			return Write("\r\n",2) == 2;
		}
		return false;
	}
};

class WideTextFileClass : public RawFileClass {
public:
	WideTextFileClass()
	{
	}
	WideTextFileClass(const char *filename) : RawFileClass(filename)
	{
	}
	bool Read_Line(WideStringClass &wstr)
	{
		StringClass str = "";
		char buf[64];
		memset(buf, 0, sizeof(buf));
		bool b;
		do
		{
			int sz = Read(buf, 63);
			b = (sz == 63);
			if (sz > 0)
			{
				for (int i = 0; i < sz; i++)
				{
					if (buf[i] == '\n')
					{
						buf[i + 1] = 0;
						Seek(i - sz + 1, 1);
						b = false;
						break;
					}
				}
				str += buf;
			}
		} while (b);
		wstr = FromUTF8(str);
		if (wstr[wstr.Get_Length() - 1] == L'\n')
		{
			wstr.TruncateRight(1);
		}
		if (wstr[wstr.Get_Length() - 1] == L'\r')
		{
			wstr.TruncateRight(1);
		}
		bool l = wstr.Get_Length() > 0;
		return l;
	}
	bool Write_Line(WideStringClass const &str)
	{
		StringClass s = ToUTF8(str);
		int len = s.Get_Length();
		if (Write((void *)s.Peek_Buffer(), len) == len)
		{
			static StringClass s2 = ToUTF8(L"\r\n");
			int len2 = s2.Get_Length();
			return Write((void *)s2.Peek_Buffer(), len2) == len2;
		}
		return false;
	}
	StringClass ToUTF8(WideStringClass const &wstr)
	{
		if (wstr.Is_Empty())
		{
			return "";
		}
		int size = WideCharToMultiByte(CP_UTF8, 0, wstr.Peek_Buffer(), -1, nullptr, 0, nullptr, nullptr);
		StringClass str;
		WideCharToMultiByte(CP_UTF8, 0, wstr.Peek_Buffer(), -1, str.Get_Buffer(size), size, nullptr, nullptr);
		return str;
	}
	WideStringClass FromUTF8(StringClass const &str)
	{
		if (str.Is_Empty())
		{
			return "";
		}
		int size = MultiByteToWideChar(CP_UTF8, 0, str.Peek_Buffer(), -1, nullptr, 0);
		WideStringClass wstr;
		MultiByteToWideChar(CP_UTF8, 0, str.Peek_Buffer(), -1, wstr.Get_Buffer(size), size);
		return wstr;
	}
};

#endif
