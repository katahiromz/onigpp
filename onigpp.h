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
#include <ostream>

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

	// oniguruma: Enable Oniguruma's native syntax and behavior
	static constexpr syntax_option_type oniguruma = (1 << 6);

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

	// Additional std::regex compatibility flags for word boundaries and continuous matching
	// match_not_bow: First position is not treated as beginning-of-word (affects \b word boundary)
	// match_not_eow: Last position is not treated as end-of-word (affects \b word boundary)
	// match_continuous: Match must start exactly at the search start position (like \G anchor)
	static constexpr match_flag_type match_not_bow = (1 << 10); // std::regex_constants::match_not_bow
	static constexpr match_flag_type match_not_eow = (1 << 11); // std::regex_constants::match_not_eow
	static constexpr match_flag_type match_continuous = (1 << 12); // std::regex_constants::match_continuous

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
#define DEFINE_ENCODING(name) static OnigEncoding const name = ONIG_ENCODING_##name;
	#include "supported_encodings.h"
#undef DEFINE_ENCODING
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

	// Compare the matched substring with another sub_match
	// Returns negative if this < other, positive if this > other, 0 if equal
	// Uses str() semantics: unmatched sub_match compares as empty string
	int compare(const sub_match& other) const {
		return str().compare(other.str());
	}

	// Compare the matched substring with a string_type
	int compare(const string_type& s) const {
		return str().compare(s);
	}

	// Compare the matched substring with a null-terminated C-string
	int compare(const value_type* s) const {
		return str().compare(s);
	}
};

////////////////////////////////////////////
// Non-member comparison operators for sub_match
// std::sub_match provides comprehensive comparison operators for:
// 1. sub_match vs sub_match
// 2. sub_match vs string_type (and vice versa)
// 3. sub_match vs const value_type* (and vice versa)

// sub_match vs sub_match
template <class BidirIt>
bool operator==(const sub_match<BidirIt>& lhs, const sub_match<BidirIt>& rhs) {
	return lhs.compare(rhs) == 0;
}

template <class BidirIt>
bool operator!=(const sub_match<BidirIt>& lhs, const sub_match<BidirIt>& rhs) {
	return lhs.compare(rhs) != 0;
}

template <class BidirIt>
bool operator<(const sub_match<BidirIt>& lhs, const sub_match<BidirIt>& rhs) {
	return lhs.compare(rhs) < 0;
}

template <class BidirIt>
bool operator<=(const sub_match<BidirIt>& lhs, const sub_match<BidirIt>& rhs) {
	return lhs.compare(rhs) <= 0;
}

template <class BidirIt>
bool operator>(const sub_match<BidirIt>& lhs, const sub_match<BidirIt>& rhs) {
	return lhs.compare(rhs) > 0;
}

template <class BidirIt>
bool operator>=(const sub_match<BidirIt>& lhs, const sub_match<BidirIt>& rhs) {
	return lhs.compare(rhs) >= 0;
}

// sub_match vs string_type
template <class BidirIt>
bool operator==(const sub_match<BidirIt>& lhs,
                const typename sub_match<BidirIt>::string_type& rhs) {
	return lhs.compare(rhs) == 0;
}

template <class BidirIt>
bool operator!=(const sub_match<BidirIt>& lhs,
                const typename sub_match<BidirIt>::string_type& rhs) {
	return lhs.compare(rhs) != 0;
}

template <class BidirIt>
bool operator<(const sub_match<BidirIt>& lhs,
               const typename sub_match<BidirIt>::string_type& rhs) {
	return lhs.compare(rhs) < 0;
}

template <class BidirIt>
bool operator<=(const sub_match<BidirIt>& lhs,
                const typename sub_match<BidirIt>::string_type& rhs) {
	return lhs.compare(rhs) <= 0;
}

template <class BidirIt>
bool operator>(const sub_match<BidirIt>& lhs,
               const typename sub_match<BidirIt>::string_type& rhs) {
	return lhs.compare(rhs) > 0;
}

template <class BidirIt>
bool operator>=(const sub_match<BidirIt>& lhs,
                const typename sub_match<BidirIt>::string_type& rhs) {
	return lhs.compare(rhs) >= 0;
}

