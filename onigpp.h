// onigpp.h --- Oniguruma++ (onigpp) regular expression engine
// Author: katahiromz
// License: BSD-2-Clause
#pragma once

#include <string>
#include <vector>
#include <exception>
#include <stdexcept>
#include <algorithm>
#include <cstring>
#include <cassert>
#include <locale>

// Oniguruma
#define ONIG_ESCAPE_UCHAR_COLLISION // Use UnigUChar instead of UChar
#define ONIG_ESCAPE_REGEX_T_COLLISION // Use OnigRegexType instead of regex_t
#include "oniguruma/src/oniguruma.h"

namespace onigpp {

////////////////////////////////////////////
// onigpp::init, uninit, version

int init(const OnigEncoding *encodings = nullptr, size_t encodings_count = 0);
void uninit();
const char* version();

////////////////////////////////////////////
// onigpp::auto_init

struct auto_init {
	auto_init(const OnigEncoding *encodings = nullptr, size_t encodings_count = 0) {
		init(encodings, encodings_count);
	}
	~auto_init() { uninit(); }
};

////////////////////////////////////////////
// onigpp::size_type

using size_type = std::size_t;

////////////////////////////////////////////
// onigpp::basic_string

template <class CharT>
using basic_string = std::basic_string<CharT>;

using string = basic_string<char>;
using wstring = basic_string<wchar_t>;
using u16string = basic_string<char16_t>;
using u32string = basic_string<char32_t>;

////////////////////////////////////////////
// onigpp::regex_constants

namespace regex_constants {
	// Error types
	using error_type = int;

	// Syntax options
	using syntax_option_type = unsigned long;
	static constexpr syntax_option_type icase = (1 << 0);
	static constexpr syntax_option_type multiline = (1 << 1);
	static constexpr syntax_option_type extended = (1 << 2);
	static constexpr syntax_option_type basic = (1 << 11);
	static constexpr syntax_option_type awk = (1 << 12);
	static constexpr syntax_option_type grep = (1 << 13);
	static constexpr syntax_option_type egrep = (1 << 14);
	static constexpr syntax_option_type ECMAScript = (1 << 15);

	// Search/Match control flags
	using match_flag_type = unsigned long;

	// Directly mappable to Oniguruma API
	static constexpr match_flag_type match_not_bol = (1 << 3); // ONIG_OPTION_NOTBOL
	static constexpr match_flag_type match_not_eol = (1 << 4); // ONIG_OPTION_NOTEOL
	static constexpr match_flag_type match_any = (1 << 5);

	// Flags processed by logic (as there are no direct Oniguruma options)
	static constexpr match_flag_type match_not_null = (1 << 6); // Disallow zero-width match (treat length 0 match as failure)
	static constexpr match_flag_type match_prev_avail = (1 << 7); // For \b, \B, etc. determination

	// Format control flags for replacement
	static constexpr match_flag_type format_first_only = (1 << 8);
	static constexpr match_flag_type format_no_copy = (1 << 9);
	static constexpr match_flag_type format_literal = (1 << 10);

	// Default values
	// Note: In std::regex, ECMAScript is the default grammar when no flags are specified
	static constexpr syntax_option_type normal = ECMAScript;
	static constexpr match_flag_type format_default = 0;
	static constexpr match_flag_type match_default = 0;
}

////////////////////////////////////////////
// onigpp::encoding_constants

namespace encoding_constants {
	// Expose pointers to encodings provided by Oniguruma
	// Using static instead of inline for C++11 compatibility
	static OnigEncoding const ASCII  = ONIG_ENCODING_ASCII;
	static OnigEncoding const UTF8 = ONIG_ENCODING_UTF8;
	static OnigEncoding const UTF16LE = ONIG_ENCODING_UTF16_LE;
	static OnigEncoding const UTF16BE = ONIG_ENCODING_UTF16_BE;
	static OnigEncoding const UTF32LE = ONIG_ENCODING_UTF32_LE;
	static OnigEncoding const UTF32BE = ONIG_ENCODING_UTF32_BE;
	static OnigEncoding const LATIN1 = ONIG_ENCODING_ISO_8859_1;
	static OnigEncoding const SHIFT_JIS = ONIG_ENCODING_SJIS;
	static OnigEncoding const EUC_JP = ONIG_ENCODING_EUC_JP;
	// Other encodings can be added as needed
}

////////////////////////////////////////////
// onigpp::regex_error

class regex_error : public std::exception {
public:
	// Construct with error code and OnigErrorInfo (copy)
	regex_error(regex_constants::error_type ecode, const OnigErrorInfo& err_info) : m_err_code(ecode), m_err_info(err_info) {
		OnigUChar err_buf[ONIG_MAX_ERROR_MESSAGE_LEN];
		onig_error_code_to_str(err_buf, static_cast<int>(m_err_code), &m_err_info);
		m_message.assign(reinterpret_cast<char*>(err_buf));
	}

