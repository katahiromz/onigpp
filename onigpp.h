// onigpp.h --- Oniguruma++ (onigpp) regular expression engine
// Author: katahiromz
// License: MIT
#pragma once

#include <string>
#include <vector>
#include <exception>
#include <stdexcept>
#include <algorithm>
#include <cstring>
#include <cassert>

// Oniguruma
#define ONIG_ESCAPE_UCHAR_COLLISION
#define ONIG_ESCAPE_REGEX_T_COLLISION
#include "oniguruma/src/oniguruma.h"

namespace onigpp {

////////////////////////////////////////////
// onigpp::size_t

using size_t = std::size_t;
using size_type = size_t;

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
	typedef int error_type;

	// Syntax options
	typedef unsigned long syntax_option_type;
	inline constexpr syntax_option_type icase = (1 << 0);
	inline constexpr syntax_option_type multiline = (1 << 1);
	inline constexpr syntax_option_type extended = (1 << 2);

	// Search/Match control flags
	typedef unsigned long match_flag_type;

	// Directly mappable to Oniguruma API
	inline constexpr match_flag_type match_not_bol = (1 << 3); // ONIG_OPTION_NOTBOL
	inline constexpr match_flag_type match_not_eol = (1 << 4); // ONIG_OPTION_NOTEOL
	inline constexpr match_flag_type match_any = (1 << 5);

	// Flags processed by logic (as there are no direct Oniguruma options)
	inline constexpr match_flag_type match_not_null = (1 << 6); // Disallow zero-width match (treat length 0 match as failure)
	inline constexpr match_flag_type match_prev_avail = (1 << 7); // For \b, \B, etc. determination

	// Format control flags for replacement
	inline constexpr match_flag_type format_first_only = (1 << 8);
	inline constexpr match_flag_type format_no_copy = (1 << 9);
	inline constexpr match_flag_type format_literal = (1 << 10);

	// Default values
	inline constexpr syntax_option_type normal = 0;
	inline constexpr match_flag_type format_default = 0;
	inline constexpr match_flag_type match_default = 0;
}

////////////////////////////////////////////
// onigpp::encoding_constants

namespace encoding_constants {
	// Expose pointers to encodings provided by Oniguruma
	inline OnigEncoding ASCII  = ONIG_ENCODING_ASCII;
	inline OnigEncoding UTF8 = ONIG_ENCODING_UTF8;
	inline OnigEncoding UTF16LE = ONIG_ENCODING_UTF16_LE;
	inline OnigEncoding UTF16BE = ONIG_ENCODING_UTF16_BE;
	inline OnigEncoding UTF32LE = ONIG_ENCODING_UTF32_LE;
	inline OnigEncoding UTF32BE = ONIG_ENCODING_UTF32_BE;
	inline OnigEncoding LATIN1 = ONIG_ENCODING_ISO_8859_1;
	inline OnigEncoding SHIFT_JIS = ONIG_ENCODING_SJIS;
	inline OnigEncoding EUC_JP = ONIG_ENCODING_EUC_JP;
	// Other encodings can be added as needed
}

////////////////////////////////////////////
// onigpp::regex_error

class regex_error : public std::exception {
public:
	regex_error(regex_constants::error_type ecode, const OnigErrorInfo& err_info);
	virtual ~regex_error();
	regex_constants::error_type code() const;
	const char* what() const noexcept override;

protected:
	regex_constants::error_type m_err_code;
	OnigErrorInfo m_err_info;
};

////////////////////////////////////////////
// onigpp::sub_match<BidirIt>

template <class BidirIt>
class sub_match : public std::pair<BidirIt, BidirIt> {
public:
	typedef BidirIt iterator;
	typedef typename std::iterator_traits<BidirIt>::value_type value_type;
	typedef typename std::iterator_traits<BidirIt>::difference_type difference_type;
	typedef basic_string<value_type> string_type;

	bool matched;

	// Default constructor
	sub_match() : std::pair<BidirIt, BidirIt>(), matched(false) {}
	
	// Added 3-argument constructor
	sub_match(BidirIt first, BidirIt second, bool is_matched) 
		: std::pair<BidirIt, BidirIt>(first, second), matched(is_matched) {}

	string_type str() const { return string_type(this->first, this->second); }
};

typedef sub_match<const char*> csub_match;
typedef sub_match<const wchar_t*> wcsub_match;
typedef sub_match<const char16_t*> u16csub_match;
typedef sub_match<const char32_t*> u32csub_match;
typedef sub_match<string::const_iterator> ssub_match;
typedef sub_match<wstring::const_iterator> wssub_match;
typedef sub_match<u16string::const_iterator> u16ssub_match;
typedef sub_match<u32string::const_iterator> u32ssub_match;

////////////////////////////////////////////
// onigpp::regex_traits<CharT>

template <class CharT>
class regex_traits {
public:
	typedef CharT char_type;
	typedef basic_string<CharT> string_type;
	static size_type length(const char_type* s) {
		return std::char_traits<char_type>::length(s);
	}
};

