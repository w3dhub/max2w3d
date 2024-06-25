#pragma once

class StringView {
	const char* m_ptr;
	size_t m_size;
public:
	TT_INLINE constexpr StringView(const char* str) noexcept : m_ptr(str), m_size(__builtin_strlen(str)) {}
	TT_INLINE constexpr StringView(const char* begin_, const char* end_) noexcept : m_ptr(begin_), m_size(size_t(end_ - begin_)) {}
	TT_INLINE constexpr StringView(const char* begin_, size_t size_) noexcept : m_ptr(begin_), m_size(size_) {}
	TT_INLINE constexpr StringView(const StringView& other) noexcept = default;
	TT_INLINE constexpr StringView(std::nullptr_t) noexcept = delete;
	TT_INLINE constexpr const char* begin() const noexcept { return m_ptr; }
	TT_INLINE constexpr const char* data() const noexcept { return m_ptr; }
	TT_INLINE constexpr const char* end() const noexcept { return m_ptr + m_size; }
	TT_INLINE constexpr size_t length() const noexcept { return m_size; }
	TT_INLINE constexpr size_t size() const noexcept { return m_size; }
	TT_INLINE constexpr bool empty() const noexcept { return m_size == 0; }
	TT_INLINE constexpr char operator[](size_t i) const noexcept { return m_ptr[i]; }
	TT_INLINE constexpr bool operator==(const StringView& other) const noexcept {
		return (this->m_size == other.m_size) && ((this->m_ptr == other.m_ptr) || (!__builtin_memcmp(this->m_ptr, other.m_ptr, m_size)));
	}
};

class WideStringView {
	const wchar_t* m_ptr;
	size_t m_size;
public:
	TT_INLINE constexpr WideStringView(const wchar_t* str) noexcept : m_ptr(str), m_size(__builtin_wcslen(str)) {}
	TT_INLINE constexpr WideStringView(const wchar_t* begin_, const wchar_t* end_) noexcept : m_ptr(begin_), m_size(size_t(end_ - begin_)) {}
	TT_INLINE constexpr WideStringView(const wchar_t* begin_, size_t size_) noexcept : m_ptr(begin_), m_size(size_) {}
	TT_INLINE constexpr WideStringView(const WideStringView& other) noexcept = default;
	TT_INLINE constexpr WideStringView(std::nullptr_t) noexcept = delete;
	TT_INLINE constexpr const wchar_t* begin() const noexcept { return m_ptr; }
	TT_INLINE constexpr const wchar_t* data() const noexcept { return m_ptr; }
	TT_INLINE constexpr const wchar_t* end() const noexcept { return m_ptr + m_size; }
	TT_INLINE constexpr size_t length() const noexcept { return m_size; }
	TT_INLINE constexpr size_t size() const noexcept { return m_size; }
	TT_INLINE constexpr bool empty() const noexcept { return m_size == 0; }
	TT_INLINE constexpr wchar_t operator[](size_t i) const noexcept { return m_ptr[i]; }
	TT_INLINE constexpr bool operator==(const WideStringView& other) const noexcept {
		return (this->m_size == other.m_size) && ((this->m_ptr == other.m_ptr) || (!__builtin_wmemcmp(this->m_ptr, other.m_ptr, m_size)));
	}
};