	virtual ~regex_error() = default;

	regex_constants::error_type code() const { return m_err_code; }
	const char* what() const noexcept override { return m_message.c_str(); }

protected:
	regex_constants::error_type m_err_code;
	OnigErrorInfo m_err_info;
	std::string m_message; // holds the formatted error message to ensure stable lifetime
};

////////////////////////////////////////////
// onigpp::sub_match<BidirIt>

template <class BidirIt>
class sub_match : public std::pair<BidirIt, BidirIt> {
public:
	using iterator = BidirIt;
	using value_type = typename std::iterator_traits<BidirIt>::value_type;
	using difference_type = typename std::iterator_traits<BidirIt>::difference_type;
	using string_type = basic_string<value_type>;

	bool matched;

	// Default constructor
	sub_match() : std::pair<BidirIt, BidirIt>(), matched(false) {}
	
	// Added 3-argument constructor
	sub_match(BidirIt first, BidirIt second, bool is_matched) 
		: std::pair<BidirIt, BidirIt>(first, second), matched(is_matched) {}

	string_type str() const { return string_type(this->first, this->second); }
};

using csub_match = sub_match<const char*>;
using wcsub_match = sub_match<const wchar_t*>;
using u16csub_match = sub_match<const char16_t*>;
using u32csub_match = sub_match<const char32_t*>;
using ssub_match = sub_match<string::const_iterator>;
using wssub_match = sub_match<wstring::const_iterator>;
using u16ssub_match = sub_match<u16string::const_iterator>;
using u32ssub_match = sub_match<u32string::const_iterator>;

////////////////////////////////////////////
// onigpp::regex_traits<CharT>

template <class CharT>
class regex_traits {
public:
	using char_type = CharT;
	using string_type = basic_string<CharT>;
	static size_type length(const char_type* s) {
		return std::char_traits<char_type>::length(s);
	}
};

////////////////////////////////////////////
// onigpp::match_results<BidirIt, Allocator>

template <class BidirIt, class Alloc = std::allocator<sub_match<BidirIt>>>
class match_results : public std::vector<sub_match<BidirIt>, Alloc> {
public:
	using value_type = sub_match<BidirIt>;
	using const_reference = const value_type&;
	using reference = value_type&;
	using const_iterator = typename std::vector<sub_match<BidirIt>, Alloc>::const_iterator;
	using iterator = typename std::vector<sub_match<BidirIt>, Alloc>::iterator;
	using difference_type = typename std::iterator_traits<BidirIt>::difference_type;
	using size_type = typename std::allocator_traits<Alloc>::size_type;
	using char_type = typename std::iterator_traits<BidirIt>::value_type;
	using string_type = basic_string<char_type>;

	match_results() : m_str_begin(), m_str_end() {}

	// Added convenience functions
	bool empty() const { return this->size() == 0; }

	// Shortcut to the entire matched string
	string_type str(size_type n = 0) const {
		return (*this)[n].str();
	}

	// Prefix and suffix
	const value_type prefix() const;
	const value_type suffix() const;

public:
	BidirIt m_str_begin; // Start iterator of the search range
	BidirIt m_str_end;   // End iterator of the search range
};

////////////////////////////////////////////
// Forward declarations

template <class CharT, class Traits> class basic_regex;

////////////////////////////////////////////
// onigpp::basic_regex<CharT>

template <class CharT, class Traits = regex_traits<CharT>>
class basic_regex {
public:
	using value_type = CharT;
	using traits_type = Traits;
	using string_type = typename Traits::string_type;
	using flag_type = regex_constants::syntax_option_type;
	using self_type = basic_regex<CharT,Traits>;
	using locale_type = std::locale;

public:
	basic_regex() : m_regex(nullptr), m_encoding(nullptr), m_flags(regex_constants::normal), m_locale(std::locale()) { }
	explicit basic_regex(const CharT* s, flag_type f = regex_constants::normal, OnigEncoding enc = nullptr)
		: basic_regex(s, Traits::length(s), f, enc) { }
	basic_regex(const CharT* s, size_type count, flag_type f = regex_constants::normal, OnigEncoding enc = nullptr);
	basic_regex(const string_type& s, flag_type f = regex_constants::normal, OnigEncoding enc = nullptr);
	
