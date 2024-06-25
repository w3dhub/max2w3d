#include "general.h"
#include "engine_string.h"

#if (_MSVC_LANG >= 201703L)
#include <charconv>
#endif

#pragma warning(disable: 4073) //warning C4073: initializers put in library initialization area - That's EXACTLY why I put that pragma in...
#pragma init_seg(lib) // Move this files static initializers up a level
#pragma warning(default: 4073)
#if (SHARED_EXPORTS || EXTERNAL)
char __declspec(thread) StringClass::TempStrings[MAX_TEMP_STRING][MAX_TEMP_BYTES] = {};
unsigned int __declspec(thread) StringClass::FreeTempStrings = (1 << MAX_TEMP_STRING) - 1;
char __declspec(thread) WideStringClass::TempStrings[MAX_TEMP_STRING][MAX_TEMP_BYTES] = {};
unsigned int __declspec(thread) WideStringClass::FreeTempStrings = (1 << MAX_TEMP_STRING) - 1;
char StringClass::m_NullChar = '\0';
wchar_t WideStringClass::m_NullChar = L'\0'; 
char *StringClass::m_EmptyString = &m_NullChar;
wchar_t *WideStringClass::m_EmptyString = &m_NullChar;
#endif


const StringClass& StringClass::operator+= (int16 i)
{
	constexpr size_t buf_size = Get_Integer_Print_String_Buf_Size(sizeof(i));
	char buf[buf_size];
#if (_MSVC_LANG >= 201703L)
	std::to_chars_result res = std::to_chars(buf, buf + buf_size, i);
	*res.ptr = '\0';
#else
	sprintf(buf, "%hd", i);
#endif
	(*this) += buf;
	return (*this);
}
const StringClass& StringClass::operator+= (uint16 i)
{
	constexpr size_t buf_size = Get_Integer_Print_String_Buf_Size(sizeof(i));
	char buf[buf_size];
#if (_MSVC_LANG >= 201703L)
	std::to_chars_result res = std::to_chars(buf, buf + buf_size, i);
	*res.ptr = '\0';
#else
	sprintf(buf, "%hu", i);
#endif
	(*this) += buf;
	return (*this);
}
const StringClass& StringClass::operator+= (int32 i)
{
	constexpr size_t buf_size = Get_Integer_Print_String_Buf_Size(sizeof(i));
	char buf[buf_size];
#if (_MSVC_LANG >= 201703L)
	std::to_chars_result res = std::to_chars(buf, buf + buf_size, i);
	*res.ptr = '\0';
#else
	sprintf(buf, "%d", i);
#endif
	(*this) += buf;
	return (*this);
}
const StringClass& StringClass::operator+= (uint32 i)
{
	constexpr size_t buf_size = Get_Integer_Print_String_Buf_Size(sizeof(i));
	char buf[buf_size];
#if (_MSVC_LANG >= 201703L)
	std::to_chars_result res = std::to_chars(buf, buf + buf_size, i);
	*res.ptr = '\0';
#else
	sprintf(buf, "%u", i);
#endif
	(*this) += buf;
	return (*this);
}
const StringClass& StringClass::operator+= (int64 i)
{
	constexpr size_t buf_size = Get_Integer_Print_String_Buf_Size(sizeof(i));
	char buf[buf_size];
#if (_MSVC_LANG >= 201703L)
	std::to_chars_result res = std::to_chars(buf, buf + buf_size, i);
	*res.ptr = '\0';
#else
	sprintf(buf, "%lld", i);
#endif
	(*this) += buf;
	return (*this);
}
const StringClass& StringClass::operator+= (uint64 i)
{
	constexpr size_t buf_size = Get_Integer_Print_String_Buf_Size(sizeof(i));
	char buf[buf_size];
#if (_MSVC_LANG >= 201703L)
	std::to_chars_result res = std::to_chars(buf, buf + buf_size, i);
	*res.ptr = '\0';
#else
	sprintf(buf, "%llu", i);
#endif
	(*this) += buf;
	return (*this);
}
const StringClass& StringClass::operator+= (float f)
{
	constexpr size_t buf_size = 78;
	char buf[buf_size];
#if (_MSVC_LANG >= 201703L) && false // to_chars doesn't match printf behavior
	std::to_chars_result res = std::to_chars(buf, buf + buf_size, f, std::chars_format::fixed);
	*res.ptr = '\0';
#else
	snprintf(buf, buf_size, "%f", f);
#endif
	(*this) += buf;
	return (*this);
}
const StringClass& StringClass::operator+= (double f)
{
	constexpr size_t buf_size = 377;
	char buf[buf_size];
#if (_MSVC_LANG >= 201703L) && false // to_chars doesn't match printf behavior
	std::to_chars_result res = std::to_chars(buf, buf + buf_size, f, std::chars_format::fixed);
	*res.ptr = '\0';
#else
	snprintf(buf, buf_size, "%f", f);
#endif
	(*this) += buf;
	return (*this);
}

