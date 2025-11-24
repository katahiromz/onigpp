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
#include <limits>

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
	// Error types (std::regex compatibility)
	// These constants match std::regex_constants::error_type values exactly.
	enum error_type {
		error_collate    = 0,   // Same as std::regex_constants::error_collate
		error_ctype      = 1,   // Same as std::regex_constants::error_ctype
		error_escape     = 2,   // Same as std::regex_constants::error_escape
		error_backref    = 3,   // Same as std::regex_constants::error_backref
		error_brack      = 4,   // Same as std::regex_constants::error_brack
		error_paren      = 5,   // Same as std::regex_constants::error_paren
		error_brace      = 6,   // Same as std::regex_constants::error_brace
		error_badbrace   = 7,   // Same as std::regex_constants::error_badbrace
		error_range      = 8,   // Same as std::regex_constants::error_range
		error_space      = 9,   // Same as std::regex_constants::error_space
		error_badrepeat  = 10,  // Same as std::regex_constants::error_badrepeat
		error_complexity = 11,  // Same as std::regex_constants::error_complexity
		error_stack      = 12   // Same as std::regex_constants::error_stack
	};
	
	// Map Oniguruma error codes to onigpp error_type
	inline error_type map_oniguruma_error(int onig_error) {
		// Pattern syntax errors
		if (onig_error == -100) return error_brace;                             // LEFT_BRACE
		if (onig_error == -101) return error_brack;                             // LEFT_BRACKET
		if (onig_error == -102 || onig_error == -103) return error_brack;      // EMPTY_CHAR_CLASS, PREMATURE_END
		if (onig_error == -104 || onig_error == -105 || onig_error == -106) return error_escape;  // END_AT_ESCAPE, META, CONTROL
		if (onig_error == -108 || onig_error == -109) return error_escape;     // META/CONTROL_CODE_SYNTAX
		if (onig_error >= -112 && onig_error <= -110) return error_range;      // CHAR_CLASS range errors
		if (onig_error >= -115 && onig_error <= -113) return error_badrepeat;  // REPEAT_OPERATOR errors
		if (onig_error >= -120 && onig_error <= -116) return error_paren;      // PARENTHESIS/GROUP errors
		if (onig_error >= -135 && onig_error <= -121) return error_badbrace;   // BRACE/QUANTIFIER errors
		if (onig_error >= -138 && onig_error <= -136) return error_backref;    // BACKREF errors
		if (onig_error >= -223 && onig_error <= -139) return error_escape;     // Various syntax errors
		
		// Resource/complexity errors
		if (onig_error == -5) return error_space;                               // MEMORY
		if (onig_error >= -20 && onig_error <= -15) return error_complexity;   // STACK/LIMIT errors
		if (onig_error >= -12 && onig_error <= -11) return error_stack;        // BUG errors
		
		// Encoding/type errors
		if (onig_error >= -403 && onig_error <= -400) return error_ctype;      // ENCODING/CODE_POINT errors
		if (onig_error >= -405 && onig_error <= -404) return error_collate;    // TOO_MANY/TOO_LONG errors
		if (onig_error == -406) return error_complexity;                        // VERY_INEFFICIENT_PATTERN
		
		// Default to error_escape for unknown errors
		return error_escape;
	}

	// Syntax options
	using syntax_option_type = unsigned long;
	static constexpr syntax_option_type icase = (1 << 0);
	static constexpr syntax_option_type multiline = (1 << 1);
	static constexpr syntax_option_type extended = (1 << 2);
	
	// std::regex compatible flags (bits 3-5, avoiding collision with existing bits 0-2 and 11-15)
	static constexpr syntax_option_type nosubs = (1 << 3);   // std::nosubs - don't store submatches in match_results
	static constexpr syntax_option_type optimize = (1 << 4); // std::optimize - currently no-op (reserved for future optimization)
	static constexpr syntax_option_type collate = (1 << 5);  // std::collate - enable locale-dependent collation
	
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

	// Additional std::regex compatibility flags
	// Note: Behavior implementation for these flags will be added in future PRs
	static constexpr match_flag_type match_not_bow = (1 << 11); // std::regex_constants::match_not_bow
	static constexpr match_flag_type match_not_eow = (1 << 12); // std::regex_constants::match_not_eow
	static constexpr match_flag_type match_continuous = (1 << 13); // std::regex_constants::match_continuous

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
	using size_type = onigpp::size_type;

	bool matched;

	// Default constructor
	sub_match() : std::pair<BidirIt, BidirIt>(), matched(false) {}
	
	// Added 3-argument constructor with default is_matched = true
	sub_match(BidirIt first, BidirIt second, bool is_matched = true) 
		: std::pair<BidirIt, BidirIt>(first, second), matched(is_matched) {}
	
	// Templated converting constructor
	template <class OtherBidirIt>
	sub_match(const sub_match<OtherBidirIt>& other)
		: std::pair<BidirIt, BidirIt>(other.first, other.second), matched(other.matched) {}

	// Returns the matched substring as a string, or empty string if unmatched
	// This matches std::sub_match::str() semantics exactly
	string_type str() const { 
		return matched ? string_type(this->first, this->second) : string_type(); 
	}
	
	// Implicit conversion to string_type (preserves str() behavior)
	operator string_type() const { return str(); }
	
	// Returns the length of the matched substring, or 0 if unmatched
	// Note: O(n) for non-random-access iterators (uses std::distance)
	size_type length() const { 
		return matched ? std::distance(this->first, this->second) : 0; 
	}
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
	using locale_type = std::locale;
	using char_class_type = int;
	
	// Constructors
	regex_traits() : m_locale() {}
	explicit regex_traits(const locale_type& loc) : m_locale(loc) {}
	
	static size_type length(const char_type* s) {
		return std::char_traits<char_type>::length(s);
	}
	
	// Get current locale
	locale_type getloc() const {
		return m_locale;
	}
	
	// Set locale and return the previous one
	locale_type imbue(const locale_type& loc) {
		locale_type old = m_locale;
		m_locale = loc;
		return old;
	}
	
	// Transform a character sequence for collation
	string_type transform(const char_type* first, const char_type* last) const {
		// For char and wchar_t, use the locale's collate facet for proper collation
		// For char16_t and char32_t, fall back to simple copy (portable default)
		return transform_impl(first, last, typename std::is_same<char_type, char>::type());
	}
	
	// Translate a character (identity transformation)
	char_type translate(char_type c) const {
		return c;
	}
	
	// Check if character is of a specific character class
	bool isctype(char_type c, char_class_type f) const {
		// Use the locale's ctype facet for char and wchar_t
		// For char16_t and char32_t, provide basic fallback
		return isctype_impl(c, f, typename std::is_same<char_type, char>::type());
	}
	
	// Convert character to numeric value
	int value(char_type c, int base = 10) const {
		// Support bases 2-36 (digits 0-9 and letters a-z/A-Z)
		if (base < 2 || base > 36)
			return -1;
		
		// Try digit 0-9
		if (c >= static_cast<char_type>('0') && c <= static_cast<char_type>('9')) {
			int val = static_cast<int>(c - static_cast<char_type>('0'));
			return (val < base) ? val : -1;
		}
		
		// Try lowercase a-z
		if (c >= static_cast<char_type>('a') && c <= static_cast<char_type>('z')) {
			int val = 10 + static_cast<int>(c - static_cast<char_type>('a'));
			return (val < base) ? val : -1;
		}
		
		// Try uppercase A-Z
		if (c >= static_cast<char_type>('A') && c <= static_cast<char_type>('Z')) {
			int val = 10 + static_cast<int>(c - static_cast<char_type>('A'));
			return (val < base) ? val : -1;
		}
		
		return -1;
	}
	
	// Lookup collation name using the locale's collate facet
	// Returns a locale-specific collation key for the given character sequence,
	// which can be used for locale-aware string comparison and collation.
	// Limitations: If the collate facet is not available for the character type,
	// falls back to returning the input sequence unchanged.
	string_type lookup_collatename(const char_type* first, const char_type* last) const {
		// Attempt to use the locale's collate facet to get a collation key
		// For char and wchar_t, try the collate facet; fallback to simple copy
		try {
			// Check if we have a collate facet for this character type
			if (std::has_facet<std::collate<char_type>>(m_locale)) {
				const std::collate<char_type>& col = std::use_facet<std::collate<char_type>>(m_locale);
				// Return the transformed collation key
				return col.transform(first, last);
			}
		} catch (...) {
			// Facet not available or error occurred
		}
		// Fallback: return the input sequence as-is
		return string_type(first, last);
	}