	// Iterator-range constructor
	template <class BidiIterator>
	basic_regex(BidiIterator first, BidiIterator last, flag_type f = regex_constants::normal, OnigEncoding enc = nullptr, const locale_type& loc = std::locale())
		: m_regex(nullptr), m_encoding(enc), m_flags(f), m_pattern(), m_locale(loc)
	{
		// Build a string_type from iterator range and delegate to existing ctor logic
		string_type s(first, last);
		self_type tmp(s.c_str(), s.length(), f, enc);
		swap(tmp);
	}
	
	basic_regex(const self_type& other);
	basic_regex(self_type&& other) noexcept;
	virtual ~basic_regex();

	self_type& operator=(const self_type& other) {
		self_type tmp(other);
		swap(tmp);
		return *this;
	}
	self_type& operator=(self_type&& other) noexcept;
	self_type& operator=(const CharT* ptr) { return assign(ptr); }

	self_type& assign(const self_type& other) {
		self_type tmp(other);
		swap(tmp);
		return *this;
	}
	self_type& assign(self_type&& other) noexcept {
		swap(other);
		return *this;
	}
	self_type& assign(const CharT* ptr, flag_type f = regex_constants::normal, OnigEncoding enc = nullptr) {
		if (!enc) enc = m_encoding;
		self_type tmp(ptr, f, enc);
		swap(tmp);
		return *this;
	}
	self_type& assign(const string_type& str, flag_type f = regex_constants::normal, OnigEncoding enc = nullptr) {
		if (!enc) enc = m_encoding;
		self_type tmp(str.c_str(), str.length(), f, enc);
		swap(tmp);
		return *this;
	}
	
	// Iterator-range assign
	template <class BidiIterator>
	self_type& assign(BidiIterator first, BidiIterator last, flag_type f = regex_constants::normal, OnigEncoding enc = nullptr) {
		if (!enc) enc = m_encoding;
		string_type s(first, last);
		self_type tmp(s.c_str(), s.length(), f, enc);
		swap(tmp);
		return *this;
	}

	unsigned mark_count() const;
	flag_type flags() const { return m_flags; }
	const string_type& pattern() const noexcept { return m_pattern; }

	void swap(self_type& other) noexcept {
		std::swap(m_regex, other.m_regex);
		std::swap(m_encoding, other.m_encoding);
		std::swap(m_flags, other.m_flags);
		std::swap(m_pattern, other.m_pattern);
		std::swap(m_locale, other.m_locale);
	}

	template <class, class> friend struct regex_access;

	locale_type getloc() const { return m_locale; }
	locale_type imbue(locale_type loc);

protected:
	OnigRegex m_regex;
	OnigEncoding m_encoding;
	flag_type m_flags;
	string_type m_pattern;
	locale_type m_locale;

	static OnigOptionType _options_from_flags(flag_type f);
	static OnigSyntaxType* _syntax_from_flags(flag_type f);
	string_type _preprocess_pattern_for_locale(const string_type& pattern) const;
	string_type _preprocess_pattern_for_ecmascript(const string_type& pattern) const;
};

////////////////////////////////////////////
// onigpp::regex, onigpp::wregex

using regex = basic_regex<char>;
using wregex = basic_regex<wchar_t>;
using u16regex = basic_regex<char16_t>;
using u32regex = basic_regex<char32_t>;

////////////////////////////////////////////
// onigpp::regex_iterator

template <class BidirIt, class CharT = typename std::iterator_traits<BidirIt>::value_type, class Traits = regex_traits<CharT>>
class regex_iterator {
public:
	using iterator_category = std::forward_iterator_tag;
	using value_type = match_results<BidirIt>;
	using difference_type = std::ptrdiff_t;
	using pointer = const value_type*;
	using reference = const value_type&;
	using regex_type = basic_regex<CharT, Traits>;
	using match_flag_type = regex_constants::match_flag_type;
	using self_type = regex_iterator<BidirIt, CharT, Traits>;

protected:
	value_type m_results;
	BidirIt m_end;
	const regex_type* m_regex;
	match_flag_type m_flags;

	// Added: store the original begin iterator of the whole search range so that
	// onig_search can be called with the correct 'str' (whole begin) and 'start'.
	// This preserves previous-character context used by \b, \B, etc.
	BidirIt m_begin;

	// Helper function: search for the next match
	void do_search(BidirIt first, BidirIt last);

public:
	// 1. End-of-Sequence constructor
	regex_iterator() : m_regex(nullptr) {}

	// 2. Value constructor (starts search)
	regex_iterator(BidirIt first, BidirIt last,
				   const regex_type& re,
				   match_flag_type flags = regex_constants::match_default);

