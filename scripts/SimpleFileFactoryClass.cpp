#include "General.h"
#include "SimpleFileFactoryClass.h"
#include "BufferedFileClass.h"

void SimpleFileFactoryClass::Get_Sub_Directories(DynamicVectorClass<StringClass>& list) const
{
    CriticalSectionClass::LockClass lock(Mutex);
    if (subdirectories.size() != 0)
    {
        char* subdir = newstr(subdirectories.c_str());

        for (const char* token = strtok(subdir, ";"); token; token = strtok(nullptr, ";"))
        {
            list.Add(token);
        }

        delete[] subdir;
    }
}

void SimpleFileFactoryClass::Set_Sub_Directory(const char *sub_directory)
{
	CriticalSectionClass::LockClass lock(Mutex);
	if (sub_directory)
	{
        subdirectories = sub_directory;
	}
}

void SimpleFileFactoryClass::Append_Sub_Directory(const char *sub_directory)
{
	char temp_sub_dir[1024];
	size_t len = strlen(sub_directory);
	if (len <= 1022 && len >= 1)
	{
		strcpy(temp_sub_dir,sub_directory);
		if (temp_sub_dir[len - 1] != '\\')
		{
			temp_sub_dir[len] = '\\';
			temp_sub_dir[len + 1] = 0;
		}

		CriticalSectionClass::LockClass lock(Mutex);
        if (subdirectories.size() && subdirectories[subdirectories.size() - 1] != ';')
		{
            subdirectories += ';';
		}
        subdirectories += temp_sub_dir;
	}
}

void SimpleFileFactoryClass::Prepend_Sub_Directory(const char *sub_directory)
{
	char temp_sub_dir[1024];
	size_t len = strlen(sub_directory);
	if (len <= 1021 && len >= 1)
	{
		strcpy(temp_sub_dir,sub_directory);
		if (temp_sub_dir[len - 1] != '\\')
		{
			temp_sub_dir[len] = '\\';
			temp_sub_dir[len + 1] = 0;
			len++; // adjust len so that ; gets put in the correct place
		}
		temp_sub_dir[len] = ';';
		temp_sub_dir[len + 1] = 0;

		CriticalSectionClass::LockClass lock(Mutex);
        subdirectories = temp_sub_dir + subdirectories;
	}
}

SimpleFileFactoryClass::SimpleFileFactoryClass(const char* path) : IsStripPath(false)
{
    if (path)
        Append_Sub_Directory(path);
}

static bool Is_Full_Path(const char *path)
{
	if (!path || path[0] == '\0')
		return false;

	return path[1] == ':' || (path[0] == '\\' && path[1] == '\\');
}

FileClass* SimpleFileFactoryClass::Get_File(const char* filename)
{
    const char* stripped_name;

    if (IsStripPath)
    {
        const char* separator = strrchr(filename, '/');
        if (separator)
            stripped_name = separator + 1;
        else
            stripped_name = filename;
    }
    else
        stripped_name = filename;

    std::string new_name = stripped_name;

    BufferedFileClass *file = new BufferedFileClass;
    if (!Is_Full_Path(stripped_name))
    {
        CriticalSectionClass::LockClass lock(Mutex);
        if (subdirectories.size() != 0)
        {
            if (strchr(subdirectories.c_str(), ';'))
            {
                char* subdir = newstr(subdirectories.c_str());

                for (const char* token = strtok(subdir, ";"); token; token = strtok(nullptr, ";"))
                {
                    new_name = token;
                    new_name.append(stripped_name);
                    file->Set_Name(new_name.c_str());
                    if (file->Is_Available(0)) break;
                }

                delete[] subdir;
            }
            else
            {
                new_name = subdirectories + stripped_name;
            }
        }
	}

    file->Set_Name(new_name.c_str());

    return file;
}

void SimpleFileFactoryClass::Return_File(FileClass* file)
{
	delete file;
}
