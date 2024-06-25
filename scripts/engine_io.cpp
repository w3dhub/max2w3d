#include "general.h"
#pragma warning(disable: 4073) //warning C4073: initializers put in library initialization area - That's EXACTLY why I put that pragma in...
#pragma init_seg(lib) // Move this files static initializers up a level
#pragma warning(default: 4073)
#include <shlobj.h>
#include "engine_string.h"
#include "straw.h"
#include "Crc32.h"
#include "BufferedFileClass.h"
#include "filefactoryclass.h"
#include "iniclass.h"

void SCRIPTS_API Strip_Path_From_Filename(StringClass& target, const char* fileName)
{
	if (strchr(fileName,'\\'))
	{
		const char *c = strrchr(fileName,'\\');
		c++;
		target = c;
	}
	else
	{
		target = fileName;
	}
}
FileClass SCRIPTS_API *Get_Data_File(const char *file)
{
	return _TheFileFactory->Get_File(file);
}

void SCRIPTS_API Close_Data_File(FileClass *file)
{
	_TheFileFactory->Return_File(file);
}

bool ReadFileBytes(FileClass* file, char** data, uint* size)
{
    int length = file->Size();
    char* buffer = new char[length + 1];

    if (file->Read(buffer, length) != length)
    {
        delete[] buffer;
        *data = nullptr;
        *size = 0;
        return false;
    }

    buffer[length] = '\0';
    *data = buffer;
    *size = length;
    return true;
}

bool ReadFileBytes(const char* filename, char** data, uint* size)
{
    bool result = false;

    FileClass *file = Get_Data_File(filename);
    if (file)
    {
        if (file->Open())
        {
            result = ReadFileBytes(file, data, size);
        }
        Close_Data_File(file);
    }

    return result;
}

void FreeFileBytes(char* data)
{
    delete[] data;
}

bool WriteFileBytes(FileClass* file, char* data, uint size)
{
    return file->Write(data, size) == (int)size;
}

bool WriteFileBytes(const char* filename, char* data, uint size)
{
    bool result = false;

    FileClass *file = Get_Data_File(filename);
    if (file)
    {
        if (file->Open(2))
        {
            result = WriteFileBytes(file, data, size);
        }
        Close_Data_File(file);
    }

    return result;
}

#if (TT_EXPORTS) || (W3DVIEWER) || (W3D_MAX_TOOLS)
void SetFileFactories();
void SetLanguageFileFactories();
#endif