////////////////////////////////////////////
// onigpp::match_results<BidirIt, Allocator>

template <class BidirIt, class Alloc = std::allocator<sub_match<BidirIt>>>
class match_results : public std::vector<sub_match<BidirIt>, Alloc> {
public:
	typedef sub_match<BidirIt> value_type;
	typedef const value_type& const_reference;
	typedef value_type& reference;
	typedef typename std::vector<sub_match<BidirIt>, Alloc>::const_iterator const_iterator;
	typedef typename std::vector<sub_match<BidirIt>, Alloc>::iterator iterator;
	typedef typename std::iterator_traits<BidirIt>::difference_type difference_type;
	typedef typename std::allocator_traits<Alloc>::size_type size_type;
	typedef typename std::iterator_traits<BidirIt>::value_type char_type;
	typedef basic_string<char_type> string_type;

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
	typedef CharT value_type;
	typedef Traits traits_type;
	typedef typename Traits::string_type string_type;
	typedef regex_constants::syntax_option_type flag_type;
	typedef basic_regex<CharT,Traits> self_type;
	typedef int locale_type;

public:
	basic_regex() : m_regex(nullptr), m_encoding(nullptr), m_flags(regex_constants::normal) { }
	explicit basic_regex(const CharT* s, flag_type f = regex_constants::normal, OnigEncoding enc = nullptr)
		: basic_regex(s, Traits::length(s), f, enc) { }
	basic_regex(const CharT* s, size_t count, flag_type f = regex_constants::normal, OnigEncoding enc = nullptr);
	basic_regex(const self_type& other);
	basic_regex(self_type&& other) noexcept;
	virtual ~basic_regex();

	self_type& operator=(const self_type& other) {
		self_type tmp(other);
		swap(tmp);
		return *this;
	}
	self_type& operator=(self_type&& other) noexcept { swap(other); return *this; }
	self_type& operator=(const CharT* ptr) { return assign(ptr); }

	self_type& assign(const self_type& other) {
		self_type tmp(other);
		swap(tmp);
		return *this;
	}
	self_type& assign(self_type&& other) noexcept {
		swap(std::move(other));
		return *this;
	}
	self_type& assign(const CharT* ptr, flag_type f = regex_constants::normal, OnigEncoding enc = nullptr) {
		if (!enc) enc = m_encoding;
		self_type tmp(ptr, f, enc);
		swap(std::move(tmp));
		return *this;
	}
	self_type& assign(const string_type& str, flag_type f = regex_constants::normal, OnigEncoding enc = nullptr) {
		if (!enc) enc = m_encoding;
		self_type tmp(str.c_str(), str.length(), f, enc);
		swap(tmp);
		return *this;
	}

	unsigned mark_count() const;
	flag_type flags() const { return m_flags; }

	void swap(self_type& other) noexcept {
		std::swap(m_regex, other.m_regex);
		std::swap(m_encoding, other.m_encoding);
		std::swap(m_flags, other.m_flags);
		std::swap(m_pattern, other.m_pattern);
	}

	template <class, class> friend struct regex_access;

	locale_type getloc() { return 0; }
	locale_type imbue(locale_type loc) { return loc; }

protected:
	OnigRegex m_regex;
	OnigEncoding m_encoding;
	flag_type m_flags;
	string_type m_pattern;

	static OnigOptionType _options_from_flags(flag_type f);
};

////////////////////////////////////////////
// onigpp::regex, onigpp::wregex

typedef basic_regex<char> regex;
typedef basic_regex<wchar_t> wregex;
typedef basic_regex<char16_t> u16regex;
typedef basic_regex<char32_t> u32regex;

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
	size_t m_subs_idx;

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
const sub_match<BidirIt> match_results<BidirIt, Alloc>::prefix() const {
	if (this->empty()) {
		return sub_match<BidirIt>(m_str_begin, m_str_begin, false);
	}
	return sub_match<BidirIt>(m_str_begin, (*this)[0].first, true);
}

