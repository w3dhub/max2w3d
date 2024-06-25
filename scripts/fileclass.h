#pragma once
class StringClass;

class FileClass {
public:
	// TODO: these should be enum classes and used as parameter types, but I'd need to touch dozens of files to fix that up
	enum SeekOrigin {
		ORIGIN_START = FILE_BEGIN,
		ORIGIN_CURRENT = FILE_CURRENT,
		ORIGIN_END = FILE_END
	};
	enum OpenMode {
		OPEN_READ_WRITE = 0,
		OPEN_READ = 1,
		OPEN_WRITE = 2,
		OPEN_OVERLAPPED_FLAG = 4
	};
	virtual ~FileClass()
	{
	}
	virtual const char *File_Name() const = 0;
	virtual const char *Set_Name(const char* name) = 0;
	virtual bool Create() = 0;
	virtual bool Delete() = 0;
	virtual bool Is_Available(int handle = 0) = 0;
	virtual bool Is_Open() const = 0;
	virtual int Open(const char* name, int mode = 1) = 0;
	virtual int Open(int mode = 1) = 0;
	virtual int Read(void* buffer, int size) = 0;
	virtual int Seek(int offset, int origin) = 0;
	virtual int Tell()
	{
		return Seek(0, ORIGIN_CURRENT);
	}
	virtual int Size() = 0;
	virtual int Write(const void* buffer, int size) = 0;
	virtual void Close() = 0;
	virtual unsigned long Get_Date_Time()
	{
		return 0;
	}
	virtual bool Set_Date_Time(unsigned long time)
	{
		return false;
	}
	virtual void Error(int a, int b, const char *c) = 0;
	virtual HANDLE Get_File_Handle() const
	{
		return INVALID_HANDLE_VALUE;
	}
	virtual void Bias(int start, int length) = 0;
	virtual bool Is_Hash_Checked() const = 0;
	virtual void Get_Bias(int& start, int& length) { start = 0; length = 0; }
};
bool ReadFileBytes(FileClass* file, char** data, uint* size);
bool ReadFileBytes(const char* filename, char** data, uint* size);
void FreeFileBytes(char* data);
FileClass SCRIPTS_API *Get_Data_File(const char *file); //Open a file using the mix file opening logic
void SCRIPTS_API Close_Data_File(FileClass *file); //Close a file that was opened with Get_Data_File
void SCRIPTS_API Strip_Path_From_Filename(StringClass& target, const char* fileName);
