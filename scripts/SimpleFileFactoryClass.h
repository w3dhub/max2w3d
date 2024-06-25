#pragma once

#include "filefactoryclass.h"
#include "CriticalSectionClass.h"
#include <string>

class SimpleFileFactoryClass : public FileFactoryClass {
public:
    std::string subdirectories;
	bool IsStripPath;
	mutable CriticalSectionClass Mutex;
	SimpleFileFactoryClass(const char* path = nullptr);
	virtual ~SimpleFileFactoryClass() {};
	virtual FileClass* Get_File    (const char* filename);
	virtual void       Return_File (FileClass* file);
	virtual void Get_Sub_Directories(DynamicVectorClass<StringClass>& list) const;
	void Set_Sub_Directory(const char* sub_directory);
	void Append_Sub_Directory(const char* sub_directory);
	void Prepend_Sub_Directory(const char* sub_directory);
	void Set_Strip_Path(bool strip)
	{
		IsStripPath = strip;
	}
	bool Get_Strip_Path() const
	{
		return IsStripPath;
	}
};

