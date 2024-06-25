#ifndef SCRIPTS_INCLUDE__ENGINE_STRING_H
#define SCRIPTS_INCLUDE__ENGINE_STRING_H

#include "HashTemplateKeyClass.h"
#include "engine_string_view.h"

class StringClass {
public:
	explicit StringClass(bool hint_temporary);
	StringClass(int initial_len = 0, bool hint_temporary = false);
	StringClass(const StringClass& string, bool hint_temporary = false);
	StringClass(const char *string, bool hint_temporary = false);
	StringClass(char ch, bool hint_temporary = false);
	StringClass(const wchar_t *string, bool hint_temporary = false);
	StringClass(const StringView& sv, bool hint_temporary = false);
	StringClass(StringClass&& string) noexcept;
	// prevent accidentally calling the wrong constructor since integers convert to bool implicitly
	template<typename T, typename = std::enable_if_t<!std::is_same_v<T, bool>>>
	StringClass(const wchar_t* string, T t) = delete;
	template<typename T, typename = std::enable_if_t<!std::is_same_v<T, bool>>>
	StringClass(const char* string, T t) = delete;
	~StringClass();
	bool operator ==(const char* rvalue) const;
	bool operator!= (const char* rvalue) const;
	const StringClass& operator=(const char* string);
	const StringClass& operator=(const StringClass& string);
	const StringClass& operator=(const StringView& string);
	const StringClass& operator=(StringClass&& string) noexcept;
	const StringClass& operator=(char ch);
	const StringClass& operator=(const wchar_t *string);

	const StringClass& operator+=(const char* string);
	const StringClass& operator+=(const StringClass& string);
	const StringClass& operator+=(wchar_t ch) = delete; // use a regular character!
	const StringClass& operator+=(char ch);
	const StringClass& operator+=(int16 i);
	const StringClass& operator+=(uint16 i);
	const StringClass& operator+=(int32 i);
	const StringClass& operator+=(uint32 i);
	const StringClass& operator+=(int64 i);
	const StringClass& operator+=(uint64 i);
	const StringClass& operator+=(float f);
	const StringClass& operator+=(double d);

	friend StringClass operator+ (const StringClass &string1, const StringClass &string2);
	friend StringClass operator+ (const StringClass &string1, const char *string2);
	friend StringClass operator+ (const char *string1, const StringClass &string2);

	bool operator < (const char *string) const;
	bool operator <= (const char *string) const;
	bool operator > (const char *string) const;
	bool operator >= (const char *string) const;

	const char & operator[] (int index) const;
	char & operator[] (int index);
	const char & operator[] (size_t index) const;
	char & operator[] (size_t index);
	operator const char * (void) const; // NOTE(Mara): would love to make this explicit but it breaks too much code
	// people migrate code from char* to StringClass all the time and forget to call !Is_Empty() instead of checking for nullptr
	operator bool() const { return !Is_Empty(); }
	operator StringView() const noexcept { return StringView(m_Buffer, size_t(Get_Length())); }

	int	Compare (const char *string) const;
	int Compare_No_Case (const char *string) const;

	int	Get_Length (void) const;
	bool Is_Empty (void) const;
	void Erase (int start_index, int char_count);
	//+V576, class:StringClass, function:Format, format_arg:1, ellipsis_arg:2
	SHARED_API int __cdecl Format(_Printf_format_string_ const char* format,...);
	SHARED_API int __cdecl Format_Args(_Printf_format_string_ const char* format,const va_list& arg_list);
	char *Get_Buffer (int new_length);
	char *Peek_Buffer (void);
	const char * Peek_Buffer (void) const;
	SHARED_API bool Copy_Wide (const wchar_t *source);
	SHARED_API static void Release_Resources();
	SHARED_API void Free_String();

	SHARED_API void Regex_Replace(const char* regexString, const char* regexReplacement);

  /*!
  * Finds any occurances of the search string within this string and replaces them with a specified
  * replacement string.
  *
  * \param[in] search
  *   The substring to be replaced
  * \param[in] replace
  *   The replacement string to insert
  * \param[in] bCaseSensitive
  *   True to perform a case sensitive search, false if case doesn't matter
  * \param[in] maxCount
  *   The maximum number of replacements to perform, or -1 to replace all instances
  *
  * \return
  *   The number of replacements that were made
  */
	SHARED_API int Replace(const char* search, const char* replace, bool bCaseSensitive = true, int maxCount=-1);

	StringClass Left(int nCount)
	{
		if (nCount < 0)
		{
			nCount = 0;
		}
		if (nCount >= Get_Header()->length)
		{
			return *this;
		}
		StringClass dest;
		if (nCount != 0)
		{
			dest.Uninitialised_Grow(nCount+1);
			dest[nCount] = 0;
			memcpy(dest.m_Buffer, m_Buffer, nCount);
		}
		return dest;
	}

	StringClass Right(int nCount)
	{
		if (nCount < 0)
		{
			nCount = 0;
		}
		if (nCount >= Get_Header()->length)
		{
			return *this;
		}
		StringClass dest;
		if (nCount != 0)
		{
			dest.Uninitialised_Grow(nCount + 1);
			dest[nCount] = 0;
			memcpy(dest.m_Buffer, m_Buffer + (Get_Header()->length - nCount), nCount);
		}
		return dest;
	}

	void TruncateLeft(uint truncateLength)
	{
		uint length = Get_Length();
		if (length <= truncateLength)
			Free_String();

		else
		{
			int newLength = length - truncateLength;
			memmove(m_Buffer, m_Buffer + truncateLength, newLength + 1);
			Store_Length(newLength);
		}
	}

	void TruncateRight(uint truncateLength)
	{
		uint length = Get_Length();
		if (length <= truncateLength)
			Free_String();

		else
		{
			int newLength = length - truncateLength;
			m_Buffer[newLength] = '\0';
			Store_Length(newLength);
		}
	}

	void cropTo(int to)
	{
		const int length = Get_Length();
		if (to <= 0)
			Free_String();
		else if (to < length)
		{
			m_Buffer[to] = '\0';
			Store_Length(to);
		}
	}

	void cropFrom(int from)
	{
		const int length = Get_Length();
		if (from >= length)
			Free_String();
		else if (from > 0)
		{
			memmove(m_Buffer, m_Buffer + from, length - from + 1);
			Store_Length(length - from);
		}
	}

	void crop(int from, int to)
	{
		cropTo(to);
		cropFrom(from);
	}

	void TrimLeft()
	{
		char* iter = m_Buffer;
		for (; *iter != '\0' && *iter <= ' '; iter++)
		{
			
		}

		TruncateLeft((int)(iter - m_Buffer));
	}