	// 3. Reference, pointer, comparison
	reference operator*() const { return m_results; }
	pointer operator->() const { return &m_results; }

	bool operator==(const regex_iterator& other) const;
	bool operator!=(const regex_iterator& other) const {
		return !(*this == other);
	}

	// 4. Increment (move to the next match)
	self_type& operator++(); // Pre-increment
	self_type operator++(int); // Post-increment
};

using cregex_iterator = regex_iterator<const char*>;
using wcregex_iterator = regex_iterator<const wchar_t*>;
using u16cregex_iterator = regex_iterator<const char16_t*>;
using u32cregex_iterator = regex_iterator<const char32_t*>;

using sregex_iterator = regex_iterator<string::const_iterator, char>;
using wsregex_iterator = regex_iterator<wstring::const_iterator, wchar_t>;
using u16sregex_iterator = regex_iterator<u16string::const_iterator, char16_t>;
using u32sregex_iterator = regex_iterator<u32string::const_iterator, char32_t>;

////////////////////////////////////////////
// onigpp::regex_token_iterator

template <class BidirIt, class CharT = typename std::iterator_traits<BidirIt>::value_type, class Traits = regex_traits<CharT>>
class regex_token_iterator {
public:
	using iterator_category = std::forward_iterator_tag;
	using value_type = sub_match<BidirIt>;
	using difference_type = std::ptrdiff_t;
	using pointer = const value_type*;
	using reference = const value_type&;

	using regex_type = basic_regex<CharT, Traits>;
	using match_flag_type = regex_constants::match_flag_type;
	using regex_iterator_type = regex_iterator<BidirIt, CharT, Traits>;

protected:
	regex_iterator_type m_itor;
	regex_iterator_type m_end;
	std::vector<int> m_subs;
	size_type m_subs_idx;

	// Added member to hold the end iterator of the string
	BidirIt m_last;

	// Current token value
	sub_match<BidirIt> m_sub_match;

	// Helper function: set the next token value
	void _do_increment();

public:
	// 1. End-of-Sequence constructor
	regex_token_iterator() : m_subs_idx(0) {}

	// 2. Value constructor (starts search)
	regex_token_iterator(BidirIt first, BidirIt last,
						 const regex_type& re,
						 const std::vector<int>& subs = {-1},
						 match_flag_type flags = regex_constants::match_default);

	// 3. Reference, pointer, comparison
	reference operator*() const { return m_sub_match; }
	pointer operator->() const { return &m_sub_match; }

	bool operator==(const regex_token_iterator& other) const;
	bool operator!=(const regex_token_iterator& other) const {
		return !(*this == other);
	}