// string_type vs sub_match
template <class BidirIt>
bool operator==(const typename sub_match<BidirIt>::string_type& lhs,
                const sub_match<BidirIt>& rhs) {
	return rhs.compare(lhs) == 0;
}

template <class BidirIt>
bool operator!=(const typename sub_match<BidirIt>::string_type& lhs,
                const sub_match<BidirIt>& rhs) {
	return rhs.compare(lhs) != 0;
}

template <class BidirIt>
bool operator<(const typename sub_match<BidirIt>::string_type& lhs,
               const sub_match<BidirIt>& rhs) {
	return rhs.compare(lhs) > 0;
}

template <class BidirIt>
bool operator<=(const typename sub_match<BidirIt>::string_type& lhs,
                const sub_match<BidirIt>& rhs) {
	return rhs.compare(lhs) >= 0;
}

template <class BidirIt>
bool operator>(const typename sub_match<BidirIt>::string_type& lhs,
               const sub_match<BidirIt>& rhs) {
	return rhs.compare(lhs) < 0;
}

template <class BidirIt>
bool operator>=(const typename sub_match<BidirIt>::string_type& lhs,
                const sub_match<BidirIt>& rhs) {
	return rhs.compare(lhs) <= 0;
}

// sub_match vs const value_type* (C-string)
template <class BidirIt>
bool operator==(const sub_match<BidirIt>& lhs,
                const typename std::iterator_traits<BidirIt>::value_type* rhs) {
	return lhs.compare(rhs) == 0;
}

template <class BidirIt>
bool operator!=(const sub_match<BidirIt>& lhs,
                const typename std::iterator_traits<BidirIt>::value_type* rhs) {
	return lhs.compare(rhs) != 0;
}

template <class BidirIt>
bool operator<(const sub_match<BidirIt>& lhs,
               const typename std::iterator_traits<BidirIt>::value_type* rhs) {
	return lhs.compare(rhs) < 0;
}

template <class BidirIt>
bool operator<=(const sub_match<BidirIt>& lhs,
                const typename std::iterator_traits<BidirIt>::value_type* rhs) {
	return lhs.compare(rhs) <= 0;
}

template <class BidirIt>
bool operator>(const sub_match<BidirIt>& lhs,
               const typename std::iterator_traits<BidirIt>::value_type* rhs) {
	return lhs.compare(rhs) > 0;
}

template <class BidirIt>
bool operator>=(const sub_match<BidirIt>& lhs,
                const typename std::iterator_traits<BidirIt>::value_type* rhs) {
	return lhs.compare(rhs) >= 0;
}

// const value_type* (C-string) vs sub_match
template <class BidirIt>
bool operator==(const typename std::iterator_traits<BidirIt>::value_type* lhs,
                const sub_match<BidirIt>& rhs) {
	return rhs.compare(lhs) == 0;
}

template <class BidirIt>
bool operator!=(const typename std::iterator_traits<BidirIt>::value_type* lhs,
                const sub_match<BidirIt>& rhs) {
	return rhs.compare(lhs) != 0;
}

template <class BidirIt>
bool operator<(const typename std::iterator_traits<BidirIt>::value_type* lhs,
               const sub_match<BidirIt>& rhs) {
	return rhs.compare(lhs) > 0;
}

template <class BidirIt>
bool operator<=(const typename std::iterator_traits<BidirIt>::value_type* lhs,
                const sub_match<BidirIt>& rhs) {
	return rhs.compare(lhs) >= 0;
}

template <class BidirIt>
bool operator>(const typename std::iterator_traits<BidirIt>::value_type* lhs,
               const sub_match<BidirIt>& rhs) {
	return rhs.compare(lhs) < 0;
}

template <class BidirIt>
bool operator>=(const typename std::iterator_traits<BidirIt>::value_type* lhs,
                const sub_match<BidirIt>& rhs) {
	return rhs.compare(lhs) <= 0;
}