	void TrimRight()
	{
		char* iter = m_Buffer + Get_Length() - 1;
		for (; *iter != '\0' && *iter <= ' '; iter--)
		{
		}

		TruncateRight((int)(m_Buffer + Get_Length() - 1 - iter));
	}

	void Trim()
	{
		TrimLeft();
		TrimRight();
	}

	bool StartsWithI(const char* string) const
	{
		return _strnicmp(m_Buffer, string, strlen(string)) == 0;
	}

	uint GetHash() const
	{
		return StringHashFunc32(*this);
	}

    int IndexOf(char c) const
    {
        int length = Get_Length();
        for (int i = 0; i < length; ++i)
        {
            if (m_Buffer[i] == c) return i;
        }
        return -1;
    }

    int LastIndexOf(char c) const
    {
        for (int i = Get_Length() - 1; i >= 0; --i)
        {
            if (m_Buffer[i] == c) return i;
        }
        return -1;
    }

	void ToUpper()
	{
		_strupr(m_Buffer);
	}

	void ToLower()
	{
		_strlwr(m_Buffer);
	}

	StringClass AsUpper() const
	{
		StringClass result = *this;
		result.ToUpper();
		return result;
	}

	StringClass AsLower() const
	{
		StringClass result = *this;
		result.ToLower();
		return result;
	}

	//+V576, class:StringClass, function:getFormattedString, format_arg:1, ellipsis_arg:2
	static StringClass getFormattedString(_Printf_format_string_ const char* format, ...)
	{
		va_list arguments;
		StringClass result;

		va_start(arguments, format);
		result.Format_Args(format, arguments);
		va_end(arguments);

		return result;
	}

private:
	typedef struct _HEADER
	{
		int	allocated_length;
		int	length;
	} HEADER;
	enum
	{
		MAX_TEMP_STRING	= 8,
		MAX_TEMP_LEN		= 256-sizeof(_HEADER),
		MAX_TEMP_BYTES		= (MAX_TEMP_LEN * sizeof (char)) + sizeof (HEADER),
	};
	SHARED_API void Get_String(int length,bool is_temp);
	char* Allocate_Buffer(int len);
	SHARED_API void Resize(int new_len);
	SHARED_API void Uninitialised_Grow(int new_len);
	SHARED_API int Get_Temp_String_Index();
	bool Is_Temp_String();
	void Store_Length(int length);
	void Store_Allocated_Length(int length);
	HEADER *Get_Header() const;
	int Get_Allocated_Length() const;
	void Set_Buffer_And_Allocated_Length(char *buffer, int length);
	char* m_Buffer;
#if (SHARED_EXPORTS) || (EXTERNAL)
	static char __declspec(thread) TempStrings[MAX_TEMP_STRING][MAX_TEMP_BYTES];
	static unsigned int __declspec(thread) FreeTempStrings;
#endif
	SHARED_API static char * m_EmptyString;
	SHARED_API static char m_NullChar;
};

inline const StringClass &StringClass::operator= (const StringClass &string)
{	
	if (!(string.Is_Empty() && Is_Empty()))// don't bother allocating if we are assigning an empty/uninitialized string to an empty/uninitialized string
	{
		int len = string.Get_Length();
		Uninitialised_Grow(len + 1);
		Store_Length(len);
		memmove(m_Buffer, string.m_Buffer, (len + 1) * sizeof(char));
	}
	return (*this);
}

inline const StringClass &StringClass::operator= (const StringView &string)
{	
	if (!(string.empty() && Is_Empty()))// don't bother allocating if we are assigning an empty/uninitialized string to an empty/uninitialized string
	{
		int len = int(string.length());
		Uninitialised_Grow(len + 1);
		Store_Length(len);
		memmove(m_Buffer, string.data(), len * sizeof(char));
		m_Buffer[len] = '\0'; // string views aren't guaranteed to be null terminated
	}
	return (*this);
}

inline const StringClass& StringClass::operator= (StringClass&& string) noexcept
{
	if (this != &string)
	{
		if (string.Is_Temp_String() && !Is_Temp_String())
		{
			// if the other string is a temp string, we sadly need to copy it, because otherwise we can run into weird issues
			// where e.g. a temp string is being returned from a function and moved into a member variable, permanently blocking a global temp string
			*this = string;
			string.Free_String();
		}
		else
		{
			if (m_Buffer != m_EmptyString)
				Free_String();

			m_Buffer = string.m_Buffer; // steal buffer
			string.m_Buffer = m_EmptyString;
		}
	}
	return (*this);
}

inline const StringClass &StringClass::operator= (const char *string)
{
	if (string && !(string[0] == '\0' && Is_Empty())) // don't bother allocating if we are assigning an empty string to an empty/uninitialized string
	{
		int len = (int)strlen(string);
		Uninitialised_Grow(len + 1);
		Store_Length(len);
		memmove(m_Buffer, string, (len + 1) * sizeof(char)); // memmove because string could be pointing into our own buffer
	}
	return (*this);
}

inline const StringClass &StringClass::operator= (const wchar_t *string)
{
	if (string != nullptr)
	{
		Copy_Wide (string);
	}
	return (*this);
}

inline const StringClass &StringClass::operator= (char ch)
{
	Uninitialised_Grow (2);
	m_Buffer[0] = ch;
	m_Buffer[1] = m_NullChar;
	Store_Length (1);
	return (*this);
}

inline StringClass::StringClass (bool hint_temporary) : m_Buffer (m_EmptyString)
{
	if (hint_temporary) {
		Get_String(MAX_TEMP_LEN, hint_temporary);
		m_Buffer[0] = m_NullChar;
	}
	return ;
}

inline StringClass::StringClass (int initial_len, bool hint_temporary) : m_Buffer (m_EmptyString)
{
	if (hint_temporary || initial_len > 0) {
		Get_String(initial_len, hint_temporary);
		m_Buffer[0] = m_NullChar;
	}
}

inline StringClass::StringClass (char ch, bool hint_temporary) : m_Buffer (m_EmptyString)
{
	Get_String (2, hint_temporary);
	(*this) = ch;
}

inline StringClass::StringClass (const StringClass &string, bool hint_temporary) : m_Buffer (m_EmptyString)
{
	if (hint_temporary || (string.Get_Length()>0))
	{
		Get_String (string.Get_Length()+1, hint_temporary);
	}
	(*this) = string;
}

inline StringClass::StringClass (const StringView& string, bool hint_temporary) : m_Buffer (m_EmptyString)
{
	if (hint_temporary || (string.length()>0))
	{
		Get_String (int(string.length())+1, hint_temporary);
	}
	(*this) = string;
}

inline StringClass::StringClass(StringClass&& string) noexcept : m_Buffer(m_EmptyString)
{
	*this = std::move(string);
}