char FilePath[MAX_PATH];
char AppDataPath[MAX_PATH];
char RegPath[MAX_PATH];
char Name[MAX_PATH];
bool PathsInit = false;
extern bool isClient;
void Read_Paths()
{
#if (TT_EXPORTS) || (W3DVIEWER) || (W3D_MAX_TOOLS)
	SetFileFactories();
#endif

	char RegBase[MAX_PATH];
	char RegClient[MAX_PATH];
	char RegFDS[MAX_PATH];
	char FileBase[MAX_PATH];
	char FileClient[MAX_PATH];
	char FileFDS[MAX_PATH];

	bool UseRenFolder = !isClient;
	INIClass *ini = Get_INI("paths.ini");
	if (ini)
	{
		ini->Get_String("paths","RegBase","Westwood",RegBase,MAX_PATH);
		ini->Get_String("paths","RegClient","Renegade",RegClient,MAX_PATH);
		ini->Get_String("paths","RegFDS","RenegadeFDS",RegFDS,MAX_PATH);
		ini->Get_String("paths","FileBase","Renegade",FileBase,MAX_PATH);
		ini->Get_String("paths","FileClient","Client",FileClient,MAX_PATH);
		ini->Get_String("paths","FileFDS","FDS",FileFDS,MAX_PATH);
		UseRenFolder = ini->Get_Bool("paths","UseRenFolder",UseRenFolder);
		Release_INI(ini);
	}
	else
	{
		strcpy(RegBase,"Westwood");
		strcpy(RegClient,"Renegade");
		strcpy(RegFDS,"RenegadeFDS");
		strcpy(FileBase,"Renegade");
		strcpy(FileClient,"Client");
		strcpy(FileFDS,"FDS");
	}
	INIClass *ini2 = Get_INI("name.ini");
	if (ini2)
	{
		ini2->Get_String("paths", "Name", "Renegade", Name, MAX_PATH);
		Release_INI(ini2);
	}
	else
	{
		strcpy(Name, "Renegade");
	}
	FilePath[0] = 0;
	if (!UseRenFolder)
	{
		wchar_t fpath[MAX_PATH];
		SHGetSpecialFolderPath(nullptr, fpath, CSIDL_MYDOCUMENTS, false);
		bool unicode = false;
		for (int i = 0;fpath[i] != 0;i++)
		{
			unsigned short value = fpath[i];
			if (value > 255)
			{
				unicode = true;
				break;
			}
		}
		if (unicode)
		{
			GetModuleFileNameA(nullptr,FilePath,MAX_PATH);
			strrchr(FilePath,'\\')[0] = 0;
		}
		else
		{
			SHGetSpecialFolderPathA(nullptr,  FilePath, CSIDL_MYDOCUMENTS, false);
		}
	}
	else
	{
		GetModuleFileNameA(nullptr,FilePath,MAX_PATH);
		strrchr(FilePath,'\\')[0] = 0;
	}
	strcat(FilePath,"\\");
	strcat(FilePath,FileBase);
	strcat(FilePath,"\\");
	strcat(FilePath,isClient ? FileClient : FileFDS);
	strcat(FilePath,"\\");

	AppDataPath[0] = 0;
	if (!UseRenFolder)
	{
		wchar_t fpath[MAX_PATH];
		SHGetSpecialFolderPath(nullptr, fpath, CSIDL_MYDOCUMENTS, false);
		bool unicode = false;
		for (int i = 0;fpath[i] != 0;i++)
		{
			unsigned short value = fpath[i];
			if (value > 255)
			{
				unicode = true;
				break;
			}
		}
		if (unicode)
		{
			GetModuleFileNameA(nullptr,AppDataPath,MAX_PATH);
			strrchr(AppDataPath,'\\')[0] = 0;
		}
		else
		{
			SHGetSpecialFolderPathA(nullptr, AppDataPath, CSIDL_APPDATA, false);
		}
	}
	else
	{
		GetModuleFileNameA(nullptr,AppDataPath,MAX_PATH);
		strrchr(AppDataPath,'\\')[0] = 0;
	}
	strcat(AppDataPath,"\\");
	strcat(AppDataPath,FileBase);
	strcat(AppDataPath,"\\");
	strcat(AppDataPath,isClient ? FileClient : FileFDS);
	strcat(AppDataPath,"\\");

	strcpy(RegPath,"Software\\");
	strcat(RegPath,RegBase);
	strcat(RegPath,"\\");
	strcat(RegPath,isClient ? RegClient : RegFDS);

#if (TT_EXPORTS)
	SetLanguageFileFactories();
#endif
	PathsInit = true;
}
const char SCRIPTS_API *Get_File_Path()
{
	if (!PathsInit)
	{
		Read_Paths();
	}
	return FilePath;
}
const char SCRIPTS_API *Get_App_Data_Path()
{
	if (!PathsInit)
	{
		Read_Paths();
	}
	return AppDataPath;
}
const char SCRIPTS_API *Get_Registry_Path()
{
	if (!PathsInit)
	{
		Read_Paths();
	}
	return RegPath;
}
const char SCRIPTS_API *Get_Name()
{
	if (!PathsInit)
	{
		Read_Paths();
	}
	return Name;
}

INIClass SCRIPTS_API *Get_INI(char const *filename)
{
	INIClass *ini = nullptr;
	FileClass *f = _TheFileFactory->Get_File(filename);
	if (f)
	{
		if (f->Is_Available())
		{
			ini = new INIClass(*f);
		}
		_TheFileFactory->Return_File(f);
	}
	return ini;
}