private:
	locale_type m_locale;
	
	// Transform implementation for char
	string_type transform_impl(const char_type* first, const char_type* last, std::true_type) const {
		// Use locale's collate facet for char
		try {
			const std::collate<char_type>& col = std::use_facet<std::collate<char_type>>(m_locale);
			return col.transform(first, last);
		} catch (...) {
			// Fallback if facet not available
			return string_type(first, last);
		}
	}
	
	// Transform implementation for non-char types
	string_type transform_impl(const char_type* first, const char_type* last, std::false_type) const {
		// For wchar_t, use locale's collate facet
		// For char16_t and char32_t, fall back to simple copy (portable default)
		return transform_wchar_impl(first, last, typename std::is_same<char_type, wchar_t>::type());
	}
	
	// Transform implementation for wchar_t
	string_type transform_wchar_impl(const char_type* first, const char_type* last, std::true_type) const {
		try {
			const std::collate<char_type>& col = std::use_facet<std::collate<char_type>>(m_locale);
			return col.transform(first, last);
		} catch (...) {
			// Fallback if facet not available
			return string_type(first, last);
		}
	}
	
	// Transform implementation for char16_t and char32_t
	string_type transform_wchar_impl(const char_type* first, const char_type* last, std::false_type) const {
		// Simple copy for char16_t and char32_t (portable default)
		return string_type(first, last);
	}
	
	// isctype implementation for char
	bool isctype_impl(char_type c, char_class_type f, std::true_type) const {
		// Use locale's ctype facet for char
		try {
			const std::ctype<char_type>& ct = std::use_facet<std::ctype<char_type>>(m_locale);
			return ct.is(static_cast<std::ctype_base::mask>(f), c);
		} catch (...) {
			// Fallback: return false as safe default
			return false;
		}
	}
	
	// isctype implementation for non-char types
	bool isctype_impl(char_type c, char_class_type f, std::false_type) const {
		// For wchar_t, use locale's ctype facet
		// For char16_t and char32_t, provide basic fallback
		return isctype_wchar_impl(c, f, typename std::is_same<char_type, wchar_t>::type());
	}
	
	// isctype implementation for wchar_t
	bool isctype_wchar_impl(char_type c, char_class_type f, std::true_type) const {
		try {
			const std::ctype<char_type>& ct = std::use_facet<std::ctype<char_type>>(m_locale);
			return ct.is(static_cast<std::ctype_base::mask>(f), c);
		} catch (...) {
			return false;
		}
	}
	
	// isctype implementation for char16_t and char32_t
	bool isctype_wchar_impl(char_type c, char_class_type f, std::false_type) const {
		// Basic fallback for char16_t and char32_t
		(void)c;
		(void)f;
		return false;
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

	// Sentinel value for "not found" (matches std::match_results behavior)
	// npos is defined as -1 to match std::match_results semantics where
	// position() returns a signed difference_type and uses -1 to indicate
	// "not found" for unmatched or out-of-range submatches
	static constexpr difference_type npos = -1;

	match_results() : m_str_begin(), m_str_end() {}
	
	// Static assertion to ensure npos semantics match expectations
	static_assert(npos == static_cast<difference_type>(-1), 
	              "npos must be -1 to match std::match_results semantics");

	// Added convenience functions
	bool empty() const { return this->size() == 0; }

	// Shortcut to the entire matched string
	string_type str(size_type n = 0) const {
		return (*this)[n].str();
	}

	// Returns the byte/element offset from search-range begin for the n-th submatch
	// Returns npos (-1) if n >= size() or if the n-th submatch is not matched
	// This matches std::match_results::position() semantics exactly
	// Note: For non-random-access iterators, this uses std::distance and is O(n)
	difference_type position(size_type n = 0) const {
		if (n >= this->size())
			return npos;
		if (!(*this)[n].matched)
			return npos;
		return std::distance(m_str_begin, (*this)[n].first);
	}

	// Returns the matched length for the n-th submatch
	// Returns 0 if n >= size() or if the n-th submatch is not matched
	// This matches std::match_results::length() semantics exactly
	// Note: For non-random-access iterators, this uses std::distance and is O(n)
	difference_type length(size_type n = 0) const {
		if (n >= this->size())
			return 0;
		if (!(*this)[n].matched)
			return 0;
		return std::distance((*this)[n].first, (*this)[n].second);
	}

	// Prefix and suffix
	const value_type prefix() const;
	const value_type suffix() const;

public:
	BidirIt m_str_begin; // Start iterator of the search range
	BidirIt m_str_end;   // End iterator of the search range
};