#if (SHARED_EXPORTS || EXTERNAL)
int __cdecl StringClass::Format(_Printf_format_string_ const char* format,...)
{
	if (format == nullptr)
		return 0;

	va_list arg_list;
	va_start(arg_list,format);
	int len = vsnprintf(nullptr, 0, format, arg_list);
	Uninitialised_Grow(len + 1);
	Store_Length(len);
	vsnprintf(Peek_Buffer(), len+1, format, arg_list);
	va_end(arg_list);
	return len;
}

int __cdecl StringClass::Format_Args(_Printf_format_string_ const char* format,const va_list& arg_list)
{
	int len = vsnprintf(nullptr, 0, format, arg_list);
	Uninitialised_Grow(len + 1);
	Store_Length(len);
	vsnprintf(Peek_Buffer(), len+1, format, arg_list);
	return len;
}

void StringClass::Get_String(int length, bool is_temp)
{	
	if (!is_temp && !length) Free_String();
	else if (is_temp && length <= MAX_TEMP_LEN && FreeTempStrings)
	{
		uint32 index = 0;
		BitScanForward((DWORD*)&index, FreeTempStrings); // Find the first free temp string
		FreeTempStrings &= ~(1 << index); // Remove it from the free pool
		char* buffer = TempStrings[index] + sizeof(_HEADER);
		Set_Buffer_And_Allocated_Length(buffer, MAX_TEMP_LEN);
	}
	else if (length > 0) Set_Buffer_And_Allocated_Length(Allocate_Buffer(length), length);
	else Free_String();
}

void StringClass::Resize(int new_len)
{
	if (new_len > Get_Allocated_Length())
	{
		char *x = Allocate_Buffer(new_len);
		strcpy(x,m_Buffer);
		Free_String();
		Set_Buffer_And_Allocated_Length(x,new_len);
	}
}

void StringClass::Uninitialised_Grow(int new_len)
{
	if (new_len > Get_Allocated_Length())
	{
		char *x = Allocate_Buffer(new_len);
		Free_String();
		Set_Buffer_And_Allocated_Length(x,new_len);
	}
	Store_Length(0);
}

void StringClass::Free_String()
{
	if (m_Buffer == m_EmptyString) return;

	int index = Get_Temp_String_Index();

	if (index >= 0)
	{
		// It was a temp string, cast Undead.
		m_Buffer[0] = m_NullChar;
		FreeTempStrings |= 1 << index;
	}
	else
	{
		ptrdiff_t buffer_base = intptr_t(m_Buffer) - sizeof(_HEADER);
		char* buffer = (char*)buffer_base;
		delete[] buffer;
	}

	m_Buffer = m_EmptyString;
}

#include <regex>

SHARED_API void StringClass::Regex_Replace(const char* regexString, const char* regexReplacement)
{
	// clumsy basic wrapper for regex_replace for StringClass
	std::regex regex_finder(regexString);
	std::string base_string = m_Buffer;

	*this = std::regex_replace(base_string, regex_finder, regexReplacement).c_str();
}

int StringClass::Get_Temp_String_Index()
{
	ptrdiff_t buffer_base = intptr_t(m_Buffer) - sizeof(_HEADER);
	ptrdiff_t diff = buffer_base - intptr_t(TempStrings[0]);

	if (diff >= 0 && diff < MAX_TEMP_BYTES * MAX_TEMP_STRING)
	{
		ptrdiff_t index = diff / MAX_TEMP_BYTES;
		return int(index);
	}
	return -1;
}