// Stream output operator for sub_match
template <class CharT, class Traits, class BidirIt>
std::basic_ostream<CharT, Traits>&
operator<<(std::basic_ostream<CharT, Traits>& os, const sub_match<BidirIt>& m) {
	return os << m.str();
}

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
	// Type aliases for std::match_results compatibility
	using value_type = sub_match<BidirIt>;
	using const_reference = const value_type&;
	using reference = value_type&;
	using const_iterator = typename std::vector<sub_match<BidirIt>, Alloc>::const_iterator;
	using iterator = typename std::vector<sub_match<BidirIt>, Alloc>::iterator;
	using difference_type = typename std::iterator_traits<BidirIt>::difference_type;
	using size_type = typename std::allocator_traits<Alloc>::size_type;
	using allocator_type = Alloc;
	using char_type = typename std::iterator_traits<BidirIt>::value_type;
	using string_type = basic_string<char_type>;

	// Sentinel value for "not found" (matches std::match_results behavior)
	// npos is defined as -1 to match std::match_results semantics where
	// position() returns a signed difference_type and uses -1 to indicate
	// "not found" for unmatched or out-of-range submatches
	static constexpr difference_type npos = -1;

	// Default constructor
	match_results() : m_str_begin(), m_str_end(), m_ready(false) {}

	// Constructor with allocator
	explicit match_results(const Alloc& a)
		: std::vector<sub_match<BidirIt>, Alloc>(a), m_str_begin(), m_str_end(), m_ready(false) {}

	// Copy constructor
	match_results(const match_results& other) = default;

	// Move constructor
	match_results(match_results&& other) noexcept = default;

	// Copy assignment operator
	match_results& operator=(const match_results& other) = default;

	// Move assignment operator
	match_results& operator=(match_results&& other) noexcept = default;

	// Destructor
	~match_results() = default;

	// Returns true if the match_results is ready (i.e., has been populated
	// by a regex_match or regex_search operation, regardless of whether
	// a match was found). Returns false for default-constructed match_results.
	// This matches std::match_results::ready() semantics.
	bool ready() const noexcept { return m_ready; }

	// Static assertion to ensure npos semantics match expectations
	static_assert(npos == static_cast<difference_type>(-1),
	              "npos must be -1 to match std::match_results semantics");

	// Returns a copy of the allocator
	allocator_type get_allocator() const noexcept {
		return std::vector<sub_match<BidirIt>, Alloc>::get_allocator();
	}

	// Added convenience functions
	bool empty() const noexcept { return this->size() == 0; }

	// Swap member function
	void swap(match_results& other) noexcept {
		std::vector<sub_match<BidirIt>, Alloc>::swap(other);
		std::swap(m_str_begin, other.m_str_begin);
		std::swap(m_str_end, other.m_str_end);
		std::swap(m_ready, other.m_ready);
	}

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

	// Format function - produces a string using the format string fmt
	// Supports the following placeholders:
	//   $n   - n-th submatch (0 = full match, 1-9+ = capture groups, multi-digit supported)
	//   ${n} - n-th submatch with explicit delimiters (safe for ${12}abc vs $12abc)
	//   $&   - full match (equivalent to $0)
	//   $`   - prefix (text before match)
	//   $'   - suffix (text after match)
	//   $$   - literal '$'
	// Escape sequences in fmt:
	//   \n   - newline
	//   \t   - tab
	//   \r   - carriage return
	//   \\   - literal backslash
	// Unmatched submatches are replaced with an empty string.
	template <class OutputIt>
	OutputIt format(OutputIt out, const char_type* fmt_first, const char_type* fmt_last,
	                regex_constants::match_flag_type flags = regex_constants::format_default) const;

	// Extended format function with named group resolver callback
	// This overload supports ${name} and \k<name>/\k'name' syntax for named groups.
	// The name_resolver callback takes (name_begin, name_end) pointers and returns
	// the group number (>= 0) or -1 if the name is not found.
	// When oniguruma_mode is true, backslash escapes are treated as backreferences (\1, \k<name>)
	// instead of escape sequences (\n, \t, etc.)
	template <class OutputIt, class NameResolver>
	OutputIt format(OutputIt out, const char_type* fmt_first, const char_type* fmt_last,
	                regex_constants::match_flag_type flags, NameResolver name_resolver, bool oniguruma_mode) const;

	template <class OutputIt>
	OutputIt format(OutputIt out, const string_type& fmt,
	                regex_constants::match_flag_type flags = regex_constants::format_default) const {
		return format(out, fmt.data(), fmt.data() + fmt.size(), flags);
	}

	string_type format(const char_type* fmt_first, const char_type* fmt_last,
	                   regex_constants::match_flag_type flags = regex_constants::format_default) const {
		string_type result;
		format(std::back_inserter(result), fmt_first, fmt_last, flags);
		return result;
	}

	string_type format(const string_type& fmt,
	                   regex_constants::match_flag_type flags = regex_constants::format_default) const {
		return format(fmt.data(), fmt.data() + fmt.size(), flags);
	}

	string_type format(const char_type* fmt,
	                   regex_constants::match_flag_type flags = regex_constants::format_default) const {
		const char_type* end = fmt;
		while (*end) ++end;
		return format(fmt, end, flags);
	}