	// 4. Increment (move to the next token)
	regex_token_iterator& operator++(); // Pre-increment
	regex_token_iterator operator++(int); // Post-increment
};

using cregex_token_iterator = regex_token_iterator<const char*>;
using wcregex_token_iterator = regex_token_iterator<const wchar_t*>;

using sregex_token_iterator = regex_token_iterator<string::const_iterator>;
using wsregex_token_iterator = regex_token_iterator<wstring::const_iterator>;

////////////////////////////////////////////
// match_results

template <class BidirIt, class Alloc>
inline const sub_match<BidirIt> match_results<BidirIt, Alloc>::prefix() const {
	if (this->empty()) {
		return sub_match<BidirIt>(m_str_begin, m_str_begin, false);
	}
	return sub_match<BidirIt>(m_str_begin, (*this)[0].first, true);
}

template <class BidirIt, class Alloc>
inline const sub_match<BidirIt> match_results<BidirIt, Alloc>::suffix() const {
	if (this->empty()) {
		return sub_match<BidirIt>(m_str_end, m_str_end, false);
	}
	return sub_match<BidirIt>((*this)[0].second, m_str_end, true);
}

////////////////////////////////////////////
// regex_match

template <class BidirIt, class Alloc, class CharT, class Traits>
bool regex_match(
	BidirIt first, BidirIt last,
	match_results<BidirIt, Alloc>& m,
	const basic_regex<CharT, Traits>& e,
	regex_constants::match_flag_type flags = regex_constants::match_default);

// C-string overload
template <class CharT, class Alloc, class Traits>
inline bool regex_match(
	const CharT* str,
	match_results<const CharT*, Alloc>& m,
	const basic_regex<CharT, Traits>& e,
	regex_constants::match_flag_type flags = regex_constants::match_default)
{
	const CharT* end = str + Traits::length(str);
	return regex_match(str, end, m, e, flags);
}

// std::string overload
template <class Alloc, class CharT, class Traits>
inline bool regex_match(
	const basic_string<CharT>& s,
	match_results<typename basic_string<CharT>::const_iterator, Alloc>& m,
	const basic_regex<CharT, Traits>& e,
	regex_constants::match_flag_type flags = regex_constants::match_default)
{
	return regex_match(s.begin(), s.end(), m, e, flags);
}

// Version without match_results
template <class BidirIt, class CharT, class Traits>
inline bool regex_match(
	BidirIt first, BidirIt last,
	const basic_regex<CharT, Traits>& e,
	regex_constants::match_flag_type flags = regex_constants::match_default)
{
	match_results<BidirIt> m;
	return regex_match(first, last, m, e, flags);
}

////////////////////////////////////////////
// regex_replace

template <class OutputIt, class BidirIt, class CharT, class Traits>
OutputIt regex_replace(
	OutputIt out,
	BidirIt first, BidirIt last,
	const basic_regex<CharT, Traits>& e,
	const basic_string<CharT>& fmt,
	regex_constants::match_flag_type flags = regex_constants::match_default);

template <class OutputIt, class BidirIt, class CharT, class Traits>
OutputIt regex_replace(
	OutputIt out,
	BidirIt first, BidirIt last,
	const basic_regex<CharT, Traits>& e,
	const CharT* fmt,
	regex_constants::match_flag_type flags = regex_constants::match_default);

// Overload taking std::string
template <class CharT, class Traits>
inline basic_string<CharT> regex_replace(
	const basic_string<CharT>& s,
	const basic_regex<CharT, Traits>& e,
	const basic_string<CharT>& fmt,
	regex_constants::match_flag_type flags = regex_constants::match_default)
{
	basic_string<CharT> result;
	regex_replace(std::back_inserter(result), s.begin(), s.end(), e, fmt, flags);
	return result;
}

// Overload taking std::string
template <class CharT, class Traits>
inline basic_string<CharT> regex_replace(
	const basic_string<CharT>& s,
	const basic_regex<CharT, Traits>& e,
	const CharT* fmt,
	regex_constants::match_flag_type flags = regex_constants::match_default)
{
	basic_string<CharT> result;
	regex_replace(std::back_inserter(result), s.begin(), s.end(), e, fmt, flags);
	return result;
}

// Overload taking C-string input and basic_string format
template <class CharT, class Traits>
inline basic_string<CharT> regex_replace(
	const CharT* s,
	const basic_regex<CharT, Traits>& e,
	const basic_string<CharT>& fmt,
	regex_constants::match_flag_type flags = regex_constants::match_default)
{
	return regex_replace(basic_string<CharT>(s), e, fmt, flags);
}

// Overload taking C-string input and C-string format
template <class CharT, class Traits>
inline basic_string<CharT> regex_replace(
	const CharT* s,
	const basic_regex<CharT, Traits>& e,
	const CharT* fmt,
	regex_constants::match_flag_type flags = regex_constants::match_default)
{
	return regex_replace(basic_string<CharT>(s), e, basic_string<CharT>(fmt), flags);
}

////////////////////////////////////////////
// regex_search

template <class BidirIt, class Alloc, class CharT, class Traits>
bool regex_search(
	BidirIt first, BidirIt last,
	match_results<BidirIt, Alloc>& m,
	const basic_regex<CharT, Traits>& e,
	regex_constants::match_flag_type flags = regex_constants::match_default);

// C-string overload
template <class CharT, class Alloc, class Traits>
inline bool regex_search(
	const CharT* str,
	match_results<const CharT*, Alloc>& m,
	const basic_regex<CharT, Traits>& e,
	regex_constants::match_flag_type flags = regex_constants::match_default)
{
	const CharT* end = str + Traits::length(str);
	return regex_search(str, end, m, e, flags);
}

// std::string overload
template <class Alloc, class CharT, class Traits>
inline bool regex_search(
	const basic_string<CharT>& s,
	match_results<typename basic_string<CharT>::const_iterator, Alloc>& m,
	const basic_regex<CharT, Traits>& e,
	regex_constants::match_flag_type flags = regex_constants::match_default)
{
	return regex_search(s.begin(), s.end(), m, e, flags);
}

// Version without match_results (simply checks for a match)
template <class BidirIt, class CharT, class Traits>
inline bool regex_search(
	BidirIt first, BidirIt last,
	const basic_regex<CharT, Traits>& e,
	regex_constants::match_flag_type flags = regex_constants::match_default)
{
	match_results<BidirIt> m;
	return regex_search(first, last, m, e, flags);
}

} // namespace onigpp