void StringClass::Release_Resources()
{
}

int StringClass::Replace(const char* search, const char* replace, bool bCaseSensitive, int maxCount)
{
	if (m_Buffer == m_EmptyString) return 0;

  int nReplacements = 0;

  char* newstring = nullptr;             // The modified string, NULL until we make a change
  size_t newstringlen = Get_Length()+1;  // The length of the modified string
  const char* searchPtr = m_Buffer;   // The starting point for the next search, always lastreplacement+1

  // Figure out lengths in advance to avoid doing it repeatedly
  size_t searchlen = strlen(search);
  size_t replacelen = strlen(replace);

  while (nullptr != searchPtr && (-1 == maxCount || nReplacements < maxCount) )
  {
    // Find the next instance of the search string
    const char* foundPtr = ( bCaseSensitive ) ? stristr(searchPtr,search) : strstr(searchPtr,search);
    searchPtr = nullptr;

    if (nullptr != foundPtr )
    {
      // Figure out the 0-based index of the first character to be replaced
      size_t replaceindex = foundPtr - ((nullptr==newstring)?m_Buffer:newstring);

      // Allocate a new string to fit the replacement data if necessary
      if ( searchlen != replacelen || nullptr == newstring )
      {
        // Cache old data so we can clean up memory
        size_t oldnewstringlen = newstringlen;
        char* oldnewstring = newstring;

        newstringlen = oldnewstringlen + (replacelen-searchlen);
        newstring = new char[newstringlen];

        // Copy characters preceeding the string to be replaced
        memcpy(newstring, (nullptr==oldnewstring)?m_Buffer:oldnewstring, replaceindex);

        // Copy characters following the string to be replaced
        size_t postsearchindex = replaceindex+searchlen;
		size_t postreplaceindex = replaceindex+replacelen;
        memcpy(newstring+postreplaceindex, ((nullptr==oldnewstring)?m_Buffer:oldnewstring)+postsearchindex, oldnewstringlen-postsearchindex);

        delete [] oldnewstring;
      }
      
      // Copy in the replacement string in the location of the located search string
      memcpy(newstring+replaceindex, replace, replacelen);

      // Update the search pointer
      searchPtr = newstring + replaceindex + replacelen;

      nReplacements++;
    }
  }

  // Update the string with the modified version, if any
  if (nullptr != newstring )
  {
    *this = newstring;
    delete [] newstring;
  }

  return nReplacements;
}

bool StringClass::Copy_Wide(const wchar_t *source)
{
	if (source != nullptr)
	{
		BOOL unmapped;
		int length = WideCharToMultiByte(CP_ACP, 0, source, -1, nullptr, 0, nullptr, &unmapped);
		if (length > 0)
		{
			WideCharToMultiByte(CP_ACP, 0, source, -1, Get_Buffer(length), length, nullptr, nullptr);
			Store_Length(length - 1);
		}
		return (!unmapped);
	}
	return false;
}


int __cdecl WideStringClass::Format(_Printf_format_string_ const wchar_t* format,...)
{
	if (format == nullptr)
	{
		return 0;
	}
	va_list arg_list;
	va_start(arg_list,format);
	int len = _vsnwprintf(nullptr, 0, format, arg_list);
	Uninitialised_Grow(len + 1);
	Store_Length(len);
	_vsnwprintf(Peek_Buffer(), len+1, format, arg_list);
	va_end(arg_list);
	return len;
}

int __cdecl WideStringClass::Format_Args(_Printf_format_string_ const wchar_t* format,const va_list& arg_list)
{
	if (format == nullptr)
		return 0;

	int len = _vsnwprintf(nullptr,0,format,arg_list);
	Uninitialised_Grow(len + 1);
	Store_Length(len);
	_vsnwprintf(Peek_Buffer(), len+1,format,arg_list);
	return len;
}

void WideStringClass::Get_String(int length,bool is_temp)
{
	if (!is_temp && !length) Free_String();
	else if (is_temp && length <= MAX_TEMP_LEN && FreeTempStrings)
	{
		uint32 index = 0;
		BitScanForward((DWORD*)&index, FreeTempStrings); // Find the first free temp string
		FreeTempStrings &= ~(1 << index); // Remove it from the free pool
		wchar_t* buffer = (wchar_t*)(TempStrings[index] + sizeof(_HEADER));
		Set_Buffer_And_Allocated_Length(buffer, MAX_TEMP_LEN);
	}
	else if (length > 0) Set_Buffer_And_Allocated_Length(Allocate_Buffer(length), length);
	else Free_String();
}