public:
	BidirIt m_str_begin; // Start iterator of the search range
	BidirIt m_str_end;   // End iterator of the search range
	bool m_ready;        // True if populated by a regex operation
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
// Non-member swap for match_results

template <class BidirIt, class Alloc>
void swap(match_results<BidirIt, Alloc>& lhs, match_results<BidirIt, Alloc>& rhs) noexcept {
	lhs.swap(rhs);
}

////////////////////////////////////////////
// Comparison operators for match_results

// Helper function to compare sub_match objects without creating strings
template <class BidirIt>
inline bool _sub_match_equal(const sub_match<BidirIt>& lhs, const sub_match<BidirIt>& rhs) {
	if (lhs.matched != rhs.matched)
		return false;
	if (!lhs.matched)
		return true;
	// Compare iterator ranges directly
	if (std::distance(lhs.first, lhs.second) != std::distance(rhs.first, rhs.second))
		return false;
	return std::equal(lhs.first, lhs.second, rhs.first);
}

template <class BidirIt, class Alloc>
bool operator==(const match_results<BidirIt, Alloc>& lhs, const match_results<BidirIt, Alloc>& rhs) {
	if (lhs.ready() != rhs.ready())
		return false;
	if (!lhs.ready())
		return true;
	if (lhs.empty() != rhs.empty())
		return false;
	if (lhs.empty())
		return true;
	if (lhs.size() != rhs.size())
		return false;
	for (typename match_results<BidirIt, Alloc>::size_type i = 0; i < lhs.size(); ++i) {
		if (!_sub_match_equal(lhs[i], rhs[i]))
			return false;
	}
	return _sub_match_equal(lhs.prefix(), rhs.prefix()) && _sub_match_equal(lhs.suffix(), rhs.suffix());
}

template <class BidirIt, class Alloc>
bool operator!=(const match_results<BidirIt, Alloc>& lhs, const match_results<BidirIt, Alloc>& rhs) {
	return !(lhs == rhs);
}

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
		oniguruma  = regex_constants::oniguruma,
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
	using match_results_type = match_results<BidirIt>;

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

	// 5. Extension API: Access to the underlying match_results
	// Returns a const reference to the match_results for the current match.
	// This allows access to prefix(), suffix(), and all capture groups.
	// Note: This is an extension to the standard C++ regex_token_iterator API.
	// Precondition: The iterator must not be at the end-of-sequence,
	//               and the underlying regex_iterator must have a valid match.
	// When the iterator is processing suffix tokens (after all matches are found),
	// the underlying regex_iterator may be at end, so this method should only
	// be called when the iterator has a valid match, not when processing suffix.
	const match_results_type& current_match_results() const {
		assert(m_itor != m_end && "current_match_results() called on end-of-sequence or during suffix processing");
		return *m_itor;
	}
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

// Helper struct for parsing numeric sequences in format strings
// Used by match_results::format to reduce code duplication
// Methods return the parsed number (or -1 if invalid) and update an output parameter
// (end_ptr) to point past the last digit parsed.
template <class CharT>
struct _format_parse_numeric {
	// Maximum digits to parse (prevents int overflow, 9 digits = max 999999999 which fits in int)
	static const int max_digits = 9;