void SCRIPTS_API Release_INI(INIClass *ini)
{
	if (ini)
	{
		delete ini;
	}
}

void SCRIPTS_API Save_INI(INIClass *ini,const char *filename)
{
	FileClass *f = _TheWritingFileFactory->Get_File(filename);
	if (f)
	{
		ini->Save(*f);
		_TheWritingFileFactory->Return_File(f);
		_TheFileFactory->Rescan(); // force a rescan, otherwise loading this config afterwards will fail (or point to the original)
	}
}

int INIClass::Load(char* filename)
{
	file_auto_ptr ptr(_TheFileFactory, filename);
	int ret = Load(*ptr);
	if (Filename)
	{
		delete[] Filename;
	}
	Filename = newstr(filename);
	return ret;
}

int INIClass::Section_Count() const
{
	return SectionIndex->Count();
}

int INIClass::Load(FileClass& file)
{
	FileStraw straw(file);
	if (Filename)
	{
		delete[] Filename;
	}
	Filename = newstr(file.File_Name());
	return Load(straw);
}

bool INIClass::Put_Wide_String(const char* section, const char* entry, const wchar_t* string)
{
	if (section && entry && string)
	{
		WideStringClass str(string);
		int length = str.Get_Length();
		if (length)
		{
			char *buffer = new char[8 * length + 32];
			BufferStraw bstraw((void *)string, 2 * length + 2);
			Base64Straw b64straw(Base64Straw::ENCODE);
			b64straw.Get_From(&bstraw);
			int count = 0;
			int size;
			do
			{
				size = b64straw.Get(&buffer[count], 16);
				count += size;
			} while (size);
			buffer[count] = 0;
			Put_String(section, entry, buffer);
			delete[] buffer;
		}
		else
		{
			Put_String(section, entry, "");
		}
		return true;
	}
	return false;
}

WideStringClass &INIClass::Get_Wide_String(WideStringClass& string, const char* section, const char* entry, const wchar_t *defaultvalue) const
{
	char result[1024];
	wchar_t buffer[1024];
	Base64Pipe b64pipe(Base64Pipe::DECODE);
	BufferPipe bpipe(buffer, sizeof(buffer));
	b64pipe.Put_To(&bpipe);
	int size = Get_String(section, entry, "", result, 1024);
	if (size)
	{
		b64pipe.Put(result, size);
		b64pipe.Flush();
		string = buffer;
	}
	else if (defaultvalue)
	{
		string = defaultvalue;
	}
	return string;
}

int Read_Line(Straw& straw,char* line,int lineSize,bool& isLast)
{
		if (!line || lineSize == 0)
				return 0;
		int i;
		for (i = 0;;)
		{
				char c;
				int getResult = straw.Get(&c, 1);
				if (getResult != 1)
				{
						isLast = true;
						break;
				}
				if (c == '\n')
						break;
				if (c != '\r' && i + 1 < lineSize)
						line[i++] = c;
		}
		line[i] = '\0';
		strtrim(line);
		return (int)strlen(line);
}