template <class BidirIt, class Alloc>
const sub_match<BidirIt> match_results<BidirIt, Alloc>::suffix() const {
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
bool regex_match(
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
bool regex_match(
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

// Overload taking std::string
template <class CharT, class Traits>
basic_string<CharT> regex_replace(
	const basic_string<CharT>& s,
	const basic_regex<CharT, Traits>& e,
	const basic_string<CharT>& fmt,
	regex_constants::match_flag_type flags = regex_constants::match_default)
{
	basic_string<CharT> result;
	regex_replace(std::back_inserter(result), s.begin(), s.end(), e, fmt, flags);
	return result;
}

// Overload taking C-string
template <class CharT, class Traits>
basic_string<CharT> regex_replace(
	const CharT* s,
	const basic_regex<CharT, Traits>& e,
	const basic_string<CharT>& fmt,
	regex_constants::match_flag_type flags = regex_constants::match_default)
{
	return regex_replace(basic_string<CharT>(s), e, fmt, flags);
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
bool regex_search(
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
bool regex_search(
	const basic_string<CharT>& s,
	match_results<typename basic_string<CharT>::const_iterator, Alloc>& m,
	const basic_regex<CharT, Traits>& e,
	regex_constants::match_flag_type flags = regex_constants::match_default)
{
	return regex_search(s.begin(), s.end(), m, e, flags);
}

// Version without match_results (simply checks for a match)
template <class BidirIt, class CharT, class Traits>
bool regex_search(
	BidirIt first, BidirIt last,
	const basic_regex<CharT, Traits>& e,
	regex_constants::match_flag_type flags = regex_constants::match_default)
{
	match_results<BidirIt> m;
	return regex_search(first, last, m, e, flags);
}

////////////////////////////////////////////
// onigpp::init

inline int init(const OnigEncoding *encodings = nullptr, size_t encodings_count = 0) {
	static OnigEncoding use_encodings[] = {
#define SUPPORTED_ENCODING(enc) enc,
#include "encodings.h"
#undef SUPPORTED_ENCODING
	};
	if (!encodings) {
		encodings = use_encodings;
		encodings_count = sizeof(use_encodings) / sizeof(use_encodings[0]);
	}
	int err = onig_initialize((OnigEncoding *)encodings, (int)encodings_count);
	if (err != ONIG_NORMAL) {
		throw std::runtime_error("onig_initialize failed");
	}
	return err;
}

////////////////////////////////////////////
// onigpp::uninit

inline void uninit() { onig_end(); }

////////////////////////////////////////////
// onigpp::version

inline const char* version() { return onig_version(); }

////////////////////////////////////////////
// Implementation helpers

// Helper to access protected members of basic_regex
template <class CharT, class Traits>
struct regex_access : public basic_regex<CharT, Traits> {
	static OnigRegex get(const basic_regex<CharT, Traits>& re) {
		return static_cast<const regex_access<CharT, Traits>&>(re).m_regex;
	}
	static OnigEncoding get_encoding(const basic_regex<CharT, Traits>& re) {
		return static_cast<const regex_access<CharT, Traits>&>(re).m_encoding;
	}
};

// Helper function to expand capture groups ($1, $2, ... or \1, \2, ...) in the replacement string
template <class CharT>
void _append_replacement(
	basic_string<CharT>& result,
	const basic_string<CharT>& fmt,
	const match_results<const CharT*>& m,
	const basic_regex<CharT>& re)
{
	using string_t = basic_string<CharT>;

	const size_t len = fmt.size();
	size_t i = 0;

	while (i < len) {
		CharT c = fmt[i];

		// --- Escape sequence: starts with '$'
		if (c == CharT('$')) {
			i++;
			if (i >= len) { result += CharT('$'); break; }

			CharT n = fmt[i];

			//--------------------------
			// "$$" → "$"
			//--------------------------
			if (n == CharT('$')) {
				result += CharT('$');
				i++;
				continue;
			}

			//--------------------------
			// "$&" → whole match
			//--------------------------
			if (n == CharT('&')) {
				if (m.size() > 0 && m[0].matched)
					result.append(m[0].first, m[0].second);
				i++;
				continue;
			}

			//--------------------------
			// "$`" → prefix
			//--------------------------
			if (n == CharT('`')) {
				auto pre = m.prefix();
				if (pre.matched)
					result.append(pre.first, pre.second);
				i++;
				continue;
			}

			//--------------------------
			// "$'" → suffix
			//--------------------------
			if (n == CharT('\'')) {
				auto suf = m.suffix();
				if (suf.matched)
					result.append(suf.first, suf.second);
				i++;
				continue;
			}

			//--------------------------
			// "$+" → last captured group
			//--------------------------
			if (n == CharT('+')) {
				int last = -1;
				for (size_t gi = 1; gi < m.size(); gi++) {
					if (m[gi].matched)
						last = (int)gi;
				}
				if (last >= 0)
					result.append(m[last].first, m[last].second);
				i++;
				continue;
			}

			//--------------------------
			// "${name}" → named capture
			//--------------------------
			if (n == CharT('{')) {
				size_t start = ++i;
				while (i < len && fmt[i] != CharT('}')) i++;

				if (i >= len) {
					// missing '}' → treat literally
					result += CharT('$');
					result += CharT('{');
					continue;
				}

				string_t name = fmt.substr(start, i - start);
				i++; // skip '}'

				// Find named group
				// Convert character pointers to byte pointers for Oniguruma API
				const OnigUChar *name_start = reinterpret_cast<const OnigUChar*>(name.c_str());
				const OnigUChar *name_end = reinterpret_cast<const OnigUChar*>(name.c_str() + name.size());
				int num = onig_name_to_backref_number(
					regex_access<CharT, regex_traits<CharT>>::get(re),
					name_start,
					name_end,
					nullptr
				);
				if (num > 0 && (size_t)num < m.size() && m[num].matched) {
					result.append(m[num].first, m[num].second);
				}
				continue;
			}

			//--------------------------
			// "$n" → numeric capture
			//--------------------------
			if (n >= CharT('0') && n <= CharT('9')) {
				size_t num = 0;
				while (i < len && fmt[i] >= CharT('0') && fmt[i] <= CharT('9')) {
					num = num * 10 + (fmt[i] - CharT('0'));
					i++;
				}
				if (num < m.size() && m[num].matched)
					result.append(m[num].first, m[num].second);
				continue;
			}

			//--------------------------
			// Unknown pattern → literal "$x"
			//--------------------------
			result += CharT('$');
			result += n;
			i++;
			continue;
		}

		// normal character
		result += c;
		i++;
	}
}

template <class CharT>
OnigEncoding _get_default_encoding_from_char_type() {
	if constexpr (std::is_same_v<CharT, char>) {
		return ONIG_ENCODING_UTF8; // Default is UTF-8
	} else if constexpr (std::is_same_v<CharT, wchar_t>) {
		// Use UTF-16 or UTF-32 depending on wchar_t size and endianness
		if constexpr (sizeof(wchar_t) == 2) {
			#if defined(_WIN32) || defined(__LITTLE_ENDIAN__) || \
				(defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
			return ONIG_ENCODING_UTF16_LE;
			#else
			return ONIG_ENCODING_UTF16_BE;
			#endif
		} else if constexpr (sizeof(wchar_t) == 4) {
			#if defined(_WIN32) || defined(__LITTLE_ENDIAN__) || \
				(defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
			return ONIG_ENCODING_UTF32_LE;
			#else
			return ONIG_ENCODING_UTF32_BE;
			#endif
		}
	} else if constexpr (std::is_same_v<CharT, char16_t>) {
		#if defined(_WIN32) || defined(__LITTLE_ENDIAN__) || \
			(defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
		return ONIG_ENCODING_UTF16_LE;
		#else
		return ONIG_ENCODING_UTF16_BE;
		#endif
	} else if constexpr (std::is_same_v<CharT, char32_t>) {
		#if defined(_WIN32) || defined(__LITTLE_ENDIAN__) || \
			(defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
		return ONIG_ENCODING_UTF32_LE;
		#else
		return ONIG_ENCODING_UTF32_BE;
		#endif
	}
	// Return UTF8 to avoid compilation errors if no suitable default
	return ONIG_ENCODING_UTF8;
}

////////////////////////////////////////////
// Implementation of regex_error

inline regex_error::regex_error(regex_constants::error_type ecode, const OnigErrorInfo& err_info)
	: m_err_code(ecode), m_err_info(err_info)
{
}

inline regex_error::~regex_error() { }

inline regex_constants::error_type regex_error::code() const {
	return m_err_code;
}

inline const char* regex_error::what() const noexcept {
	static thread_local char err_buf[ONIG_MAX_ERROR_MESSAGE_LEN];
	if (onig_is_error_code_needs_param(m_err_code)) {
		onig_error_code_to_str((OnigUChar*)err_buf, m_err_code, &m_err_info);
	} else {
		onig_error_code_to_str((OnigUChar*)err_buf, m_err_code);
	}
	return err_buf;
}

////////////////////////////////////////////
// Implementation of basic_regex

template <class CharT, class Traits>
OnigOptionType basic_regex<CharT, Traits>::_options_from_flags(flag_type f) {
	bool icase = (f & regex_constants::icase);
	bool multiline = (f & regex_constants::multiline);
	bool extended = (f & regex_constants::extended);

	OnigOptionType options = 0;
	options |= (icase ? ONIG_OPTION_IGNORECASE : 0);
	options |= (multiline ? (ONIG_OPTION_MULTILINE | ONIG_OPTION_NEGATE_SINGLELINE) : ONIG_OPTION_SINGLELINE);
	options |= (extended ? ONIG_OPTION_EXTEND : 0);
	return options;
}

template <class CharT, class Traits>
basic_regex<CharT, Traits>::basic_regex(self_type&& other) noexcept 
	: m_regex(other.m_regex), m_encoding(other.m_encoding), m_flags(other.m_flags), m_pattern(std::move(other.m_pattern)) 
{
	other.m_regex = nullptr;
	other.m_encoding = nullptr;
}

template <class CharT, class Traits>
basic_regex<CharT, Traits>::basic_regex(const CharT* s, size_t count, flag_type f, OnigEncoding enc) 
	: m_regex(nullptr), m_encoding(nullptr), m_flags(f), m_pattern(s, count) 
{
	OnigSyntaxType* syntax = ONIG_SYNTAX_ONIGURUMA;
	OnigOptionType options = _options_from_flags(f);
	OnigErrorInfo err_info;
	if (!enc) enc = _get_default_encoding_from_char_type<CharT>();
	m_encoding = enc;
	int err = onig_new(&m_regex, reinterpret_cast<const OnigUChar*>(s), reinterpret_cast<const OnigUChar*>(s + count), options, enc, syntax, &err_info);
	if (err != ONIG_NORMAL) throw regex_error(err, err_info);
}

template <class CharT, class Traits>
basic_regex<CharT, Traits>::basic_regex(const self_type& other) 
	: m_regex(nullptr), m_encoding(other.m_encoding), m_flags(other.m_flags), m_pattern(other.m_pattern)
{
	if (!other.m_regex) return; // If the original object is invalid

	OnigSyntaxType* syntax = ONIG_SYNTAX_ONIGURUMA;
	OnigOptionType options = _options_from_flags(m_flags);
	OnigErrorInfo err_info;

	const CharT* s = m_pattern.c_str();
	size_t count = m_pattern.length();

	int err = onig_new(&m_regex, reinterpret_cast<const OnigUChar*>(s), reinterpret_cast<const OnigUChar*>(s + count), 
					   options, m_encoding, syntax, &err_info);
	if (err != ONIG_NORMAL) throw regex_error(err, err_info);
}

template <class CharT, class Traits>
inline basic_regex<CharT, Traits>::~basic_regex() {
	if (m_regex) {
		onig_free(m_regex);
	}
}

template <class CharT, class Traits>
unsigned basic_regex<CharT, Traits>::mark_count() const {
	if (!m_regex) return 0;
	return onig_number_of_captures(m_regex);
}

////////////////////////////////////////////
// regex_search implementation

template <class BidirIt, class Alloc, class CharT, class Traits>
bool regex_search(
	BidirIt first, BidirIt last,
	match_results<BidirIt, Alloc>& m,
	const basic_regex<CharT, Traits>& e,
	regex_constants::match_flag_type flags)
{
	// Get Oniguruma regex object (using accessor hack)
	OnigRegex reg = regex_access<CharT, Traits>::get(e);
	if (!reg) return false;

	// Options before search execution
	OnigOptionType onig_options = 0;
	// Extract Oniguruma options from match_flag_type
	if (flags & regex_constants::match_not_bol) onig_options |= ONIG_OPTION_NOTBOL;
	if (flags & regex_constants::match_not_eol) onig_options |= ONIG_OPTION_NOTEOL;

	// Iterator distance (number of characters)
	size_t len = std::distance(first, last);

	// Get pointer to the search target
	// Use stable static buffer for empty ranges to avoid passing nullptr to C API
	static CharT empty_char = CharT();
	const CharT* start_ptr = (len > 0) ? &(*first) : &empty_char;
	const CharT* end_ptr = start_ptr + len;

	// Cast to Oniguruma pointers
	const OnigUChar* u_start = reinterpret_cast<const OnigUChar*>(start_ptr);
	const OnigUChar* u_end   = reinterpret_cast<const OnigUChar*>(end_ptr);
	const OnigUChar* u_range = u_end; // Forward search range

	// Allocate OnigRegion
	OnigRegion* region = onig_region_new();
	if (!region) throw std::bad_alloc();

	// Execute search
	int r = onig_search(reg, u_start, u_end, u_start, u_range, region, onig_options);

	if (r >= 0) {
		if (flags & regex_constants::match_not_null) {
			// If match length is zero, treat it as a match failure
			if (region->beg[0] == region->end[0]) {
				onig_region_free(region, 1);
				return false; // Equivalent to ONIG_MISMATCH
			}
		}

		// If matched, store results in match_results
		m.m_str_begin = first;
		m.m_str_end = last;
		m.clear();
		m.resize(region->num_regs);

		for (int i = 0; i < region->num_regs; ++i) {
			int beg = region->beg[i];
			int end = region->end[i];

			if (beg != ONIG_REGION_NOTPOS) {
				// Oniguruma returns byte offsets, convert to character offsets
				int beg_chars = beg / sizeof(CharT);
				int end_chars = end / sizeof(CharT);

				// Advance iterators
				BidirIt sub_start = first;
				std::advance(sub_start, beg_chars);

				BidirIt sub_end = first;
				std::advance(sub_end, end_chars);

				m[i].first = sub_start;
				m[i].second = sub_end;
				m[i].matched = true;
			} else {
				// Unmatched group
				m[i].first = last;
				m[i].second = last;
				m[i].matched = false;
			}
		}

		onig_region_free(region, 1); // 1: Free the region itself too
		return true;
	}
	else if (r == ONIG_MISMATCH) {
		onig_region_free(region, 1);
		return false;
	}
	else {
		// On error
		onig_region_free(region, 1);
		OnigErrorInfo einfo;
		std::memset(&einfo, 0, sizeof(einfo));
		throw regex_error(r, einfo);
	}
}

////////////////////////////////////////////
// regex_match implementation

template <class BidirIt, class Alloc, class CharT, class Traits>
bool regex_match(
	BidirIt first, BidirIt last,
	match_results<BidirIt, Alloc>& m,
	const basic_regex<CharT, Traits>& e,
	regex_constants::match_flag_type flags)
{
	// Get Oniguruma regex object (using accessor hack)
	OnigRegex reg = regex_access<CharT, Traits>::get(e);
	if (!reg) return false;

	// Options before search execution
	OnigOptionType onig_options = 0;
	// Extract Oniguruma options from match_flag_type
	if (flags & regex_constants::match_not_bol) onig_options |= ONIG_OPTION_NOTBOL;
	if (flags & regex_constants::match_not_eol) onig_options |= ONIG_OPTION_NOTEOL;

	// Iterator distance (number of characters)
	size_t len = std::distance(first, last);

	// Get pointer to the search target
	// Use stable static buffer for empty ranges to avoid passing nullptr to C API
	static CharT empty_char = CharT();
	const CharT* start_ptr = (len > 0) ? &(*first) : &empty_char;
	const CharT* end_ptr = start_ptr + len;

	// Cast to Oniguruma pointers
	const OnigUChar* u_start = reinterpret_cast<const OnigUChar*>(start_ptr);
	const OnigUChar* u_end   = reinterpret_cast<const OnigUChar*>(end_ptr);

	// Allocate OnigRegion
	OnigRegion* region = onig_region_new();
	if (!region) {
		throw std::bad_alloc();
	}

	// Execute match
	int r = onig_match(reg, u_start, u_end, u_start, region, onig_options);

	if (r >= 0) {
		// regex_match requires full match with the entire string
		// Check if the match end position matches the string end
		// region->end[0] is in bytes, so convert to characters for comparison
		if (region->end[0] != (int)(len * sizeof(CharT))) {
			onig_region_free(region, 1);
			return false;
		}

		if (flags & regex_constants::match_not_null) {
			// If match length is zero, treat it as a match failure
			if (region->beg[0] == region->end[0]) {
				onig_region_free(region, 1);
				return false;
			}
		}

		// If matched, store results in match_results
		m.m_str_begin = first;
		m.m_str_end = last;
		m.clear();
		m.resize(region->num_regs);

		for (int i = 0; i < region->num_regs; ++i) {
			int beg = region->beg[i];
			int end = region->end[i];

			if (beg != ONIG_REGION_NOTPOS) {
				// Oniguruma returns byte offsets, convert to character offsets
				int beg_chars = beg / sizeof(CharT);
				int end_chars = end / sizeof(CharT);

				BidirIt sub_start = first;
				std::advance(sub_start, beg_chars);

				BidirIt sub_end = first;
				std::advance(sub_end, end_chars);

				m[i].first = sub_start;
				m[i].second = sub_end;
				m[i].matched = true;
			} else {
				m[i].first = last;
				m[i].second = last;
				m[i].matched = false;
			}
		}

		onig_region_free(region, 1);
		return true;
	}
	else if (r == ONIG_MISMATCH) {
		onig_region_free(region, 1);
		return false;
	}
	else {
		// On error
		onig_region_free(region, 1);
		OnigErrorInfo einfo;
		std::memset(&einfo, 0, sizeof(einfo));
		throw regex_error(r, einfo);
	}
}

////////////////////////////////////////////
// regex_replace implementation

template <class OutputIt, class BidirIt, class CharT, class Traits>
OutputIt regex_replace(
	OutputIt out,
	BidirIt first, BidirIt last,
	const basic_regex<CharT, Traits>& e,
	const basic_string<CharT>& fmt,
	regex_constants::match_flag_type flags)
{
	// Get Oniguruma regex object
	OnigRegex reg = regex_access<CharT, Traits>::get(e);
	if (!reg) {
		// If regex object is invalid, output input as is
		for (auto it = first; it != last; ++it) {
			*out++ = *it;
		}
		return out;
	}

	// Get encoding
	OnigEncoding enc = regex_access<CharT, Traits>::get_encoding(e);

	// Search options
	OnigOptionType onig_options = ONIG_OPTION_NONE;

	// Allocate OnigRegion for repeated searches
	OnigRegion* region = onig_region_new();
	if (!region) {
		throw std::bad_alloc();
	}

	// Input data pointers
	size_t len = std::distance(first, last);
	// Use stable static buffer for empty ranges to avoid passing nullptr to C API
	static CharT empty_char = CharT();
	const CharT* start_ptr = (len > 0) ? &(*first) : &empty_char;
	const CharT* end_ptr = start_ptr + len;

	const OnigUChar* u_start = reinterpret_cast<const OnigUChar*>(start_ptr);
	const OnigUChar* u_end   = reinterpret_cast<const OnigUChar*>(end_ptr);

	// last_output: End position already output (initially the beginning)
	const OnigUChar* u_last_output = u_start;
	// search_pos: Position to start the next onig_search from
	const OnigUChar* u_search = u_start;

	bool global_replace = !(flags & regex_constants::format_first_only);

	while (u_search <= u_end) {
		// Execute search (using u_search as the starting position)
		int r = onig_search(reg, u_start, u_end, u_search, u_end, region, onig_options);

		if (r >= 0) {
			int match_beg = region->beg[0];
			int match_end = region->end[0];
			const OnigUChar* match_start = u_start + match_beg;
			const OnigUChar* match_finish = u_start + match_end;

			// 1) Output the content from last_output up to match_start (prefix)
			// Iterate by character, not by byte
			for (const OnigUChar* p = u_last_output; p < match_start; p += sizeof(CharT)) {
				*out++ = *reinterpret_cast<const CharT*>(p);
			}

			// 2) Convert to match_results and expand replacement
			match_results<const CharT*> m;
			m.m_str_begin = start_ptr;
			m.m_str_end = end_ptr;

			m.resize(region->num_regs);

			for (int i = 0; i < region->num_regs; ++i) {
				int beg = region->beg[i];
				int end = region->end[i];
				if (beg != ONIG_REGION_NOTPOS) {
					// Oniguruma returns byte offsets, convert to character offsets
					int beg_chars = beg / sizeof(CharT);
					int end_chars = end / sizeof(CharT);
					m[i].first = start_ptr + beg_chars;
					m[i].second = start_ptr + end_chars;
					m[i].matched = true;
				} else {
					m[i].first = end_ptr;
					m[i].second = end_ptr;
					m[i].matched = false;
				}
			}

			basic_string<CharT> replacement_str;
			_append_replacement(replacement_str, fmt, m, e);
			for (CharT c : replacement_str) *out++ = c;

			// 3) Determine next position
			// The end of output is the end of the match (normal match)
			// For zero-width match, the end of output remains match_start (no change)
			if (match_beg != match_end) {
				// For normal match, move the end of output to the match end
				u_last_output = match_finish;
				// Next search starts from the match end position
				u_search = match_finish;
			} else {
				// Zero-width match: End of output remains the same (match_start)
				u_last_output = match_start;
				// Next search start position is advanced by 1 character (considering encoding)
				if (match_start < u_end) {
					int char_len = enc ? onig_enc_len(enc, match_start, u_end) : 1;
					if (char_len < 1) char_len = 1;
					u_search = match_start + char_len;
				} else {
					// If at the end of the string, terminate
					u_search = u_end;
					// set u_last_output = u_end in case we need to copy suffix
					u_last_output = u_end;
					break;
				}
			}

			if (!global_replace) {
				// If not global replace, output the rest and terminate
				break;
			}
		}
		else if (r == ONIG_MISMATCH) {
			// No match found -> Output the rest and terminate
			break;
		}
		else {
			// Error
			onig_region_free(region, 1);
			OnigErrorInfo einfo;
			std::memset(&einfo, 0, sizeof(einfo));
			throw regex_error(r, einfo);
		}
	}

	// 5) Output the remainder (u_last_output .. u_end)
	// Iterate by character, not by byte
	for (const OnigUChar* p = u_last_output; p < u_end; p += sizeof(CharT)) {
		*out++ = *reinterpret_cast<const CharT*>(p);
	}

	onig_region_free(region, 1);
	return out;
}

////////////////////////////////////////////
// Implementation of regex_iterator

template <class BidirIt, class CharT, class Traits>
void regex_iterator<BidirIt, CharT, Traits>::do_search(BidirIt first, BidirIt last) {
	// If no match found, or end iterator reached
	if (first == last || !regex_search(first, last, m_results, *m_regex, m_flags)) {
		// Invalidate as end iterator
		m_regex = nullptr;
		m_results.clear();
	}
}

template <class BidirIt, class CharT, class Traits>
regex_iterator<BidirIt, CharT, Traits>::regex_iterator(
	BidirIt first, BidirIt last,
	const regex_type& re,
	match_flag_type flags)
	: m_end(last), m_regex(&re), m_flags(flags)
{
	// Execute the first search
	do_search(first, last);
}

template <class BidirIt, class CharT, class Traits>
bool regex_iterator<BidirIt, CharT, Traits>::operator==(
	const regex_iterator& other) const
{
	// Check for end iterator
	if (m_regex == nullptr && other.m_regex == nullptr) return true;
	if (m_regex == nullptr || other.m_regex == nullptr) return false;

	// Normal iterator comparison
	if (m_results.empty() || other.m_results.empty()) return false;
	
	return m_results[0].first == other.m_results[0].first &&
		   m_results[0].second == other.m_results[0].second;
}

template <class BidirIt, class CharT, class Traits>
regex_iterator<BidirIt, CharT, Traits>& regex_iterator<BidirIt, CharT, Traits>::operator++() {
	if (m_regex == nullptr || m_results.empty()) {
		return *this;
	}

	// Get the end of the current search result
	BidirIt current_match_end = m_results[0].second;

	// ★ Zero-width match handling ★
	if (m_results[0].first == current_match_end) {
		if (current_match_end != m_end) {
			std::advance(current_match_end, 1); // Advance by 1 character
		} else {
			// Reached the end of the string
			m_regex = nullptr;
			m_results.clear();
			return *this;
		}
	}

	// Execute the next search
	do_search(current_match_end, m_end);

	return *this;
}

template <class BidirIt, class CharT, class Traits>
regex_iterator<BidirIt, CharT, Traits> regex_iterator<BidirIt, CharT, Traits>::operator++(int) {
	regex_iterator tmp = *this;
	++(*this);
	return tmp;
}

////////////////////////////////////////////
// Implementation of regex_token_iterator

template <class BidirIt, class CharT, class Traits>
void regex_token_iterator<BidirIt, CharT, Traits>::_do_increment() {
	// Detect processed mark (after suffix output)
	if (!m_subs.empty() && m_subs[0] == -2) {
		m_subs.clear();
		return;
	}

	// A. Process next sub-index from current match
	if (++m_subs_idx < m_subs.size()) {
		int sub_idx = m_subs[m_subs_idx];

		if (sub_idx == -1) {
			// -1 should have been processed already, skip
			_do_increment();
			return;
		} else {
			// Capture group
			if (sub_idx >= 0 && sub_idx < (int)(*m_itor).size()) {
				m_sub_match = (*m_itor)[sub_idx];
			} else {
				// Invalid index
				m_sub_match = sub_match<BidirIt>();
				m_sub_match.matched = false;
			}
		}
	} else {
		// B. Current match processing complete, move to next match
		if (m_itor == m_end) {
			m_subs.clear();
			return;
		}

		// Save positions before incrementing
		BidirIt prev_match_end = (*m_itor)[0].second;
		++m_itor;

		// C. Check state of next iterator
		if (m_itor == m_end) {
			// No more matches - check if we need to output suffix
			if (std::find(m_subs.begin(), m_subs.end(), -1) != m_subs.end() && 
				!m_subs.empty()) { // Don't process if already cleared
				
				// Output suffix (from end of last match to end of string)
				m_sub_match = sub_match<BidirIt>(prev_match_end, m_last, 
												 prev_match_end != m_last);
				
				// Set m_subs to a special non-empty state (e.g., m_subs = {-2}) 
				// to mark as processed for the next increment to terminate
				m_subs.assign({-2}); // Processed mark
				m_subs_idx = 0;
				return;
			} else {
				m_subs.clear(); // Terminate
				return;
			}
		} else {
			// Next match found
			m_subs_idx = 0;
			int sub_idx = m_subs[m_subs_idx];

			if (sub_idx == -1) {
				// For -1, output text between previous match end and current match start
				m_sub_match = sub_match<BidirIt>(prev_match_end, (*m_itor)[0].first, 
												 prev_match_end != (*m_itor)[0].first);
			} else if (sub_idx >= 0 && sub_idx < (int)(*m_itor).size()) {
				m_sub_match = (*m_itor)[sub_idx];
			} else {
				m_sub_match = sub_match<BidirIt>();
				m_sub_match.matched = false;
			}
		}
	}
}

template <class BidirIt, class CharT, class Traits>
regex_token_iterator<BidirIt, CharT, Traits>::regex_token_iterator(
	BidirIt first, BidirIt last,
	const regex_type& re,
	const std::vector<int>& subs,
	match_flag_type flags) 
	: m_itor(first, last, re, flags), m_end(), m_subs(subs), m_subs_idx(0), m_last(last)
{
	if (m_itor == m_end) {
		// No matches
		if (std::find(m_subs.begin(), m_subs.end(), -1) != m_subs.end()) {
			// Entire string as single token
			m_sub_match = sub_match<BidirIt>(first, last, first != last);
			m_subs.clear(); // Will terminate on next increment
		} else {
			m_subs.clear();
		}
	} else {
		// Match found
		int sub_idx = m_subs[m_subs_idx];

		if (sub_idx == -1) {
			// Output prefix (text before first match)
			m_sub_match = sub_match<BidirIt>(first, (*m_itor)[0].first, first != (*m_itor)[0].first);
		} else if (sub_idx >= 0 && sub_idx < (int)(*m_itor).size()) {
			m_sub_match = (*m_itor)[sub_idx];
		} else {
			m_sub_match = sub_match<BidirIt>();
			m_sub_match.matched = false;
		}
	}
}

template <class BidirIt, class CharT, class Traits>
bool regex_token_iterator<BidirIt, CharT, Traits>::operator==(const regex_token_iterator& other) const {
	// Check for termination
	if (m_subs.empty() && other.m_subs.empty()) return true;
	if (m_subs.empty() || other.m_subs.empty()) return false;

	// Normal comparison
	return m_itor == other.m_itor &&
		   m_sub_match.first == other.m_sub_match.first &&
		   m_sub_match.second == other.m_sub_match.second;
}

template <class BidirIt, class CharT, class Traits>
regex_token_iterator<BidirIt, CharT, Traits>& regex_token_iterator<BidirIt, CharT, Traits>::operator++() {
	_do_increment();
	return *this;
}

template <class BidirIt, class CharT, class Traits>
regex_token_iterator<BidirIt, CharT, Traits> regex_token_iterator<BidirIt, CharT, Traits>::operator++(int) {
	regex_token_iterator tmp = *this;
	++(*this);
	return tmp;
}

} // namespace onigpp
