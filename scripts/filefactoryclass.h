#pragma once
class FileClass;
#include "vector.h"
class StringClass;
class FileFactoryClass {
public:
	virtual ~FileFactoryClass() {};
	virtual FileClass* Get_File(const char* Filename) = 0;
	virtual void Return_File(FileClass* File) = 0;
	virtual void Build_Filename_List(DynamicVectorClass<StringClass>& list) { };
	virtual void Rescan() {}; // force a rescan to detect new files (e.g. when we just saved something to disk)
	virtual void Get_Sub_Directories(DynamicVectorClass<StringClass>& list) const {} // only implemented for file factories targeting folders (Simple/Loose)
}; // 0004
class SCRIPTS_API file_auto_ptr
{

	FileClass* _Ptr;
	FileFactoryClass* _Fac;

public:

	file_auto_ptr(FileFactoryClass* fac, const char* filename);
	~file_auto_ptr();

	FileClass* operator ->() { return _Ptr; }
	operator FileClass *() { return _Ptr; }
	FileClass& operator*() { return *_Ptr; }
	FileClass* get() { return _Ptr; }
};
extern SHARED_API FileFactoryClass *_TheWritingFileFactory;
extern SHARED_API FileFactoryClass *_TheFileFactory;