	// Parse a numeric string from [first, last) returning the number or -1 if non-numeric
	// Updates end_ptr to point past the last digit parsed
	static int parse_bounded(const CharT* first, const CharT* last, const CharT*& end_ptr) {
		// Check if content is purely numeric and within max_digits
		size_t len = last - first;
		if (len == 0 || len > static_cast<size_t>(max_digits)) {
			end_ptr = first;
			return -1;
		}
		int num = 0;
		for (const CharT* q = first; q != last; ++q) {
			if (*q < CharT('0') || *q > CharT('9')) {
				end_ptr = first;
				return -1; // Non-numeric character found
			}
			num = num * 10 + static_cast<int>(*q - CharT('0'));
		}
		end_ptr = last;
		return num;
	}

	// Parse digits greedily from p, up to max_digits, returning the number
	// Updates end_ptr to point past the last digit consumed
	static int parse_greedy(const CharT* p, const CharT* fmt_last, const CharT*& end_ptr) {
		int num = 0;
		const CharT* q = p;
		int digit_count = 0;
		while (q != fmt_last && *q >= CharT('0') && *q <= CharT('9') && digit_count < max_digits) {
			num = num * 10 + static_cast<int>(*q - CharT('0'));
			++q;
			++digit_count;
		}
		// Skip any remaining digits beyond max_digits (overflow protection)
		while (q != fmt_last && *q >= CharT('0') && *q <= CharT('9')) {
			++q;
		}
		end_ptr = q;
		return num;
	}
};

// match_results::format implementation
// Produces a string using the format string, supporting placeholders:
//   $n   - n-th submatch (0-9, multi-digit supported)
//   ${n} - n-th submatch with explicit delimiters
//   $&   - full match (equivalent to $0)
//   $`   - prefix (text before match)
//   $'   - suffix (text after match)
//   $$   - literal '$'
// Escape sequences:
//   \n   - newline
//   \t   - tab
//   \r   - carriage return
//   \\   - literal backslash
// Unmatched submatches are replaced with an empty string.
template <class BidirIt, class Alloc>
template <class OutputIt>
OutputIt match_results<BidirIt, Alloc>::format(
	OutputIt out,
	const char_type* fmt_first,
	const char_type* fmt_last,
	regex_constants::match_flag_type flags) const
{
	(void)flags; // Currently unused, reserved for future use

	const char_type* p = fmt_first;
	while (p != fmt_last) {
		// Handle '$' placeholders
		if (*p == char_type('$') && p + 1 != fmt_last) {
			char_type next = *(p + 1);

			// $$ -> literal '$'
			if (next == char_type('$')) {
				*out++ = char_type('$');
				p += 2;
				continue;
			}

			// $& -> full match ($0)
			if (next == char_type('&')) {
				if (!this->empty() && (*this)[0].matched) {
					out = std::copy((*this)[0].first, (*this)[0].second, out);
				}
				p += 2;
				continue;
			}

			// $` -> prefix
			if (next == char_type('`')) {
				auto pf = this->prefix();
				if (pf.matched) {
					out = std::copy(pf.first, pf.second, out);
				}
				p += 2;
				continue;
			}

			// $' -> suffix
			if (next == char_type('\'')) {
				auto sf = this->suffix();
				if (sf.matched) {
					out = std::copy(sf.first, sf.second, out);
				}
				p += 2;
				continue;
			}

			// ${n} - safe numbered group reference with explicit delimiters
			if (next == char_type('{')) {
				const char_type* name_start = p + 2;
				const char_type* name_end = name_start;
				while (name_end != fmt_last && *name_end != char_type('}')) {
					++name_end;
				}
				if (name_end != fmt_last && name_end > name_start) {
					// Found a valid ${...} reference with non-empty content
					// Use helper to parse numeric content
					const char_type* parse_end = name_start;
					int num = _format_parse_numeric<char_type>::parse_bounded(name_start, name_end, parse_end);
					if (num >= 0) {
						// Valid numeric reference: ${1}, ${2}, etc.
						if (static_cast<size_type>(num) < this->size() && (*this)[num].matched) {
							out = std::copy((*this)[num].first, (*this)[num].second, out);
						}
						p = name_end + 1; // Skip past the closing '}'
						continue;
					}
					// Non-numeric ${...} (named groups) - not supported in basic format,
					// skip the entire ${...} and output nothing (consistent with out-of-range group behavior)
					p = name_end + 1; // Skip past the closing '}'
					continue;
				} else {
					// Invalid ${...} reference (empty or unclosed), output literal '$'
					*out++ = *p++;
					continue;
				}
			}

			// $n, $nn - numeric capture group reference
			if (next >= char_type('0') && next <= char_type('9')) {
				// Use helper to parse digits greedily
				const char_type* q;
				int num = _format_parse_numeric<char_type>::parse_greedy(p + 1, fmt_last, q);
				// Output the submatch if valid and matched
				if (static_cast<size_type>(num) < this->size() && (*this)[num].matched) {
					out = std::copy((*this)[num].first, (*this)[num].second, out);
				}
				p = q;
				continue;
			}

			// Unknown $ sequence - output as-is
			*out++ = *p++;
			continue;
		}

		// Handle '\' escape sequences
		if (*p == char_type('\\') && p + 1 != fmt_last) {
			char_type next = *(p + 1);

			// \\ -> literal backslash
			if (next == char_type('\\')) {
				*out++ = char_type('\\');
				p += 2;
				continue;
			}

			// \n -> newline
			if (next == char_type('n')) {
				*out++ = char_type('\n');
				p += 2;
				continue;
			}

			// \t -> tab
			if (next == char_type('t')) {
				*out++ = char_type('\t');
				p += 2;
				continue;
			}

			// \r -> carriage return
			if (next == char_type('r')) {
				*out++ = char_type('\r');
				p += 2;
				continue;
			}

			// Unknown escape - output as-is
			*out++ = *p++;
			continue;
		}

		// Regular character
		*out++ = *p++;
	}

	return out;
}

