#pragma once
#include "fileclass.h"
#include "engine_string.h"
#include "w3d.h"
#include <vector>
#include <stdarg.h>

class XMLWriter
{
private:
	FileClass* File;
	std::vector<StringClass> Tags;
	bool StartTagOpen;
	bool TagClosed;
	int ContainsData;
	unsigned int PaddingOffset;
public:
	int WriteFile(char* buffer, int size)
	{
		return size == File->Write(buffer, size) ? size : 0;
	}
	void SetPaddingOffset(unsigned int offset)
	{
		PaddingOffset = offset;
	}
	bool WriteFormatted(const char* format, ...)
	{
		va_list va;
		va_start(va, format);
		StringClass buffer;
		buffer.Format_Args(format, va);
		return WriteFile(buffer.Peek_Buffer(), buffer.Get_Length()) == buffer.Get_Length();
	}
	bool SetStringAttribute(const char* name, const char* value)
	{
		return WriteFormatted(" %s=\"%s\"", name, value);
	}
	bool SetIntAttribute(const char* name, int value)
	{
		return WriteFormatted(" %s=\"%d\"", name, value);
	}
	bool SetUnsignedIntAttribute(const char* name, unsigned int value)
	{
		return WriteFormatted(" %s=\"%d\"", name, value);
	}
	bool SetFloatAttribute(const char* name, float value)
	{
		return WriteFormatted(" %s=\"%f\"", name, value);
	}
	bool SetBoolAttribute(const char* name, bool value)
	{
		char* str = "true";
		if (!value)
		{
			str = "false";
		}
		return WriteFormatted(" %s=\"%s\"", name, str);
	}
	bool WritePadding()
	{
		if (!WriteFormatted("\n"))
		{
			return false;
		}
		for (int i = 1; i < PaddingOffset + Tags.size(); i++)
		{
			if (!WriteFormatted("\t"))
			{
				return false;
			}
		}
		return true;
	}
	bool EndTag()
	{
		StartTagOpen = false;
		if (ContainsData)
		{
			return WriteFormatted(">");
		}
		ContainsData = true;
		Tags.pop_back();
		TagClosed = true;
		return WriteFormatted("/>");
	}
	bool WriteClosingTag()
	{
		if ((TagClosed && !WritePadding()) || !WriteFormatted("</%s>", Tags.back()))
		{
			return false;
		}
		Tags.pop_back();
		TagClosed = true;
		return true;
	}
	XMLWriter(FileClass *file, bool writedeceleration) : File(file), StartTagOpen(false), TagClosed(false), ContainsData(true), PaddingOffset(0)
	{
		if (writedeceleration)
		{
			WriteFormatted("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
		}
	}
	bool StartTag(const char* tag, int containsdata)
	{
		Tags.push_back(tag);
		ContainsData = containsdata;
		TagClosed = false;
		StartTagOpen = true;
		return WritePadding() && WriteFormatted("<%s", tag);
	}
	bool WriteVector(const char* tag, W3dVectorStruct value)
	{
		return StartTag(tag, 0) && SetFloatAttribute("X", value.X) && SetFloatAttribute("Y", value.Y) && SetFloatAttribute("Z", value.Z) && EndTag();
	}
	bool WriteQuaternion(const char* tag, W3dQuaternionStruct value)
	{
		return StartTag(tag, 0) && SetFloatAttribute("X", value.Q[0]) && SetFloatAttribute("Y", value.Q[1]) && SetFloatAttribute("Z", value.Q[2]) && SetFloatAttribute("W", value.Q[3]) && EndTag();
	}
	bool WriteString(const char* tag, const char *value)
	{
		return StartTag(tag, 1) && EndTag() && WriteFormatted("%s", value) && WriteClosingTag();
	}
	bool WriteInt(const char* tag, int value)
	{
		return StartTag(tag, 1) && EndTag() && WriteFormatted("%d", value) && WriteClosingTag();
	}
	bool WriteUnsignedInt(const char* tag, unsigned int value)
	{
		return StartTag(tag, 1) && EndTag() && WriteFormatted("%u", value) && WriteClosingTag();
	}
	bool WriteFloat(const char* tag, float value)
	{
		return StartTag(tag, 1) && EndTag() && WriteFormatted("%f", value) && WriteClosingTag();
	}
	bool WriteBool(const char* tag, int value)
	{
		if (!StartTag(tag, 1) || !EndTag())
		{
			return false;
		}
		char* str = "true";
		if (!value)
		{
			str = "false";
		}
		return WriteFormatted("%s", str) && WriteClosingTag();
	}
};