int INIClass::Load(Straw& straw)
{
		bool isLastLine = false;
		CacheStraw cacheStraw(4096);
		cacheStraw.Get_From(&straw);
		char line[512];
 
		// Ignore everything above first section (indicated by a line like "[sectionName]")
		while (!isLastLine)
		{
				Read_Line(cacheStraw, line, 512, isLastLine);
				if (isLastLine)
						return false;
				if (line[0] == '[' && strchr(line, ']'))
						break;
		}
 
		if (Section_Count() > 0)
		{
				while (!isLastLine)
				{
						char* sectionEnd = strchr(line, ']');
						TT_ASSERT(line[0] == '[' && sectionEnd && (sectionEnd - line) < 64); // at start of section
						line[0] = ' ';
						*sectionEnd = '\0';
						strtrim(line);
						char sectionName[64];
						strcpy_s(sectionName, line);
						while (!isLastLine)
						{
								int count = Read_Line(cacheStraw, line, 512, isLastLine);
								if (line[0] == '[' && strchr(line, ']'))
										break;
								Strip_Comments(line);
								if (count)
								{
										if (line[0] != ';')
										{
												if (line[0] != '=')
												{
														char* delimiter = strchr(line, '=');
														if (delimiter)
														{
																*delimiter = '\0';
																char* key = line;
																char* value = delimiter + 1;
																strtrim(key);
																if (key[0] != '\0')
																{
																		strtrim(value);
																		if (value[0] == '\0')
																		{
																				continue;
																		}
																		if (!Put_String(sectionName, key, value))
																				return false;
																}
														}
												}
										}
								}
						}
				}
		}
		else
		{
				while (!isLastLine)
				{
						TT_ASSERT(line[0] == '[' && strchr(line, ']')); // at start of section
						line[0] = ' ';
						*strchr(line, ']') = '\0';
						strtrim(line);
						INISection* section = new INISection(newstr(line));
						if (!section)
						{
								Clear(nullptr, nullptr);
								return false;
						}
						while (!isLastLine)
						{
								int count = Read_Line(cacheStraw, line, 512, isLastLine);
								if (line[0] == '[' && strchr(line, ']'))
										break;
								Strip_Comments(line);
								char* delimiter = strchr(line, '=');
								if (count)
								{
										if (line[0] != ';')
										{
												if (line[0] != '=')
												{
														if (delimiter)
														{
																*delimiter = '\0';
																char* key = line;
																char* value = delimiter + 1;
																strtrim(key);
																if (key[0] != '\0')
																{
																		strtrim(value);
																		if (value[0] == '\0')
																		{
																				continue;
																		}
																		INIEntry* entry = new INIEntry(newstr(key), newstr(value));
																		if (!entry)
																		{
																				delete section;
																				Clear(nullptr, nullptr);
																				return false;
																		}
																		uint32 crc = CRC_String(entry->Entry, 0);
																		if (section->EntryIndex.Is_Present(crc))
																				DuplicateCRCError(__FUNCTION__, section->Section, line);
																		section->EntryIndex.Add_Index(crc, entry);
																		section->EntryList.Add_Tail(entry);
																}
														}
												}
										}
								}
						}
						if (section->EntryList.Is_Empty())
						{
								delete section;
						}
						else
						{
								uint32 crc = CRC_String(section->Section, 0);
								SectionIndex->Add_Index(crc, section);
								SectionList->Add_Tail(section);
						}
				}
		}
		return true;
}

StringClass &INIClass::Get_String(StringClass& string, const char* section, const char* entry, const char *defaultvalue) const
{
	const char *value = defaultvalue;
	if (!section || !entry)
	{
		string = "";
	}
	INIEntry *Entry = Find_Entry(section,entry);
	if (Entry)
	{
		value = Entry->Value;
	}
	if (value)
	{
		string = value;
	}
	else
	{
		string = "";
	}
	return string;
}

std::string INIClass::Get_String(const char* section, const char* entry, const char *defaultvalue) const
{
	const char *value = defaultvalue;
	INIEntry *Entry = Find_Entry(section, entry);
	if (Entry)
	{
		value = Entry->Value;
	}
	return std::string(value ? value : "");
}

