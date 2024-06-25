#pragma once
#include "listnode.h"
#include "index.h"
#include "engine_string.h"
#include <string>
#include "Crc32.h"
class Straw;
class Pipe;
struct INIEntry : public Node<INIEntry *>
{
	char* Entry; // 000C
	char* Value; // 0010
	~INIEntry()
	{
		delete[] Entry;
		Entry = nullptr;
		delete[] Value;
		Value = nullptr;
	}
	INIEntry(char *entry, char *value) : Entry(entry), Value(value)
	{
	}
	int Index_ID()
	{
		return CRC_String(Entry, 0);
	}
}; // 0014
struct INISection : public Node<INISection *>
{
	char* Section; // 000C
	List<INIEntry *> EntryList; // 0010
	IndexClass<int, INIEntry *> EntryIndex; // 002C
	~INISection()
	{
		delete[] Section;
		EntryList.Delete();
	}
	INIEntry *Find_Entry(const char *entry)
	{
		if (entry)
		{
			int crc = CRC_String(entry, 0);
			if (EntryIndex.Is_Present(crc))
			{
				return EntryIndex[crc];
			}
		}
		return nullptr;
	}
	INISection(char *section) : Section(section)
	{
	}
	int Index_ID()
	{
		return CRC_String(Section, 0);
	}
}; // 0040
class SCRIPTS_API INIClass {
	List<INISection *> *SectionList;
	IndexClass<int, INISection *> *SectionIndex;
	char *Filename;
public:
	static void Strip_Comments(char* buffer);
	static int CRC(char *string);
	void DuplicateCRCError(const char *function, const char* message, const char* entry);
	INIClass();
	INIClass(FileClass &file);
	void Initialize();
	void Shutdown();
	bool Clear(const char* section, const char* entry);
	int Get_Int(char const *section, char const *entry, int defaultvalue) const;
	uint Get_Color_UInt(char const *section, char const *entry, uint defaultvalue) const;
	float Get_Float(char const *section, char const *entry, float defaultvalue) const;
	bool Get_Bool(char const *section, char const *entry, bool defaultvalue) const;
	int Get_String(char const *section, char const *entry, char const *defaultvalue, char *result, int size) const;
	StringClass &Get_String(StringClass &str, const char* section, const char* entry, const char* defaultvalue = nullptr) const;
	WideStringClass &Get_Wide_String(WideStringClass &str, const char* section, const char* entry, const wchar_t* defaultvalue = nullptr) const;
	std::string Get_String(const char* section, const char* entry, const char* defaultvalue = nullptr) const;
	int Get_TextBlock(char const *section, char *buffer, int length) const;
	bool Put_Wide_String(const char* section, const char* entry, const wchar_t* string);
	bool Put_String(const char* section, const char* entry, const char* string);
	bool Put_Int(const char* section, const char* entry, int value, int format);
	bool Put_Bool(const char* section, const char* entry, bool value);
	bool Put_Float(const char* section, const char* entry, float value);
	bool Put_TextBlock(char const *section, char const *text);
	int Entry_Count(char const *section) const;
	const char *Get_Entry(char const *section, int index) const;
	INIEntry *Find_Entry(const char* section, const char* entry) const;
	INISection *Find_Section(const char* section) const;
	virtual ~INIClass();
	int Load(Straw& ffile);
	int Load(FileClass& file);
	int Load(char* filename);
	int Save(FileClass& file);
	int Save(Pipe& pipe);
	int Section_Count() const;
	bool Is_Present(const char *section, const char *entry) const
	{
		if (entry)
		{
			return Find_Entry(section, entry) != nullptr;
		}
		else
		{
			return Find_Section(section) != nullptr;
		}
	}
	bool Section_Present(const char *section) const
	{
		return Find_Section(section) != nullptr;
	}
	List<INISection *> &Get_Section_List()
	{
		return *SectionList;
	}
	IndexClass<int, INISection *>&Get_Section_Index()
	{
		return *SectionIndex;
	}

	const char* Get_File_Name() const { return Filename; }
};

INIClass SCRIPTS_API *Get_INI(char const *filename); //Open an INI file (reading using the normal mix file opening logic) and read stuff from it
void SCRIPTS_API Release_INI(INIClass *ini); //Close an INI file opened with Get_INI
void SCRIPTS_API Save_INI(INIClass *ini, const char *filename); //Save an INI file