inline StringClass::StringClass (const char *string, bool hint_temporary) : m_Buffer (m_EmptyString)
{
	int len=string ? (int)strlen(string) : 0;
	if (hint_temporary || len>0)
	{
		Get_String (len+1, hint_temporary);
	}
	(*this) = string;
}

inline StringClass::StringClass (const wchar_t *string, bool hint_temporary) : m_Buffer (m_EmptyString)
{
	int len = string ? (int)wcslen (string) : 0;
	if (hint_temporary || len > 0)
	{
		Get_String (len + 1, hint_temporary);
	}
	(*this) = string;
}

inline StringClass::~StringClass (void)
{
	if (m_Buffer != m_EmptyString)
		Free_String ();
}

inline bool StringClass::Is_Temp_String()
{
	return Get_Temp_String_Index() >= 0;
}

inline bool StringClass::Is_Empty (void) const
{
	return (m_Buffer[0] == m_NullChar);
}

inline int StringClass::Compare (const char *string) const
{
	return strcmp (m_Buffer, string);
}

inline int StringClass::Compare_No_Case (const char *string) const
{
	return _stricmp (m_Buffer, string);
}

inline const char &StringClass::operator[] (int index) const
{
	return m_Buffer[index];
}

inline char &StringClass::operator[] (int index)
{
	return m_Buffer[index];
}

inline const char &StringClass::operator[] (size_t index) const
{
	return m_Buffer[index];
}

inline char &StringClass::operator[] (size_t index)
{
	return m_Buffer[index];
}

inline StringClass::operator const char * (void) const
{
	return m_Buffer;
}

inline bool StringClass::operator== (const char *rvalue) const
{
	return (Compare (rvalue) == 0);
}

inline bool StringClass::operator!= (const char *rvalue) const
{
	return (Compare (rvalue) != 0);
}

inline bool StringClass::operator < (const char *string) const
{
	return (strcmp (m_Buffer, string) < 0);
}

inline bool StringClass::operator <= (const char *string) const
{
	return (strcmp (m_Buffer, string) <= 0);
}

inline bool StringClass::operator > (const char *string) const
{
	return (strcmp (m_Buffer, string) > 0);
}

inline bool StringClass::operator >= (const char *string) const
{
	return (strcmp (m_Buffer, string) >= 0);
}

inline void StringClass::Erase (int start_index, int char_count)
{
	if (start_index < 0 || char_count <= 0)
		return;

	int len = Get_Length ();
	if (start_index < len)
	{
		if (start_index + char_count > len)
		{
			char_count = len - start_index;
		}
		memmove (	&m_Buffer[start_index], &m_Buffer[start_index + char_count], (len - (start_index + char_count) + 1) * sizeof (char));
		Store_Length( len - char_count );
	}
}

inline const StringClass& StringClass::operator+= (const char* string)
{
	if (string)
	{
		int src_len = (int)strlen(string);
		if (src_len > 0)
		{
			int cur_len = Get_Length();
			int new_len = cur_len + src_len;
			Resize(new_len + 1);
			Store_Length(new_len);
			memcpy(&m_Buffer[cur_len], string, (src_len + 1) * sizeof(char));
		}
	}
	return (*this);
}

inline const StringClass &StringClass::operator+= (char ch)
{
	int cur_len = Get_Length ();
	Resize (cur_len + 2);
	m_Buffer[cur_len] = ch;
	m_Buffer[cur_len + 1] = m_NullChar;
	if (ch != m_NullChar)
	{
		Store_Length (cur_len + 1);
	}
	return (*this);
}

constexpr size_t Get_Integer_Print_String_Buf_Size(size_t size)
{
	switch (size) {
	case 1: return 5; // -128
	case 2: return 7; // -32'768
	case 4: return 12; // -2'147'483'648
	case 8: return 21; // -9'223'372'036'854'775'808
	}
	return 0;
}

inline char *StringClass::Get_Buffer (int new_length)
{
	Uninitialised_Grow (new_length);
	return m_Buffer;
}

inline char *StringClass::Peek_Buffer (void)
{
	return m_Buffer;
}

inline const char *StringClass::Peek_Buffer (void) const
{
	return m_Buffer;
}

inline const StringClass &StringClass::operator+= (const StringClass &string)
{
	int src_len = string.Get_Length();
	if (src_len > 0)
	{
		int cur_len = Get_Length ();
		int new_len = cur_len + src_len;
		Resize (new_len + 1);
		Store_Length (new_len);
		memcpy (&m_Buffer[cur_len], (const char *)string, (src_len + 1) * sizeof (char));				
	}
	return (*this);
}

inline StringClass operator+ (const StringClass &string1, const StringClass &string2)
{
	StringClass new_string(string1, true);
	new_string += string2;
	return new_string;
}

inline StringClass operator+ (const char *string1, const StringClass &string2)
{
	StringClass new_string(string1, true);
	new_string += string2;
	return new_string;
}

inline StringClass operator+ (const StringClass &string1, const char *string2)
{
	StringClass new_string(string1, true);
	new_string += string2;
	return new_string;
}

inline int StringClass::Get_Allocated_Length (void) const
{
	int allocated_length = 0;
	if (m_Buffer != m_EmptyString)
	{
		HEADER *header = Get_Header ();
		allocated_length = header->allocated_length;
	}
	return allocated_length;
}

inline int StringClass::Get_Length (void) const
{
	int length = 0;
	if (m_Buffer != m_EmptyString)
	{
		HEADER *header = Get_Header ();
		length = header->length;
		if (length == 0)
		{
			length = (int)strlen (m_Buffer);
			((StringClass *)this)->Store_Length (length);
		}
	}
	return length;
}

inline void StringClass::Set_Buffer_And_Allocated_Length (char *buffer, int length)
{
	if (m_Buffer != m_EmptyString)
		Free_String ();

	m_Buffer = buffer;
	if (m_Buffer != m_EmptyString)
	{
		Store_Allocated_Length (length);
		Store_Length (0);		
	}
}

inline char *StringClass::Allocate_Buffer (int length)
{
	char *buffer = new char[(sizeof (char) * length) + sizeof (StringClass::_HEADER)];
	HEADER *header = reinterpret_cast<HEADER *>(buffer);
	header->length = 0;
	header->allocated_length = length;
	return reinterpret_cast<char *>(buffer + sizeof (StringClass::_HEADER));
}

inline StringClass::HEADER *StringClass::Get_Header (void) const
{
	return reinterpret_cast<HEADER *>(((char *)m_Buffer) - sizeof (StringClass::_HEADER));
}

inline void StringClass::Store_Allocated_Length (int allocated_length)
{
	if (m_Buffer != m_EmptyString)
	{
		HEADER *header = Get_Header ();
		header->allocated_length = allocated_length;
	}
}