INISection *INIClass::Find_Section(const char* section) const
{
	if (section)
	{
		int crc = CRC_String(section,0);
		if (SectionIndex->Is_Present(crc))
		{
			return (*SectionIndex)[crc];
		}
	}
	return nullptr;
}
INIEntry *INIClass::Find_Entry(const char* section,const char* entry) const
{
	INISection *Section = Find_Section(section);
	if (Section)
	{
		return Section->Find_Entry(entry);
	}
	return nullptr;
}
int INIClass::Get_Int(char const *section,char const *entry,int defaultvalue) const
{
	if (section)
	{
		if (entry)
		{
			INIEntry *Entry = Find_Entry(section,entry);
			if (Entry)
			{
				if (Entry->Value)
				{
					int *value;
					const char *pattern;
					if (Entry->Value[0] == '$')
					{
						value = &defaultvalue;
						pattern = "$%x";
					}
					else
					{
						if (tolower(Entry->Value[strlen(Entry->Value) - 1]) != 'h')
						{
							return atoi(Entry->Value);
						}
						value = &defaultvalue;
						pattern = "%xh";
					}
#pragma warning(suppress: 6031) //warning C6031: return value ignored
					sscanf(Entry->Value, pattern, value);
				}
			}
		}
	}
	return defaultvalue;
}
float INIClass::Get_Float(char const *section,char const *entry,float defaultvalue) const
{
	if (section)
	{
		if (entry)
		{
			INIEntry *Entry = Find_Entry(section,entry);
			if (Entry)
			{
				if (Entry->Value)
				{
					float c = defaultvalue;
#pragma warning(suppress: 6031) //warning C6031: return value ignored
					sscanf(Entry->Value, "%f", &c);
					defaultvalue = c;
					if (strchr(Entry->Value, '%'))
					{
						defaultvalue = defaultvalue / 100.0f;
					}
				}
			}
		}
	}
	return defaultvalue;
}
bool INIClass::Get_Bool(char const *section,char const *entry,bool defaultvalue) const
{
	if (section)
	{
		if (entry)
		{
			INIEntry *Entry = Find_Entry(section,entry);
			if (Entry)
			{
				if (Entry->Value)
				{
					switch ( toupper(Entry->Value[0]) )
					{
						case '1':
						case 'T':
						case 'Y':
							return true;
					case '0':
						case 'F':
						case 'N':
							return false;
					}
				}
			}
		}
	}
	return defaultvalue;
}
int INIClass::Get_String(char const *section,char const *entry,char const *defaultvalue,char *result,int size) const
{
	if (!result || size <= 1 || !section || !entry)
	{
		return 0;
	}
	INIEntry *Entry = Find_Entry(section,entry);
	const char *value = defaultvalue;
	if (Entry)
	{
		if (Entry->Value)
		{
			value = Entry->Value;
		}
	}
	if (!value)
	{
		result[0] = 0;
		return 0;
	}
	strncpy(result, value, size);
	result[size - 1] = 0;
	strtrim(result);
	return (int)strlen(result);
}
int INIClass::Entry_Count(char const *section) const
{
	INISection *Section = Find_Section(section);
	if (Section)
	{
		return Section->EntryIndex.Count();
	}
	return 0;
}
const char *INIClass::Get_Entry(char const *section,int index) const
{
	int count = index;
	INISection *Section = Find_Section(section);
	if (Section && index < Section->EntryIndex.Count())
	{
		for (INIEntry *i = Section->EntryList.First();i; i = i->Next())
		{
			if (!i->Is_Valid())
			{
				break;
			}
			if (!count)
			{
				return i->Entry;
			}
			count--;
		}
	}
	return nullptr;
}
void INIClass::Initialize()
{
	SectionList = new List<INISection *>;
	SectionIndex = new IndexClass<int,INISection *>;
	Filename = newstr("<unknown>");
}
void INIClass::Shutdown()
{
	if (SectionList)
	{
		delete SectionList;
	}
	if (SectionIndex)
	{
		delete SectionIndex;
	}
	if (Filename)
	{
		delete[] Filename;
	}
}
bool INIClass::Clear(const char* section,const char* entry)
{
	if (section)
	{
		INISection *Section = Find_Section(section);
		if (Section)
		{
			if (entry)
			{
				INIEntry *Entry = Section->Find_Entry(entry);
				if (!Entry)
				{
					return true;
				}
				Section->EntryIndex.Remove_Index(CRC_String(Entry->Entry,0));
				delete Entry;
				return true;
			}
			else
			{
				SectionIndex->Remove_Index(CRC_String(Section->Section,0));
				delete Section;
				return true;
			}
		}
	}
	else
	{
		SectionList->Delete();
		SectionIndex->Clear();
		delete[] Filename;
		Filename = newstr("<unknown>");
	}
	return true;
}
int INIClass::CRC(char *string)
{
	return CRC_String(string,0);
}