void WideStringClass::Resize(int new_len)
{
	if (new_len > Get_Allocated_Length())
	{
		wchar_t *x = Allocate_Buffer(new_len);
		wcscpy(x,m_Buffer);
		Free_String();
		Set_Buffer_And_Allocated_Length(x,new_len);
	}
}

void WideStringClass::Uninitialised_Grow(int new_len)
{
	if (new_len > Get_Allocated_Length())
	{
		wchar_t *x = Allocate_Buffer(new_len);
		Free_String();
		Set_Buffer_And_Allocated_Length(x,new_len);
	}
	Store_Length(0);
}

void WideStringClass::Free_String()
{
	if (m_Buffer == m_EmptyString) return;

	int index = Get_Temp_String_Index();

	if (index >= 0)
	{
		// It was a temp string, cast Undead.
		m_Buffer[0] = m_NullChar;
		FreeTempStrings |= 1 << index;
	}
	else
	{
		ptrdiff_t buffer_base = intptr_t(m_Buffer) - sizeof(_HEADER);
		char* buffer = (char*)buffer_base;
		delete[] buffer;
	}

	m_Buffer = m_EmptyString;
}

int WideStringClass::Get_Temp_String_Index()
{
	ptrdiff_t buffer_base = intptr_t(m_Buffer) - sizeof(_HEADER);
	ptrdiff_t diff = buffer_base - intptr_t(TempStrings[0]);

	if (diff >= 0 && diff < MAX_TEMP_BYTES * MAX_TEMP_STRING)
	{
		ptrdiff_t index = diff / MAX_TEMP_BYTES;
		return int(index);
	}
	return -1;
}

void WideStringClass::Release_Resources()
{
}

int WideStringClass::Replace(const wchar_t* search, const wchar_t* replace, bool bCaseSensitive, int maxCount)
{
	if (m_Buffer == m_EmptyString) return 0;

	int nReplacements = 0;

	wchar_t* newstring = nullptr;             // The modified string, NULL until we make a change
	size_t newstringlen = Get_Length() + 1;  // The length of the modified string
	const wchar_t* searchPtr = m_Buffer;   // The starting point for the next search, always lastreplacement+1

	// Figure out lengths in advance to avoid doing it repeatedly
	size_t searchlen = wcslen(search);
	size_t replacelen = wcslen(replace);

	while (nullptr != searchPtr && (-1 == maxCount || nReplacements < maxCount))
	{
		// Find the next instance of the search string
		const wchar_t* foundPtr = (bCaseSensitive) ? wcsistr(searchPtr, search) : wcsstr(searchPtr, search);
		searchPtr = nullptr;

		if (nullptr != foundPtr)
		{
			// Figure out the 0-based index of the first character to be replaced
			size_t replaceindex = foundPtr - ((nullptr == newstring) ? m_Buffer : newstring);

			// Allocate a new string to fit the replacement data if necessary
			if (searchlen != replacelen || nullptr == newstring)
			{
				// Cache old data so we can clean up memory
				size_t oldnewstringlen = newstringlen;
				wchar_t* oldnewstring = newstring;

				newstringlen = oldnewstringlen + (replacelen - searchlen);
				newstring = new wchar_t[newstringlen];

				// Copy characters preceeding the string to be replaced
				memcpy(newstring, (nullptr == oldnewstring) ? m_Buffer : oldnewstring, replaceindex * sizeof(wchar_t));

				// Copy characters following the string to be replaced
				size_t postsearchindex = replaceindex + searchlen;
				size_t postreplaceindex = replaceindex + replacelen;
				memcpy(newstring + postreplaceindex, ((nullptr == oldnewstring) ? m_Buffer : oldnewstring) + postsearchindex, (oldnewstringlen - postsearchindex) * sizeof(wchar_t));

				delete[] oldnewstring;
			}

			// Copy in the replacement string in the location of the located search string
			memcpy(newstring + replaceindex, replace, replacelen * sizeof(wchar_t));

			// Update the search pointer
			searchPtr = newstring + replaceindex + replacelen;

			nReplacements++;
		}
	}

	// Update the string with the modified version, if any
	if (nullptr != newstring)
	{
		*this = newstring;
		delete[] newstring;
	}

	return nReplacements;
}