inline void StringClass::Store_Length (int length)
{
	if (m_Buffer != m_EmptyString)
	{
		HEADER *header = Get_Header();
		header->length = length;
	}
}

class WideStringClass {
public:
	WideStringClass (int initial_len = 0, bool hint_temporary = false);
	WideStringClass (const WideStringClass &string,	bool hint_temporary = false);
	WideStringClass (const wchar_t *string, bool hint_temporary = false);
	WideStringClass (wchar_t ch, bool hint_temporary = false);
	WideStringClass (const char *string, bool hint_temporary = false);
	WideStringClass (const WideStringView& string, bool hint_temporary = false);
	WideStringClass (WideStringClass &&string) noexcept;
	// prevent accidentally calling the wrong constructor since there are lots of things that convert to bool implicitly
	template<typename T, typename = std::enable_if_t<!std::is_same_v<T, bool>>>
	WideStringClass (const wchar_t* string, T t) = delete; 
	template<typename T, typename = std::enable_if_t<!std::is_same_v<T, bool>>>
	WideStringClass (const char* string, T t) = delete;
	~WideStringClass (void);
	bool operator== (const wchar_t *rvalue) const;
	bool operator!= (const wchar_t *rvalue) const;
	const WideStringClass &operator= (const WideStringClass &string);
	const WideStringClass &operator= (const WideStringView &string);
	const WideStringClass &operator= (WideStringClass &&string) noexcept;
	const WideStringClass &operator= (const wchar_t *string);
	const WideStringClass &operator= (wchar_t ch);
	const WideStringClass &operator= (const char *string);

	const WideStringClass &operator+= (const WideStringClass &string);
	const WideStringClass &operator+= (const wchar_t *string);
	const WideStringClass &operator+= (wchar_t ch);
	const WideStringClass &operator+= (char ch) = delete; // use the L prefix to add a wide character!
	//const WideStringClass& operator+=(int16 i);
	//const WideStringClass& operator+=(uint16 i);
	const WideStringClass& operator+=(int32 i);
	const WideStringClass& operator+=(uint32 i);
	const WideStringClass& operator+=(int64 i);
	const WideStringClass& operator+=(uint64 i);
	const WideStringClass& operator+=(float f);
	const WideStringClass& operator+=(double d);

	friend WideStringClass operator+ (const WideStringClass &string1, const WideStringClass &string2);
	friend WideStringClass operator+ (const wchar_t *string1, const WideStringClass &string2);
	friend WideStringClass operator+ (const WideStringClass &string1, const wchar_t *string2);

	bool operator < (const wchar_t *string) const;
	bool operator <= (const wchar_t *string) const;
	bool operator > (const wchar_t *string) const;
	bool operator >= (const wchar_t *string) const;

	wchar_t operator[] (int index) const;
	wchar_t& operator[] (int index);
	wchar_t operator[] (size_t index) const;
	wchar_t& operator[] (size_t index);
	operator const wchar_t * (void) const; // NOTE(Mara): would love to make this explicit but it breaks too much code
	// people migrate code from wchar_t* to WideStringClass all the time and forget to call !Is_Empty() instead of checking for nullptr
	operator bool() const { return !Is_Empty(); }
	operator WideStringView() const noexcept { return WideStringView(m_Buffer, size_t(Get_Length())); }

	int Compare (const wchar_t *string) const;
	int Compare_No_Case (const wchar_t *string) const;
	int Get_Length (void) const;
	bool Is_Empty (void) const;
	void Erase (int start_index, int char_count);
	//+V576, class:WideStringClass, function:Format, format_arg:1, ellipsis_arg:2
	SHARED_API int _cdecl Format (_Printf_format_string_ const wchar_t *format, ...);
	SHARED_API int _cdecl Format_Args (_Printf_format_string_ const wchar_t *format, const va_list & arg_list );
	SHARED_API bool Convert_From (const char *text);
	bool Convert_To (StringClass &string);
	bool Convert_To (StringClass &string) const;
	SHARED_API bool Is_ANSI(void);
	wchar_t *		Get_Buffer (int new_length);
	wchar_t *		Peek_Buffer (void);
	const wchar_t*	Peek_Buffer() const;
	SHARED_API static void	Release_Resources (void);
	SHARED_API void Free_String();

  /*!
  * Finds any occurances of the search string within this string and replaces them with a specified
  * replacement string.
  *
  * \param[in] search
  *   The substring to be replaced
  * \param[in] replace
  *   The replacement string to insert
  * \param[in] bCaseSensitive
  *   True to perform a case sensitive search, false if case doesn't matter
  * \param[in] maxCount
  *   The maximum number of replacements to perform, or -1 to replace all instances
  *
  * \return
  *   The number of replacements that were made
  */
	SHARED_API int Replace(const wchar_t* search, const wchar_t* replace, bool bCaseSensitive = true, int maxCount=-1);

	void TruncateLeft(uint truncateLength)
	{
		uint length = Get_Length();
		if (length <= truncateLength)
			Free_String();

		else
		{
			int newLength = length - truncateLength;
			memmove(m_Buffer, m_Buffer + truncateLength, (newLength + 1)*2);
			Store_Length(newLength);
		}
	}
	void TruncateRight(uint truncateLength)
	{
		uint length = Get_Length();
		if (length <= truncateLength)
			Free_String();
		else
		{
			int newLength = length - truncateLength;
			m_Buffer[newLength] = L'\0';
			Store_Length(newLength);
		}
	}

	void TrimLeft()
	{
		wchar_t* iter = m_Buffer;
		for (; *iter != L'\0' && *iter <= L' '; iter++)
		{
		}
		TruncateLeft((int)(iter - m_Buffer));
	}

	void TrimRight()
	{
		wchar_t* iter = m_Buffer + Get_Length() - 1;
		for (; *iter != L'\0' && *iter <= L' '; iter--)
		{
		}
		TruncateRight((int)(m_Buffer + Get_Length() - 1 - iter));
	}

	void Trim()
	{
		TrimLeft();
		TrimRight();
	}

	//+V576, class:WideStringClass, function:getFormattedString, format_arg:1, ellipsis_arg:2
	static WideStringClass getFormattedString(_Printf_format_string_ const wchar_t* format, ...)
	{
		va_list arguments;
		WideStringClass result;

		va_start(arguments, format);
		result.Format_Args(format, arguments);
		va_end(arguments);

		return result;
	}