INIClass::INIClass()
{
	Filename = nullptr;
	Initialize();
}

INIClass::INIClass(FileClass &file)
{
	Filename = nullptr;
	Initialize();
	Load(file);
}

INIClass::~INIClass()
{
	Clear(nullptr,nullptr);
	Shutdown();
}

uint INIClass::Get_Color_UInt(char const *section, char const *entry, uint defaultvalue) const
{
	char buf[256], hex[11];

	snprintf(buf, sizeof(buf), "%sHex",entry);
	this->Get_String(section, buf, "0xNotValid", hex, sizeof(hex));
	if (strcmp(hex, "0xNotValid") != 0) // We've got us a supposedly valid hex value
	{
		uint color;
		int res = sscanf(hex, "%x", &color);
		if (res == 1) return color; // Yay, we've got a color. Party!
	};

	snprintf(buf, sizeof(buf), "%sAlpha",entry);
	uint a = this->Get_Int(section, buf, (defaultvalue >> 24) & 0xFF);
	snprintf(buf, sizeof(buf), "%sRed",entry);
	uint r = this->Get_Int(section, buf, (defaultvalue >> 16) & 0xFF);
	snprintf(buf, sizeof(buf), "%sGreen",entry);
	uint g = this->Get_Int(section, buf, (defaultvalue >> 8) & 0xFF);
	snprintf(buf, sizeof(buf), "%sBlue",entry);
	uint b = this->Get_Int(section, buf, defaultvalue & 0xFF);

	return ((a & 0xFF) << 24) | ((r & 0xFF) << 16) | ((g & 0xFF) << 8) | (b & 0xFF);
};

void INIClass::Strip_Comments(char* buffer)
{
	if (buffer)
	{
		char *buf = strchr(buffer,';');
		if (buf)
		{
			buf[0] = 0;
			strtrim(buffer);
		}
	}
}

void INIClass::DuplicateCRCError(const char *function,const char* section,const char* entry)
{
	wchar_t OutputString[512];
	_snwprintf(OutputString,512,L"%hs - Duplicate Entry \"%hs\" in section \"%hs\" (%hs)\n",function,entry,section,Filename);
	OutputString[511] = 0;
	OutputDebugString(OutputString);
	MessageBox(nullptr,OutputString,L"Duplicate CRC in INI file.",16);
}

int INIClass::Save(FileClass& file)
{
	FilePipe pipe(&file);
	if (Filename)
	{
		delete[] Filename;
	}
	Filename = newstr(file.File_Name());
	return Save(pipe);
}

int INIClass::Save(Pipe& pipe)
{
	int pos = 0;
	for (INISection *i = SectionList->First();i;i = i->Next())
	{
		if (!i->Is_Valid())
		{
			break;
		}
		int i1 = pipe.Put("[",1) + pos;
		int i2 = pipe.Put(i->Section,(int)strlen(i->Section)) + i1;
		int i3 = pipe.Put("]",1) + i2;
		int i4 = pipe.Put("\n",(int)strlen("\n")) + i3;
		for (INIEntry *j = i->EntryList.First();j;j = j->Next())
		{
			if (!j->Is_Valid())
			{
				break;
			}
			int i5 = pipe.Put(j->Entry,(int)strlen(j->Entry)) + i4;
			int i6 = pipe.Put("=",1) + i5;
			int i7 = pipe.Put(j->Value,(int)strlen(j->Value)) + i6;
			i4 = pipe.Put("\n",(int)strlen("\n")) + i7;
		}
		pos = pipe.Put("\n",(int)strlen("\n")) + i4;
	}
	return pipe.End() + pos;
}