// Extended match_results::format implementation with named group resolver
// Supports all basic placeholders plus:
//   ${name} - named group reference (resolved via name_resolver callback)
// When oniguruma_mode is true, backslash escapes are treated as backreferences:
//   \n      - n-th submatch (numeric backreference)
//   \k<name> or \k'name' - named group reference
//   \\      - literal backslash
// When oniguruma_mode is false, backslash escapes are escape sequences (\n, \t, \r, \\)
template <class BidirIt, class Alloc>
template <class OutputIt, class NameResolver>
OutputIt match_results<BidirIt, Alloc>::format(
	OutputIt out,
	const char_type* fmt_first,
	const char_type* fmt_last,
	regex_constants::match_flag_type flags,
	NameResolver name_resolver,
	bool oniguruma_mode) const
{
	(void)flags; // Currently unused, reserved for future use

	const char_type* p = fmt_first;
	while (p != fmt_last) {
		// Handle '$' placeholders
		if (*p == char_type('$') && p + 1 != fmt_last) {
			char_type next = *(p + 1);

			// $$ -> literal '$'
			if (next == char_type('$')) {
				*out++ = char_type('$');
				p += 2;
				continue;
			}

			// $& -> full match ($0)
			if (next == char_type('&')) {
				if (!this->empty() && (*this)[0].matched) {
					out = std::copy((*this)[0].first, (*this)[0].second, out);
				}
				p += 2;
				continue;
			}

			// $` -> prefix
			if (next == char_type('`')) {
				auto pf = this->prefix();
				if (pf.matched) {
					out = std::copy(pf.first, pf.second, out);
				}
				p += 2;
				continue;
			}

			// $' -> suffix
			if (next == char_type('\'')) {
				auto sf = this->suffix();
				if (sf.matched) {
					out = std::copy(sf.first, sf.second, out);
				}
				p += 2;
				continue;
			}

			// ${n} or ${name} - safe numbered reference or named group reference
			if (next == char_type('{')) {
				const char_type* name_start = p + 2;
				const char_type* name_end = name_start;
				while (name_end != fmt_last && *name_end != char_type('}')) {
					++name_end;
				}
				if (name_end != fmt_last && name_end > name_start) {
					// Found a valid ${...} reference with non-empty content
					// Use helper to parse numeric content
					const char_type* parse_end = name_start;
					int num = _format_parse_numeric<char_type>::parse_bounded(name_start, name_end, parse_end);
					if (num >= 0) {
						// Valid numeric reference: ${1}, ${2}, etc.
						if (static_cast<size_type>(num) < this->size() && (*this)[num].matched) {
							out = std::copy((*this)[num].first, (*this)[num].second, out);
						}
					} else {
						// Try as named group reference: ${name}
						num = name_resolver(name_start, name_end);
						if (num >= 0 && static_cast<size_type>(num) < this->size() && (*this)[num].matched) {
							out = std::copy((*this)[num].first, (*this)[num].second, out);
						}
					}
					p = name_end + 1; // Skip past the closing '}'
					continue;
				} else {
					// Invalid ${...} reference (empty or unclosed), output literal '$'
					*out++ = *p++;
					continue;
				}
			}

			// $n, $nn - numeric capture group reference
			if (next >= char_type('0') && next <= char_type('9')) {
				// Use helper to parse digits greedily
				const char_type* q;
				int num = _format_parse_numeric<char_type>::parse_greedy(p + 1, fmt_last, q);
				if (static_cast<size_type>(num) < this->size() && (*this)[num].matched) {
					out = std::copy((*this)[num].first, (*this)[num].second, out);
				}
				p = q;
				continue;
			}

			// Unknown $ sequence - output as-is
			*out++ = *p++;
			continue;
		}

		// Handle '\' sequences (different behavior based on oniguruma_mode)
		if (*p == char_type('\\') && p + 1 != fmt_last) {
			char_type next = *(p + 1);

			// \\ -> literal backslash (same in both modes)
			if (next == char_type('\\')) {
				*out++ = char_type('\\');
				p += 2;
				continue;
			}

			if (oniguruma_mode) {
				// Oniguruma mode: backslash escapes are backreferences
				// \k<name> or \k'name' - named group reference
				if (next == char_type('k') && p + 2 != fmt_last) {
					char_type delim = *(p + 2);
					char_type close_delim = char_type('\0');
					if (delim == char_type('<')) {
						close_delim = char_type('>');
					} else if (delim == char_type('\'')) {
						close_delim = char_type('\'');
					}
					if (close_delim != char_type('\0')) {
						const char_type* name_start = p + 3;
						const char_type* name_end = name_start;
						while (name_end != fmt_last && *name_end != close_delim) {
							++name_end;
						}
						if (name_end != fmt_last && name_end > name_start) {
							// Found a valid \k<name> or \k'name' reference
							int num = name_resolver(name_start, name_end);
							if (num >= 0 && static_cast<size_type>(num) < this->size() && (*this)[num].matched) {
								out = std::copy((*this)[num].first, (*this)[num].second, out);
							}
							p = name_end + 1; // Skip past the closing delimiter
							continue;
						} else {
							// Invalid \k<...> or \k'...' reference, output literal '\k'
							*out++ = char_type('\\');
							*out++ = char_type('k');
							p += 2;
							continue;
						}
					} else {
						// \k not followed by < or ', output literal '\k'
						*out++ = char_type('\\');
						*out++ = char_type('k');
						p += 2;
						continue;
					}
				}

				// \n - numeric backreference (Oniguruma-style)
				if (next >= char_type('0') && next <= char_type('9')) {
					// Use helper to parse digits greedily
					const char_type* q;
					int num = _format_parse_numeric<char_type>::parse_greedy(p + 1, fmt_last, q);
					if (static_cast<size_type>(num) < this->size() && (*this)[num].matched) {
						out = std::copy((*this)[num].first, (*this)[num].second, out);
					}
					p = q;
					continue;
				}

				// Other escape sequences in oniguruma mode - output literal backslash
				*out++ = *p++;
				continue;
			} else {
				// Standard mode: backslash escapes are escape sequences
				// \n -> newline
				if (next == char_type('n')) {
					*out++ = char_type('\n');
					p += 2;
					continue;
				}

				// \t -> tab
				if (next == char_type('t')) {
					*out++ = char_type('\t');
					p += 2;
					continue;
				}

				// \r -> carriage return
				if (next == char_type('r')) {
					*out++ = char_type('\r');
					p += 2;
					continue;
				}

				// Unknown escape - output as-is
				*out++ = *p++;
				continue;
			}
		}

		// Regular character
		*out++ = *p++;
	}

	return out;
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