	SHARED_API WideStringClass Substring(int start, int length) const;
	SHARED_API void RemoveSubstring(int start, int length);
	SHARED_API void ReplaceSubstring(int start, int length, const WideStringClass& substring);

private:
	typedef struct _HEADER
	{
		int	allocated_length;
		int	length;
	} HEADER;
	enum
	{
		MAX_TEMP_STRING	= 8,
		MAX_TEMP_LEN		= 256,
		MAX_TEMP_BYTES		= (MAX_TEMP_LEN * sizeof (wchar_t)) + sizeof (HEADER),
	};
	SHARED_API void Get_String(int length,bool is_temp);
	wchar_t *		Allocate_Buffer (int length);
	SHARED_API void			Resize (int size);
	SHARED_API void			Uninitialised_Grow (int length);
	SHARED_API int Get_Temp_String_Index();
	bool Is_Temp_String();
	void Store_Length(int length);
	void Store_Allocated_Length(int length);
	HEADER *Get_Header() const;
	int Get_Allocated_Length() const;
	void Set_Buffer_And_Allocated_Length(wchar_t *buffer, int length);
	wchar_t* m_Buffer;
#if (SHARED_EXPORTS) || (EXTERNAL)
	static char __declspec(thread) TempStrings[MAX_TEMP_STRING][MAX_TEMP_BYTES];
	static unsigned int __declspec(thread) FreeTempStrings;
#endif
	SHARED_API static wchar_t * m_EmptyString;
	SHARED_API static wchar_t m_NullChar;
};

inline WideStringClass::WideStringClass (int initial_len, bool hint_temporary) : m_Buffer (m_EmptyString)
{
	if (hint_temporary || initial_len > 0) {
		Get_String(initial_len, hint_temporary);
		m_Buffer[0] = m_NullChar;
	}
}

inline WideStringClass::WideStringClass (wchar_t ch, bool hint_temporary) : m_Buffer (m_EmptyString)
{
	Get_String (2, hint_temporary);
	(*this) = ch;
}

inline WideStringClass::WideStringClass (const WideStringClass &string, bool hint_temporary) : m_Buffer (m_EmptyString)
{
	if (hint_temporary || (string.Get_Length()>1))
	{
		Get_String(string.Get_Length()+1, hint_temporary);
	}
	(*this) = string;
}

inline WideStringClass::WideStringClass (const WideStringView &string, bool hint_temporary) : m_Buffer (m_EmptyString)
{
	if (hint_temporary || (string.length()>1))
	{
		Get_String(int(string.length())+1, hint_temporary);
	}
	(*this) = string;
}

inline WideStringClass::WideStringClass (const wchar_t *string, bool hint_temporary) : m_Buffer (m_EmptyString)
{
	int len=string ? (int)wcslen(string) : 0;
	if (hint_temporary || len>0)
	{
		Get_String (len+1, hint_temporary);
	}
	(*this) = string;
}

inline WideStringClass::WideStringClass (const char *string, bool hint_temporary) : m_Buffer (m_EmptyString)
{
	if (hint_temporary || (string && string[0] != '\0'))
	{
		Get_String ((int)strlen(string) + 1, hint_temporary);
	}
	(*this) = string;
}

inline WideStringClass::WideStringClass(WideStringClass&& string) noexcept : m_Buffer(m_EmptyString)
{
	*this = std::move(string);
}

inline WideStringClass::~WideStringClass (void)
{
	if (m_Buffer != m_EmptyString)
		Free_String ();
}

inline bool WideStringClass::Is_Temp_String()
{
	return Get_Temp_String_Index() >= 0;
}

inline bool WideStringClass::Is_Empty (void) const
{
	return (m_Buffer[0] == m_NullChar);
}

inline int WideStringClass::Compare (const wchar_t *string) const
{
	if (string)
	{
		return wcscmp (m_Buffer, string);
	}
	return -1;
}

inline int WideStringClass::Compare_No_Case (const wchar_t *string) const
{
	if (string)
	{
		return _wcsicmp (m_Buffer, string);
	}
	return -1;
}

inline wchar_t WideStringClass::operator[] (int index) const
{
	return m_Buffer[index];
}

inline wchar_t& WideStringClass::operator[] (int index)
{
	return m_Buffer[index];
}

inline wchar_t WideStringClass::operator[] (size_t index) const
{
	return m_Buffer[index];
}

inline wchar_t& WideStringClass::operator[] (size_t index)
{
	return m_Buffer[index];
}

inline WideStringClass::operator const wchar_t * (void) const
{
	return m_Buffer;
}

inline bool WideStringClass::operator== (const wchar_t *rvalue) const
{
	return (Compare (rvalue) == 0);
}

inline bool WideStringClass::operator!= (const wchar_t *rvalue) const
{
	return (Compare (rvalue) != 0);
}

inline const WideStringClass & WideStringClass::operator= (const WideStringClass &string)
{	
	if (!(string.Is_Empty() && Is_Empty())) // don't bother allocating if we are assigning an empty/uninitialized string to an empty/uninitialized string
	{
		int len = string.Get_Length();
		Uninitialised_Grow(len + 1);
		Store_Length(len);
		memmove(m_Buffer, string.m_Buffer, (len + 1) * sizeof(wchar_t));
	}
	return (*this);
}

inline const WideStringClass & WideStringClass::operator= (const WideStringView &string)
{	
	if (!(string.empty() && Is_Empty())) // don't bother allocating if we are assigning an empty/uninitialized string to an empty/uninitialized string
	{
		int len = int(string.length());
		Uninitialised_Grow(len + 1);
		Store_Length(len);
		memmove(m_Buffer, string.data(), len * sizeof(wchar_t));
		m_Buffer[len] = L'\0'; // string views aren't guaranteed to be null terminated
	}
	return (*this);
}

inline const WideStringClass& WideStringClass::operator= (WideStringClass&& string) noexcept
{
	if (this != &string)
	{
		if (string.Is_Temp_String() && !Is_Temp_String())
		{
			// if the other string is a temp string, we sadly need to copy it, because otherwise we can run into weird issues
			// where e.g. a temp string is being returned from a function and moved into a member variable, permanently blocking a global temp string
			*this = string;
			string.Free_String();
		}
		else
		{
			if (m_Buffer != m_EmptyString)
				Free_String();

			m_Buffer = string.m_Buffer; // steal buffer
			string.m_Buffer = m_EmptyString;
		}
	}
	return (*this);
}

inline bool WideStringClass::operator < (const wchar_t *string) const
{
	if (string)
	{
		return (wcscmp (m_Buffer, string) < 0);
	}
	return false;
}

inline bool WideStringClass::operator <= (const wchar_t *string) const
{
	if (string)
	{
		return (wcscmp (m_Buffer, string) <= 0);
	}
	return false;
}

inline bool WideStringClass::operator > (const wchar_t *string) const
{
	if (string)
	{
		return (wcscmp (m_Buffer, string) > 0);
	}
	return true;
}