bool INIClass::Put_String(const char* section, const char* entry, const char* string)
{
	if (!section || !entry)
	{
		return false;
	}
	INISection *sec = Find_Section(section);
	if (!sec)
	{
		sec = new INISection(newstr(section));
		SectionList->Add_Tail(sec);
		SectionIndex->Add_Index(CRC_String(sec->Section,0),sec);
	}
	INIEntry *ent = sec->Find_Entry(entry);
	if (ent)
	{
		if (!strcmp(ent->Entry,entry))
		{
			DuplicateCRCError("INIClass::Put_String",section,entry);
		}
		SectionIndex->Remove_Index(CRC_String(ent->Entry,0));
		delete ent;
	}
	if (string && *string)
	{
		ent = new INIEntry(newstr(entry),newstr(string));
		sec->EntryList.Add_Tail(ent);
		sec->EntryIndex.Add_Index(CRC_String(ent->Entry,0),ent);
	}
	return true;
}

bool INIClass::Put_Int(const char* section, const char* entry, int value, int format)
{
	char *form;
	if (format == 1)
	{
		form = "%Xh";
	}
	else
	{
		if (format > 1 && format == 2)
		{
			form = "$%X";
		}
		else
		{
			form = "%d";
		}
	}
	char buf[524];
	snprintf(buf, sizeof(buf),form,value);
	return Put_String(section,entry,buf);
}

bool INIClass::Put_Bool(const char* section, const char* entry, bool value)
{
	char *str;
	if (value)
	{
		str = "yes";
	}
	else
	{
		str = "no";
	}
	return Put_String(section,entry,str);
}

bool INIClass::Put_Float(const char* section, const char* entry, float value)
{
	char buf[524];
	snprintf(buf, sizeof(buf),"%f",value);
	return Put_String(section,entry,buf);
}

//Thanks to OmniBlade and CCHyper for these 2:
bool INIClass::Put_TextBlock(const char *section, char const *text)
{
	assert(text != nullptr);

	char entry[32];
	char buffer[76];

	if (section != nullptr && text != nullptr)
	{
		Clear(section,nullptr);
		char const *block_ptr = text;
		for (int line = 1; *block_ptr != '\0'; line++)
		{
			strncpy(buffer, block_ptr, sizeof(buffer));
			buffer[sizeof(buffer) - 1] = 0;
			snprintf(entry, sizeof(entry), "%d", line);
			size_t block_len = strlen(buffer);
			if (block_len <= 0)
			{
				break;
			}
			strtrim(buffer);
			Put_String(section, entry, buffer);
			block_ptr += block_len;
		}
		return true;
	}
	return false;
}

int INIClass::Get_TextBlock(char const *section, char *buffer, int length) const
{
	assert(buffer != nullptr);
	assert(length > 0);

	int total = 0;
	if (section != nullptr && buffer != nullptr && length > 0)
	{
		buffer[0] = '\0';
		int elen = Entry_Count(section);
		for (int i = 0; i < elen && length > 1; ++i)
		{
			if (i > 0)
			{
				//Puts a space between lines
				*buffer++ = ' ';
				--length;
			}
			Get_String(section, Get_Entry(section, i), nullptr, buffer, length);
			total = (int)strlen(buffer);
			length -= total;
			buffer += total;
		}
	}
	return total;
}
unsigned int SCRIPTS_API Get_Registry_Int(const char *entry,int defaultvalue)
{
	HKEY key;
	RegOpenKeyExA(HKEY_CURRENT_USER,Get_Registry_Path(),0,KEY_READ,&key);
	unsigned int value = 0;
	unsigned long size = 4;
	unsigned long type;
	LONG error = RegQueryValueExA(key,entry,nullptr,&type,(BYTE *)&value,&size);
	if (error == ERROR_FILE_NOT_FOUND)
	{
		value = defaultvalue;
	}
	RegCloseKey(key);
	return value;
}

file_auto_ptr::file_auto_ptr(FileFactoryClass* fac, const char* filename)
{
	_Fac = fac;
	_Ptr = fac->Get_File(filename);
	if (!_Ptr)
	{
		_Ptr = new BufferedFileClass();
	}
}

file_auto_ptr::~file_auto_ptr()
{
	_Fac->Return_File(_Ptr);
}