bool WideStringClass::Convert_From(const char *text)
{
	if (text != nullptr)
	{
		int length = MultiByteToWideChar(CP_ACP, 0, text, -1, nullptr, 0);
		if (length > 0)
		{
			Uninitialised_Grow(length);
			Store_Length(length - 1);
			MultiByteToWideChar(CP_ACP, 0, text, -1, m_Buffer, length);
			return true;
		}
	}
	return false;
}

bool WideStringClass::Is_ANSI()
{
	if (m_Buffer)
	{
		for (int i = 0;m_Buffer[i] != 0;i++)
		{
			unsigned short value = m_Buffer[i];
			if (value > 255)
			{
				return false;
			}
		}
	}
	return true;
}
WideStringClass WideStringClass::Substring(int start, int length) const
{
	TT_ASSERT(start + length <= Get_Length());

	WideStringClass result;
	result.Uninitialised_Grow(length+1);
	result.Store_Length(length);
	memcpy(result.m_Buffer, m_Buffer + start, length * sizeof(wchar_t));
	result.m_Buffer[length] = L'\0';

	return result;
}

void WideStringClass::RemoveSubstring(int start, int length)
{
	if (length > 0)
	{
		int oldLength = Get_Length();
		int newLength = oldLength - length;
		TT_ASSERT(start + length <= oldLength);

		memmove(m_Buffer + start, m_Buffer + start + length, (newLength - start) * sizeof(wchar_t));
		m_Buffer[newLength] = L'\0';
		Store_Length(newLength);
	}
}

void WideStringClass::ReplaceSubstring(int start, int length, const WideStringClass& substring)
{
	int substringLength = substring.Get_Length();
	int oldLength = Get_Length();
	int newLength = oldLength - length + substringLength;
	TT_ASSERT(start + length <= oldLength);

	if (substringLength > length)
		Resize(newLength + 1);
	
	memmove(m_Buffer + start + substringLength, m_Buffer + start + length, (oldLength - start - length) * sizeof(wchar_t));
	memcpy(m_Buffer + start, substring.m_Buffer, substringLength * sizeof(wchar_t));
	m_Buffer[newLength] = L'\0';
	Store_Length(newLength);
}
#endif
SCRIPTS_API char *newstr(const char *str)
{
	if (!str)
	{
		return nullptr;
	}
	size_t len = strlen(str)+1;
	char *s = new char[len];
	memcpy(s,str,len);
	return s;	
};
SCRIPTS_API wchar_t *newwcs(const wchar_t *str)
{
	if (!str)
		return nullptr;
	size_t len = wcslen(str)+1;
	wchar_t *s = new wchar_t[len];
	memcpy(s,str,len*2);
	return s;
};
SCRIPTS_API char *strtrim(char *v)
{
	if (v)
	{
		char *r = v;
		while (*r > 0 && *r < 0x21)
			r++;
		strcpy(v,r);
		r = v + strlen(v);
		while (r > v && r[-1] > 0 && r[-1] < 0x21)
			r--;
		*r = 0;
	}
	return v;
}

SCRIPTS_API char *strrtrim(char *s) 
{
	char *t, *tt;

	TT_ASSERT(s != nullptr);

	for (tt = t = s; *t != '\0'; ++t)
		if (!isspace(*(unsigned char *)t))
			tt = t+1;
	*tt = '\0';

	return s;
}

SCRIPTS_API const char *stristr(const char *str, const char *substr){
	size_t substr_len = strlen(substr);
	while (*str){
		if (_strnicmp(str, substr, substr_len) == 0)
			return str;		
		str++;
	}
	return nullptr;
}

SCRIPTS_API const wchar_t *wcsistr(const wchar_t *str, const wchar_t *substr){
	if (!*str)
		return nullptr;
	size_t substr_len = wcslen(substr);
	while (*str){
		if (_wcsnicmp(str, substr, substr_len) == 0)
			return str;
		str++;
	}
	return nullptr;
}