inline bool WideStringClass::operator >= (const wchar_t *string) const
{
	if (string)
	{
		return (wcscmp (m_Buffer, string) >= 0);
	}
	return true;
}

inline void WideStringClass::Erase (int start_index, int char_count)
{
	if (start_index < 0 || char_count <= 0)
		return;

	int len = Get_Length ();
	if (start_index < len)
	{
		if (start_index + char_count > len)
		{
			char_count = len - start_index;
		}
		memmove (&m_Buffer[start_index],&m_Buffer[start_index + char_count],(len - (start_index + char_count) + 1) * sizeof (wchar_t));
		Store_Length( (int)wcslen(m_Buffer) );
	}
}

inline const WideStringClass & WideStringClass::operator= (const wchar_t *string)
{
	if (string && !(string[0] == L'\0' && Is_Empty())) // don't bother allocating if we are assigning an empty string to an empty/uninitialized string
	{
		int len = (int)wcslen(string);
		Uninitialised_Grow(len + 1);
		Store_Length(len);
		memmove(m_Buffer, string, (len + 1) * sizeof(wchar_t));
	}
	return (*this);
}

inline const WideStringClass &WideStringClass::operator= (const char *string)
{
	Convert_From(string);
	return (*this);
}

inline const WideStringClass &WideStringClass::operator= (wchar_t ch)
{
	Uninitialised_Grow (2);
	m_Buffer[0] = ch;
	m_Buffer[1] = m_NullChar;
	Store_Length (1);
	return (*this);
}

inline const WideStringClass &WideStringClass::operator+= (const wchar_t *string)
{
	if (string)
	{
		int src_len = (int)wcslen (string);
		if (src_len > 0)
		{
			int cur_len = Get_Length();
			int new_len = cur_len + src_len;
			Resize(new_len + 1);
			Store_Length(new_len);
			memcpy(&m_Buffer[cur_len], string, (src_len + 1) * sizeof(wchar_t));
		}
	}
	return (*this);
}

inline const WideStringClass &WideStringClass::operator+= (wchar_t ch)
{
	int cur_len = Get_Length ();
	Resize (cur_len + 2);
	m_Buffer[cur_len] = ch;
	m_Buffer[cur_len + 1] = m_NullChar;
	if (ch != m_NullChar)
	{
		Store_Length (cur_len + 1);
	}
	return (*this);
}

//inline const WideStringClass& WideStringClass::operator+= (int16 i)
//{
//	constexpr size_t buf_size = Get_Integer_Print_String_Buf_Size(sizeof(i));
//	wchar_t buf[buf_size];
//	_snwprintf(buf, buf_size, L"%hd", i);
//	(*this) += buf;
//	return (*this);
//}
//inline const WideStringClass& WideStringClass::operator+= (uint16 i)
//{
//	constexpr size_t buf_size = Get_Integer_Print_String_Buf_Size(sizeof(i));
//	wchar_t buf[buf_size];
//	_snwprintf(buf, buf_size, L"%hu", i);
//	(*this) += buf;
//	return (*this);
//}
inline const WideStringClass& WideStringClass::operator+= (int32 i)
{
	constexpr size_t buf_size = Get_Integer_Print_String_Buf_Size(sizeof(i));
	wchar_t buf[buf_size];
	_snwprintf(buf, buf_size, L"%d", i);
	(*this) += buf;
	return (*this);
}
inline const WideStringClass& WideStringClass::operator+= (uint32 i)
{
	constexpr size_t buf_size = Get_Integer_Print_String_Buf_Size(sizeof(i));
	wchar_t buf[buf_size];
	_snwprintf(buf, buf_size, L"%u", i);
	(*this) += buf;
	return (*this);
}
inline const WideStringClass& WideStringClass::operator+= (int64 i)
{
	constexpr size_t buf_size = Get_Integer_Print_String_Buf_Size(sizeof(i));
	wchar_t buf[buf_size];
	_snwprintf(buf, buf_size, L"%lld", i);
	(*this) += buf;
	return (*this);
}
inline const WideStringClass& WideStringClass::operator+= (uint64 i)
{
	constexpr size_t buf_size = Get_Integer_Print_String_Buf_Size(sizeof(i));
	wchar_t buf[buf_size];
	_snwprintf(buf, buf_size, L"%llu", i);
	(*this) += buf;
	return (*this);
}
inline const WideStringClass& WideStringClass::operator+= (float f)
{
	constexpr size_t buf_size = 78;
	wchar_t buf[buf_size];
	_snwprintf(buf, buf_size, L"%f", f);
	(*this) += buf;
	return (*this);
}
inline const WideStringClass& WideStringClass::operator+= (double f)
{
	constexpr size_t buf_size = 377;
	wchar_t buf[buf_size];
	_snwprintf(buf, buf_size, L"%f", f);
	(*this) += buf;
	return (*this);
}

inline wchar_t *WideStringClass::Get_Buffer (int new_length)
{
	Uninitialised_Grow (new_length);
	return m_Buffer;
}

inline wchar_t *WideStringClass::Peek_Buffer (void)
{
	return m_Buffer;
}

inline const wchar_t* WideStringClass::Peek_Buffer() const
{
	return m_Buffer;
}

inline const WideStringClass &WideStringClass::operator+= (const WideStringClass &string)
{
	int src_len = string.Get_Length();
	if (src_len > 0)
	{
		int cur_len = Get_Length ();
		int new_len = cur_len + src_len;
		Resize (new_len + 1);
		Store_Length (new_len);
		memcpy (&m_Buffer[cur_len], (const wchar_t *)string, (src_len + 1) * sizeof (wchar_t));				
	}
	return (*this);
}

inline WideStringClass operator+ (const WideStringClass &string1, const WideStringClass &string2)
{
	WideStringClass new_string(string1, true);
	new_string += string2;
	return new_string;
}

inline WideStringClass operator+ (const wchar_t *string1, const WideStringClass &string2)
{
	WideStringClass new_string(string1, true);
	new_string += string2;
	return new_string;
}

inline WideStringClass operator+ (const WideStringClass &string1, const wchar_t *string2)
{
	WideStringClass new_string(string1, true);
	new_string += string2;
	return new_string;
}

inline int WideStringClass::Get_Allocated_Length (void) const
{
	int allocated_length = 0;
	if (m_Buffer != m_EmptyString)
	{
		HEADER *header = Get_Header ();
		allocated_length = header->allocated_length;		
	}
	return allocated_length;
}

inline int WideStringClass::Get_Length (void) const
{
	int length = 0;
	if (m_Buffer != m_EmptyString)
	{
		HEADER *header = Get_Header ();
		length = header->length;
		if (length == 0)
		{
			length = (int)wcslen (m_Buffer);
			((WideStringClass *)this)->Store_Length (length);
		}
	}
	return length;
}