using cmatch = match_results<const char*>;
using wcmatch = match_results<const wchar_t*>;
using u16cmatch = match_results<const char16_t*>;
using u32cmatch = match_results<const char32_t*>;
using smatch = match_results<string::const_iterator>;
using wsmatch = match_results<wstring::const_iterator>;
using u16smatch = match_results<u16string::const_iterator>;
using u32smatch = match_results<u32string::const_iterator>;

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

	// Provide std::regex-like nested names for common flags so callers can write:
	//   onigpp::regex::ECMAScript, onigpp::regex::icase, etc.
	enum : flag_type {
		ECMAScript = regex_constants::ECMAScript,
		basic      = regex_constants::basic,
		extended   = regex_constants::extended,
		awk        = regex_constants::awk,
		grep       = regex_constants::grep,
		egrep      = regex_constants::egrep,
		icase      = regex_constants::icase,
		multiline  = regex_constants::multiline,
		collate    = regex_constants::collate,
		normal     = regex_constants::normal
	};

	basic_regex() : m_regex(nullptr), m_encoding(nullptr), m_flags(regex_constants::normal), m_locale(std::locale()) { }
	explicit basic_regex(const CharT* s, flag_type f = regex_constants::normal, OnigEncoding enc = nullptr)
		: basic_regex(s, Traits::length(s), f, enc) { }
	basic_regex(const CharT* s, size_type count, flag_type f = regex_constants::normal, OnigEncoding enc = nullptr);
	basic_regex(const string_type& s, flag_type f = regex_constants::normal, OnigEncoding enc = nullptr);
	
	// Iterator-range constructor
	template <class BidiIterator>
	basic_regex(BidiIterator first, BidiIterator last, flag_type f = regex_constants::normal, OnigEncoding enc = nullptr)
		: m_regex(nullptr), m_encoding(enc), m_flags(f), m_pattern(), m_locale(std::locale())
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
	string_type _emulate_ecmascript_multiline(const string_type& pattern) const;
};

////////////////////////////////////////////
// onigpp::regex, onigpp::wregex

using regex = basic_regex<char>;
using wregex = basic_regex<wchar_t>;
using u16regex = basic_regex<char16_t>;
using u32regex = basic_regex<char32_t>;

////////////////////////////////////////////
// Non-member swap for basic_regex

template <class CharT, class Traits>
void swap(basic_regex<CharT, Traits>& lhs, basic_regex<CharT, Traits>& rhs) noexcept {
	lhs.swap(rhs);
}

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
	
	// 2a. Convenience constructor with std::initializer_list
	regex_token_iterator(BidirIt first, BidirIt last,
						 const regex_type& re,
						 std::initializer_list<int> subs,
						 match_flag_type flags = regex_constants::match_default);
	
	// 2b. Convenience constructor with single int (common case)
	regex_token_iterator(BidirIt first, BidirIt last,
						 const regex_type& re,
						 int sub,
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