inline void WideStringClass::Set_Buffer_And_Allocated_Length (wchar_t *buffer, int length)
{
	if (m_Buffer != m_EmptyString)
		Free_String ();

	m_Buffer = buffer;
	if (m_Buffer != m_EmptyString)
	{
		Store_Allocated_Length (length);
		Store_Length (0);		
	}
}

inline wchar_t * WideStringClass::Allocate_Buffer (int length)
{
	char *buffer = new char[(sizeof (wchar_t) * length) + sizeof (WideStringClass::_HEADER)];
	HEADER *header = reinterpret_cast<HEADER *>(buffer);
	header->length = 0;
	header->allocated_length = length;
	return reinterpret_cast<wchar_t *>(buffer + sizeof (WideStringClass::_HEADER));
}

inline WideStringClass::HEADER * WideStringClass::Get_Header (void) const
{
	return reinterpret_cast<HEADER *>(((char *)m_Buffer) - sizeof (WideStringClass::_HEADER));
}

inline void WideStringClass::Store_Allocated_Length (int allocated_length)
{
	if (m_Buffer != m_EmptyString)
	{
		HEADER *header = Get_Header ();
		header->allocated_length = allocated_length;
	}
}

inline void WideStringClass::Store_Length (int length)
{
	if (m_Buffer != m_EmptyString)
	{
		HEADER *header = Get_Header ();
		header->length = length;
	}
}

inline bool WideStringClass::Convert_To (StringClass &string)
{
	return (string.Copy_Wide (m_Buffer));
}

inline bool WideStringClass::Convert_To (StringClass &string) const
{
	return (string.Copy_Wide (m_Buffer));
}

struct hash_istring
{
	// NOTE(Mara): this is for "heterogeneous lookup", e.g. looking up in a map with key StringClass using a char* without allocating
	using is_transparent = void;

    size_t operator()(const char* str) const noexcept
    {
		return IStringHashFunc(str);
    }

    size_t operator()(const StringClass& str) const noexcept
    {
        return IStringHashFunc(str);
    }

    size_t operator()(const StringView& str) const noexcept
    {
        return IStringHashFunc(str);
    }
};

struct hash_iwstring
{
	// NOTE(Mara): this is for "heterogeneous lookup", e.g. looking up in a map with key StringClass using a char* without allocating
	using is_transparent = void;

	size_t operator()(const wchar_t* str) const noexcept
	{
		return IStringHashFunc(str);
	}

	size_t operator()(const WideStringClass& str) const noexcept
	{
		return IStringHashFunc(str);
	}

	size_t operator()(const WideStringView& str) const noexcept
	{
		return IStringHashFunc(str);
	}
};

struct equals_istring
{
	// NOTE(Mara): this is for "heterogeneous lookup", e.g. looking up in a map with key StringClass using a char* without allocating
	using is_transparent = void;

	// NOTE(Mara): StringView isn't guaranteed to be null terminated

	bool operator()(const char* a, const char* b) const noexcept
	{
		return _stricmp(a, b) == 0;
	}

	bool operator()(const StringClass& a, const StringClass& b) const noexcept
	{
		return _stricmp(a.Peek_Buffer(), b.Peek_Buffer()) == 0;
	}

	bool operator()(const StringView& a, const StringView& b) const noexcept
	{
		return (a.length() == b.length()) && (_strnicmp(a.data(), b.data(), a.length()) == 0);
	}

	bool operator()(const StringClass& a, const StringView& b) const noexcept
	{
		return (_strnicmp(a.Peek_Buffer(), b.data(), b.length()) == 0) && (a[b.length()] == '\0');
	}
	
	bool operator()(const StringView& a, const StringClass& b) const noexcept
	{
		return (_strnicmp(b.Peek_Buffer(), a.data(), a.length()) == 0) && (b[a.length()] == '\0');
	}

	bool operator()(const StringClass& a, const char* b) const noexcept
	{
		return _stricmp(a.Peek_Buffer(), b) == 0;
	}

	bool operator()(const StringView& a, const char* b) const noexcept
	{
		return (_strnicmp(a.data(), b, a.length()) == 0) && (b[a.length()] == '\0');
	}

	bool operator()(const char* a, const StringClass& b) const noexcept
	{
		return _stricmp(a, b.Peek_Buffer()) == 0;
	}

	bool operator()(const char* a, const StringView& b) const noexcept
	{
		return (_strnicmp(a, b.data(), b.length()) == 0) && (a[b.length()] == '\0');
	}
};


struct equals_iwstring 
{
	// NOTE(Mara): this is for "heterogeneous lookup", e.g. looking up in a map with key StringClass using a char* without allocating
	using is_transparent = void;

	// NOTE(Mara): WideStringView isn't guaranteed to be null terminated

	bool operator()(const wchar_t* a, const wchar_t* b) const noexcept
	{
		return _wcsicmp(a, b) == 0;
	}

	bool operator()(const WideStringClass& a, const WideStringClass& b) const noexcept
	{
		return _wcsicmp(a.Peek_Buffer(), b.Peek_Buffer()) == 0;
	}

	bool operator()(const WideStringView& a, const WideStringView& b) const noexcept
	{
		return (a.length() == b.length()) && (_wcsnicmp(a.data(), b.data(), a.length()) == 0);
	}

	bool operator()(const WideStringClass& a, const WideStringView& b) const noexcept
	{
		return (_wcsnicmp(a.Peek_Buffer(), b.data(), b.length()) == 0) && (a[b.length()] == '\0');
	}

	bool operator()(const WideStringView& a, const WideStringClass& b) const noexcept
	{
		return (_wcsnicmp(b.Peek_Buffer(), a.data(), a.length()) == 0) && (b[a.length()] == '\0');
	}

	bool operator()(const WideStringClass& a, const wchar_t* b) const noexcept
	{
		return _wcsicmp(a.Peek_Buffer(), b) == 0;
	}

	bool operator()(const WideStringView& a, const wchar_t* b) const noexcept
	{
		return (_wcsnicmp(a.data(), b, a.length()) == 0) && (b[a.length()] == '\0');
	}

	bool operator()(const wchar_t* a, const WideStringClass& b) const noexcept
	{
		return _wcsicmp(a, b.Peek_Buffer()) == 0;
	}

	bool operator()(const wchar_t* a, const WideStringView& b) const noexcept
	{
		return (_wcsnicmp(a, b.data(), b.length()) == 0) && (a[b.length()] == '\0');
	}
};

struct hash_string
{
	// NOTE(Mara): this is for "heterogeneous lookup", e.g. looking up in a map with key StringClass using a char* without allocating
	using is_transparent = void;

    size_t operator()(const char* str) const noexcept
    {
		return StringHashFunc(str);
    }

    size_t operator()(const StringClass& str) const noexcept
    {
        return StringHashFunc(str);
    }

    size_t operator()(const StringView& str) const noexcept
    {
        return StringHashFunc(str);
    }
};

struct hash_wstring
{
	// NOTE(Mara): this is for "heterogeneous lookup", e.g. looking up in a map with key StringClass using a char* without allocating
	using is_transparent = void;

	size_t operator()(const wchar_t* str) const noexcept
	{
		return StringHashFunc(str);
	}

	size_t operator()(const WideStringClass& str) const noexcept
	{
		return StringHashFunc(str);
	}

	size_t operator()(const WideStringView& str) const noexcept
	{
		return StringHashFunc(str);
	}
};

struct equals_string
{
	// NOTE(Mara): this is for "heterogeneous lookup", e.g. looking up in a map with key StringClass using a char* without allocating
	using is_transparent = void;

	// NOTE(Mara): StringView isn't guaranteed to be null terminated

	bool operator()(const char* a, const char* b) const noexcept
	{
		return strcmp(a, b) == 0;
	}

	bool operator()(const StringClass& a, const StringClass& b) const noexcept
	{
		return strcmp(a.Peek_Buffer(), b.Peek_Buffer()) == 0;
	}

	bool operator()(const StringView& a, const StringView& b) const noexcept
	{
		return (a == b);
	}

	bool operator()(const StringClass& a, const StringView& b) const noexcept
	{
		return (strncmp(a.Peek_Buffer(), b.data(), b.length()) == 0) && (a[b.length()] == '\0');
	}

	bool operator()(const StringView& a, const StringClass& b) const noexcept
	{
		return (strncmp(b.Peek_Buffer(), a.data(), a.length()) == 0) && (b[a.length()] == '\0');
	}

	bool operator()(const StringClass& a, const char* b) const noexcept
	{
		return strcmp(a.Peek_Buffer(), b) == 0;
	}

	bool operator()(const StringView& a, const char* b) const noexcept
	{
		return (strncmp(a.data(), b, a.length()) == 0) && (b[a.length()] == '\0');
	}

	bool operator()(const char* a, const StringClass& b) const noexcept
	{
		return strcmp(a, b.Peek_Buffer()) == 0;
	}

	bool operator()(const char* a, const StringView& b) const noexcept
	{
		return (strncmp(a, b.data(), b.length()) == 0) && (a[b.length()] == '\0');
	}
};

struct equals_wstring
{
	// NOTE(Mara): this is for "heterogeneous lookup", e.g. looking up in a map with key StringClass using a char* without allocating
	using is_transparent = void;

	// NOTE(Mara): WideStringView isn't guaranteed to be null terminated

	bool operator()(const wchar_t* a, const wchar_t* b) const noexcept
	{
		return wcscmp(a, b) == 0;
	}

	bool operator()(const WideStringClass& a, const WideStringClass& b) const noexcept
	{
		return wcscmp(a.Peek_Buffer(), b.Peek_Buffer()) == 0;
	}

	bool operator()(const WideStringView& a, const WideStringView& b) const noexcept
	{
		return (a == b);
	}

	bool operator()(const WideStringClass& a, const WideStringView& b) const noexcept
	{
		return (wcsncmp(a.Peek_Buffer(), b.data(), b.length()) == 0) && (a[b.length()] == '\0');
	}

	bool operator()(const WideStringView& a, const WideStringClass& b) const noexcept
	{
		return (wcsncmp(b.Peek_Buffer(), a.data(), a.length()) == 0) && (b[a.length()] == '\0');
	}

	bool operator()(const WideStringClass& a, const wchar_t* b) const noexcept
	{
		return wcscmp(a.Peek_Buffer(), b) == 0;
	}

	bool operator()(const WideStringView& a, const wchar_t* b) const noexcept
	{
		return (wcsncmp(a.data(), b, a.length()) == 0) && (b[a.length()] == '\0');
	}

	bool operator()(const wchar_t* a, const WideStringClass& b) const noexcept
	{
		return wcscmp(a, b.Peek_Buffer()) == 0;
	}

	bool operator()(const wchar_t* a, const WideStringView& b) const noexcept
	{
		return (wcsncmp(a, b.data(), b.length()) == 0) && (a[b.length()] == '\0');
	}
};

// sort in the order Windows Explorer does
struct string_explorer_sort
{
	// WORKAROUND: we want to use StrCmpLogicalW() instead of _wcsicmp() here as the former is what the Windows shell uses to sort files.
	// Unfortunately StrCmpLogicalW() is implemented slightly differently on Wine/Linux which leads to always*.dats being loaded in the wrong order.
	// Eventually this should get fixed, see https://bugs.winehq.org/show_bug.cgi?id=49778 and https://bugs.winehq.org/show_bug.cgi?id=10767
	bool operator()(const StringClass& a, const StringClass& b) const noexcept {
		return _wcsicmp(WideStringClass(a, true), WideStringClass(b, true)) < 0;
	}
	bool operator()(const WideStringClass& a, const WideStringClass& b) const noexcept {
		return _wcsicmp(a, b) < 0;
	}
};
// sort in the reverse order Windows Explorer does
struct string_reverse_explorer_sort
{
	// WORKAROUND: we want to use StrCmpLogicalW() instead of _wcsicmp() here as the former is what the Windows shell uses to sort files.
	// Unfortunately StrCmpLogicalW() is implemented slightly differently on Wine/Linux which leads to always*.dats being loaded in the wrong order.
	// Eventually this should get fixed, see https://bugs.winehq.org/show_bug.cgi?id=49778 and https://bugs.winehq.org/show_bug.cgi?id=10767
	bool operator()(const StringClass& a, const StringClass& b) const noexcept {
		return _wcsicmp(WideStringClass(a, true), WideStringClass(b, true)) > 0;
	}
	bool operator()(const WideStringClass& a, const WideStringClass& b) const noexcept {
		return _wcsicmp(a, b) > 0;
	}
};

SCRIPTS_API char *newstr(const char *str); //duplicate a character string
SCRIPTS_API wchar_t *newwcs(const wchar_t *str);  //duplicate a wide character string
SCRIPTS_API char *strtrim(char *); //trim a string
SCRIPTS_API char* strrtrim(char *); //trim trailing whitespace from a string
SCRIPTS_API const char *stristr(const char *str, const char *substr); //like strstr but case insenstive
SCRIPTS_API const wchar_t *wcsistr(const wchar_t *str, const wchar_t *substr); //like strstr but case insenstive and for wchar_t
#endif
