// onigpp.cpp --- Oniguruma++ (onigpp) regular expression engine
// Author: katahiromz
// License: BSD-2-Clause

#include "../onigpp.h"
#include <iterator>
#include <memory>
#include <cctype>
#include <type_traits>

namespace onigpp {

////////////////////////////////////////////
// Implementation helpers

// Helper templates for POSIX class expansion with SFINAE
// Only enabled for char and wchar_t where std::ctype is available

// Primary template - returns pattern unchanged for unsupported types
template <class CharT, class Enable = void>
struct posix_class_expander {
	typedef std::basic_string<CharT> string_type;
	static string_type expand(const std::locale& loc, const string_type& pattern) {
		// For char16_t, char32_t, etc. - no locale support, return unchanged
		return pattern;
	}
};

// Specialization for char and wchar_t where std::ctype is available
template <class CharT>
struct posix_class_expander<CharT,
	typename std::enable_if<
		std::is_same<CharT, char>::value || std::is_same<CharT, wchar_t>::value
	>::type>
{
	typedef std::basic_string<CharT> string_type;
	static string_type expand(const std::locale& loc, const string_type& pattern);
};

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

// Helper trait to detect contiguous iterators (for optimization)
// In C++11, we conservatively detect only pointer types as contiguous
// This avoids complexity while still optimizing the most common case

// Primary template - assume non-contiguous
template <typename Iter>
struct is_contiguous_iterator : std::false_type {};

// Specialization for pointer types (always contiguous)
template <typename T>
struct is_contiguous_iterator<T*> : std::true_type {};

template <typename T>
struct is_contiguous_iterator<const T*> : std::true_type {};

// Helper function to get pointer from contiguous iterator (enabled only for pointer types)
template <typename Iter>
typename std::enable_if<
	is_contiguous_iterator<Iter>::value,
	Iter
>::type
get_contiguous_pointer(Iter it) {
	return it;  // For pointers, just return the pointer itself
}

// Helper function to expand capture groups ($1, $2, ... or \1, \2, ...) in the replacement string
template <class CharT>
void _append_replacement(
	basic_string<CharT>& result,
	const basic_string<CharT>& fmt,
	const match_results<const CharT*>& m,
	const basic_regex<CharT>& re)
{
	using string_t = basic_string<CharT>;

	const size_type len = fmt.size();
	size_type i = 0;

	while (i < len) {
		CharT c = fmt[i];

		// --- Escape sequence: starts with '$'
		if (c == CharT('$')) {
			i++;
			if (i >= len) { result += CharT('$'); break; }

			CharT n = fmt[i];

			//--------------------------
			// "$$" --> "$"
			//--------------------------
			if (n == CharT('$')) {
				result += CharT('$');
				i++;
				continue;
			}

			//--------------------------
			// "$&" --> whole match
			//--------------------------
			if (n == CharT('&')) {
				if (m.size() > 0 && m[0].matched)
					result.append(m[0].first, m[0].second);
				i++;
				continue;
			}

			//--------------------------
			// "$`" --> prefix
			//--------------------------
			if (n == CharT('`')) {
				auto pre = m.prefix();
				if (pre.matched)
					result.append(pre.first, pre.second);
				i++;
				continue;
			}

			//--------------------------
			// "$'" --> suffix
			//--------------------------
			if (n == CharT('\'')) {
				auto suf = m.suffix();
				if (suf.matched)
					result.append(suf.first, suf.second);
				i++;
				continue;
			}

			//--------------------------
			// "$+" --> last captured group
			//--------------------------
			if (n == CharT('+')) {
				int last = -1;
				for (size_type gi = 1; gi < m.size(); gi++) {
					if (m[gi].matched)
						last = (int)gi;
				}
				if (last >= 0)
					result.append(m[last].first, m[last].second);
				i++;
				continue;
			}

			//--------------------------
			// "${name}" --> named capture
			//--------------------------
			if (n == CharT('{')) {
				size_type start = ++i;
				while (i < len && fmt[i] != CharT('}')) i++;

				if (i >= len) {
					// missing '}' --> treat literally
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
				if (num > 0 && (size_type)num < m.size() && m[num].matched) {
					result.append(m[num].first, m[num].second);
				}
				continue;
			}

			//--------------------------
			// "$n" --> numeric capture
			//--------------------------
			if (n >= CharT('0') && n <= CharT('9')) {
				size_type num = 0;
				while (i < len && fmt[i] >= CharT('0') && fmt[i] <= CharT('9')) {
					num = num * 10 + (fmt[i] - CharT('0'));
					i++;
				}
				if (num < m.size() && m[num].matched)
					result.append(m[num].first, m[num].second);
				continue;
			}

			//--------------------------
			// Unknown pattern --> literal "$x"
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

// Helper template function specializations for C++11 compatibility
// Forward declaration of the primary template
template <class CharT>
OnigEncoding _get_default_encoding_from_char_type_impl();

// Note: Only the explicit specializations below for char, wchar_t, char16_t, 
// and char32_t should be used.

template <>
OnigEncoding _get_default_encoding_from_char_type_impl<char>() {
	return ONIG_ENCODING_UTF8; // Default is UTF-8
}

template <>
OnigEncoding _get_default_encoding_from_char_type_impl<wchar_t>() {
	// Use UTF-16 or UTF-32 depending on wchar_t size and endianness
	if (sizeof(wchar_t) == 2) {
		#if defined(_WIN32) || defined(__LITTLE_ENDIAN__) || \
			(defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
		return ONIG_ENCODING_UTF16_LE;
		#else
		return ONIG_ENCODING_UTF16_BE;
		#endif
	} else if (sizeof(wchar_t) == 4) {
		#if defined(_WIN32) || defined(__LITTLE_ENDIAN__) || \
			(defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
		return ONIG_ENCODING_UTF32_LE;
		#else
		return ONIG_ENCODING_UTF32_BE;
		#endif
	}
	return ONIG_ENCODING_UTF8;
}

template <>
OnigEncoding _get_default_encoding_from_char_type_impl<char16_t>() {
	#if defined(_WIN32) || defined(__LITTLE_ENDIAN__) || \
		(defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
	return ONIG_ENCODING_UTF16_LE;
	#else
	return ONIG_ENCODING_UTF16_BE;
	#endif
}

template <>
OnigEncoding _get_default_encoding_from_char_type_impl<char32_t>() {
	#if defined(_WIN32) || defined(__LITTLE_ENDIAN__) || \
		(defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
	return ONIG_ENCODING_UTF32_LE;
	#else
	return ONIG_ENCODING_UTF32_BE;
	#endif
}

template <class CharT>
OnigEncoding _get_default_encoding_from_char_type() {
	return _get_default_encoding_from_char_type_impl<CharT>();
}

// Internal implementation for non-contiguous iterators (uses buffer copy)
template <class BidirIt, class Alloc, class CharT, class Traits>
typename std::enable_if<
	!is_contiguous_iterator<BidirIt>::value,
	bool
>::type
_regex_search_with_context_impl(
	BidirIt whole_first, BidirIt search_start, BidirIt last,
	match_results<BidirIt, Alloc>& m,
	const basic_regex<CharT, Traits>& e,
	regex_constants::match_flag_type flags,
	OnigRegex reg,
	OnigOptionType onig_options,
	size_type total_len,
	size_type search_offset)
{
	// Copy the subject range into a temporary contiguous buffer to support
	// non-contiguous BidirectionalIterators (e.g., std::list, std::deque)
	std::basic_string<CharT> subject_buf(whole_first, last);

	// Use stable static buffer for empty ranges to avoid passing nullptr to C API
	static thread_local CharT empty_char = CharT();
	const CharT* whole_begin_ptr = (total_len > 0) ? subject_buf.c_str() : &empty_char;
	const CharT* end_ptr = whole_begin_ptr + total_len;
	const CharT* start_ptr = whole_begin_ptr + search_offset;

	const OnigUChar* u_start = reinterpret_cast<const OnigUChar*>(whole_begin_ptr);
	const OnigUChar* u_end   = reinterpret_cast<const OnigUChar*>(end_ptr);
	const OnigUChar* u_search_start = reinterpret_cast<const OnigUChar*>(start_ptr);
	const OnigUChar* u_range = u_end; // Forward search range

	// Allocate OnigRegion
	OnigRegion* region = onig_region_new();
	if (!region) throw std::bad_alloc();

	// Execute search: pass whole begin as str, and search_start as start.
	int r = onig_search(reg, u_start, u_end, u_search_start, u_range, region, onig_options);

	if (r >= 0) {
		if (flags & regex_constants::match_not_null) {
			// If match length is zero, treat it as a match failure
			if (region->beg[0] == region->end[0]) {
				onig_region_free(region, 1);
				return false; // Equivalent to ONIG_MISMATCH
			}
		}

		// If matched, store results in match_results
		m.m_str_begin = whole_first;
		m.m_str_end = last;
		m.clear();
		
		// Check if nosubs flag is set on the regex - if so, don't populate submatches
		if (e.flags() & regex_constants::nosubs) {
			// nosubs: only indicate match success, don't store submatches
			onig_region_free(region, 1);
			return true;
		}
		
		m.resize(region->num_regs);

		for (int i = 0; i < region->num_regs; ++i) {
			int beg = region->beg[i];
			int end = region->end[i];

			if (beg != ONIG_REGION_NOTPOS) {
				int beg_chars = beg / sizeof(CharT);
				int end_chars = end / sizeof(CharT);

				BidirIt sub_start = whole_first;
				std::advance(sub_start, beg_chars);

				BidirIt sub_end = whole_first;
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
		throw regex_error(regex_constants::map_oniguruma_error(r), einfo);
	}
}

// Internal implementation for contiguous iterators (optimized, no buffer copy)
template <class BidirIt, class Alloc, class CharT, class Traits>
typename std::enable_if<
	is_contiguous_iterator<BidirIt>::value,
	bool
>::type
_regex_search_with_context_impl(
	BidirIt whole_first, BidirIt search_start, BidirIt last,
	match_results<BidirIt, Alloc>& m,
	const basic_regex<CharT, Traits>& e,
	regex_constants::match_flag_type flags,
	OnigRegex reg,
	OnigOptionType onig_options,
	size_type total_len,
	size_type search_offset)
{
	// Fast path: use direct pointer access for contiguous iterators
	// Use stable static buffer for empty ranges to avoid passing nullptr to C API
	static thread_local CharT empty_char = CharT();
	const CharT* whole_begin_ptr = (total_len > 0) ? get_contiguous_pointer(whole_first) : &empty_char;
	const CharT* end_ptr = whole_begin_ptr + total_len;
	const CharT* start_ptr = whole_begin_ptr + search_offset;

	const OnigUChar* u_start = reinterpret_cast<const OnigUChar*>(whole_begin_ptr);
	const OnigUChar* u_end   = reinterpret_cast<const OnigUChar*>(end_ptr);
	const OnigUChar* u_search_start = reinterpret_cast<const OnigUChar*>(start_ptr);
	const OnigUChar* u_range = u_end; // Forward search range

	// Allocate OnigRegion
	OnigRegion* region = onig_region_new();
	if (!region) throw std::bad_alloc();

	// Execute search: pass whole begin as str, and search_start as start.
	int r = onig_search(reg, u_start, u_end, u_search_start, u_range, region, onig_options);

	if (r >= 0) {
		if (flags & regex_constants::match_not_null) {
			// If match length is zero, treat it as a match failure
			if (region->beg[0] == region->end[0]) {
				onig_region_free(region, 1);
				return false; // Equivalent to ONIG_MISMATCH
			}
		}

		// If matched, store results in match_results
		m.m_str_begin = whole_first;
		m.m_str_end = last;
		m.clear();
		
		// Check if nosubs flag is set on the regex - if so, don't populate submatches
		if (e.flags() & regex_constants::nosubs) {
			// nosubs: only indicate match success, don't store submatches
			onig_region_free(region, 1);
			return true;
		}
		
		m.resize(region->num_regs);

		for (int i = 0; i < region->num_regs; ++i) {
			int beg = region->beg[i];
			int end = region->end[i];

			if (beg != ONIG_REGION_NOTPOS) {
				int beg_chars = beg / sizeof(CharT);
				int end_chars = end / sizeof(CharT);

				BidirIt sub_start = whole_first;
				std::advance(sub_start, beg_chars);

				BidirIt sub_end = whole_first;
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
		throw regex_error(regex_constants::map_oniguruma_error(r), einfo);
	}
}

// Public wrapper that dispatches to the appropriate implementation
// This performs search using Oniguruma's str/end/start parameters correctly.
// whole_first: iterator pointing to the beginning of the entire subject string
// search_start: iterator where search should begin (may be >= whole_first)
// Note: Optimized for contiguous iterators; uses buffer copy for non-contiguous iterators
template <class BidirIt, class Alloc, class CharT, class Traits>
bool _regex_search_with_context(
	BidirIt whole_first, BidirIt search_start, BidirIt last,
	match_results<BidirIt, Alloc>& m,
	const basic_regex<CharT, Traits>& e,
	regex_constants::match_flag_type flags)
{
	// Get Oniguruma regex object (using accessor hack)
	OnigRegex reg = regex_access<CharT, Traits>::get(e);
	if (!reg) return false;

	// Options before search execution
	OnigOptionType onig_options = 0;
	if (flags & regex_constants::match_not_bol) onig_options |= ONIG_OPTION_NOTBOL;
	if (flags & regex_constants::match_not_eol) onig_options |= ONIG_OPTION_NOTEOL;

	// Compute lengths
	size_type total_len = std::distance(whole_first, last);
	size_type search_offset = std::distance(whole_first, search_start);

	// Dispatch to the appropriate implementation based on iterator type
	return _regex_search_with_context_impl(whole_first, search_start, last, m, e, flags,
	                                       reg, onig_options, total_len, search_offset);
}

////////////////////////////////////////////
// Implementation of basic_regex

template <class CharT, class Traits>
OnigOptionType basic_regex<CharT, Traits>::_options_from_flags(flag_type f) {
	bool icase = !!(f & regex_constants::icase);
	bool multiline = !!(f & regex_constants::multiline);
	bool extended = !!(f & regex_constants::extended);
	bool ecmascript = !!(f & regex_constants::ECMAScript);

	// Note: New std::regex compatible flags are handled as follows:
	// - nosubs: handled at match/search time by not populating match_results
	// - optimize: currently a no-op (reserved for future optimization)
	// - collate: handled by _preprocess_pattern_for_locale in constructors
	// These flags don't map directly to Oniguruma options

	OnigOptionType options = 0;
	options |= (icase ? ONIG_OPTION_IGNORECASE : 0);
	
	// ECMAScript mode: handle dot and anchor behavior separately
	if (ecmascript) {
		// In ECMAScript:
		// - By default, dot does NOT match newline (use SINGLELINE option)
		// - When multiline flag is set, we use pattern transformation to emulate
		//   ECMAScript semantics (^ and $ match at line boundaries) without enabling
		//   Oniguruma's MULTILINE option (which would also change dot behavior)
		// - Pattern transformation is handled in _emulate_ecmascript_multiline()
		options |= ONIG_OPTION_SINGLELINE; // dot doesn't match newline
		// Note: multiline anchor behavior is implemented via pattern rewriting,
		// so we do NOT set ONIG_OPTION_MULTILINE here
	} else {
		// Non-ECMAScript modes: use original behavior
		options |= (multiline ? (ONIG_OPTION_MULTILINE | ONIG_OPTION_NEGATE_SINGLELINE) : ONIG_OPTION_SINGLELINE);
	}
	
	options |= (extended ? ONIG_OPTION_EXTEND : 0);
	return options;
}

template <class CharT, class Traits>
OnigSyntaxType* basic_regex<CharT, Traits>::_syntax_from_flags(flag_type f) {
	// Priority order: pick the first matching explicit grammar flag.
	// If multiple grammar flags are set, the priority ensures stable, deterministic selection.
	// Note: regex_constants::extended serves dual purpose - both POSIX extended grammar
	// and free-spacing option (ONIG_OPTION_EXTEND), which is compatible with std::regex semantics.
	if (f & regex_constants::basic)
		return ONIG_SYNTAX_POSIX_BASIC;
	else if (f & regex_constants::extended)
		return ONIG_SYNTAX_POSIX_EXTENDED;
	else if (f & regex_constants::awk)
		return ONIG_SYNTAX_POSIX_EXTENDED; // AWK regex is similar to POSIX extended
	else if (f & regex_constants::grep)
		return ONIG_SYNTAX_GREP;
	else if (f & regex_constants::egrep)
		return ONIG_SYNTAX_POSIX_EXTENDED; // egrep uses extended regex (grep -E)
	else if (f & regex_constants::ECMAScript)
		return ONIG_SYNTAX_ONIGURUMA; // Use Oniguruma default for ECMAScript-like behavior
	else
		return ONIG_SYNTAX_ONIGURUMA; // Default when no grammar flag specified
}

template <class CharT, class Traits>
basic_regex<CharT, Traits>::basic_regex(const string_type& s, flag_type f, OnigEncoding enc) : basic_regex(s.c_str(), s.length(), f, enc) { }

template <class CharT, class Traits>
basic_regex<CharT, Traits>::basic_regex(const CharT* s, size_type count, flag_type f, OnigEncoding enc)
	: m_regex(nullptr), m_encoding(nullptr), m_flags(f), m_pattern(s, count), m_locale(std::locale())
{
	OnigSyntaxType* syntax = _syntax_from_flags(f);
	OnigOptionType options = _options_from_flags(f);
	OnigErrorInfo err_info;
	if (!enc) enc = _get_default_encoding_from_char_type<CharT>();
	m_encoding = enc;
	
	// Preprocess pattern for ECMAScript compatibility if needed
	string_type compiled_pattern = m_pattern;
	if (f & regex_constants::ECMAScript) {
		compiled_pattern = _preprocess_pattern_for_ecmascript(compiled_pattern);
	}
	
	// Preprocess pattern for locale support (when collate flag is set)
	if (f & regex_constants::collate) {
		compiled_pattern = _preprocess_pattern_for_locale(compiled_pattern);
	}
	const CharT* pattern_ptr = compiled_pattern.c_str();
	size_type pattern_len = compiled_pattern.length();
	
	int err = onig_new(&m_regex, reinterpret_cast<const OnigUChar*>(pattern_ptr), 
	                   reinterpret_cast<const OnigUChar*>(pattern_ptr + pattern_len), 
	                   options, enc, syntax, &err_info);
	if (err != ONIG_NORMAL) throw regex_error(regex_constants::map_oniguruma_error(err), err_info);
}

template <class CharT, class Traits>
basic_regex<CharT, Traits>::basic_regex(const self_type& other) 
	: m_regex(nullptr), m_encoding(other.m_encoding), m_flags(other.m_flags), m_pattern(other.m_pattern), m_locale(other.m_locale)
{
	if (!other.m_regex) return; // If the original object is invalid

	OnigSyntaxType* syntax = _syntax_from_flags(m_flags);
	OnigOptionType options = _options_from_flags(m_flags);
	OnigErrorInfo err_info;

	// Preprocess pattern for ECMAScript compatibility if needed
	string_type compiled_pattern = m_pattern;
	if (m_flags & regex_constants::ECMAScript) {
		compiled_pattern = _preprocess_pattern_for_ecmascript(compiled_pattern);
	}
	
	// Preprocess pattern for locale support (when collate flag is set)
	if (m_flags & regex_constants::collate) {
		compiled_pattern = _preprocess_pattern_for_locale(compiled_pattern);
	}
	const CharT* s = compiled_pattern.c_str();
	size_type count = compiled_pattern.length();

	int err = onig_new(&m_regex, reinterpret_cast<const OnigUChar*>(s), reinterpret_cast<const OnigUChar*>(s + count), 
					   options, m_encoding, syntax, &err_info);
	if (err != ONIG_NORMAL) throw regex_error(regex_constants::map_oniguruma_error(err), err_info);
}

template <class CharT, class Traits>
basic_regex<CharT, Traits>::basic_regex(self_type&& other) noexcept
	: m_regex(other.m_regex), m_encoding(other.m_encoding), m_flags(other.m_flags), 
	  m_pattern(std::move(other.m_pattern)), m_locale(std::move(other.m_locale))
{
	// leave other in safe state
	other.m_regex = nullptr;
	other.m_encoding = nullptr;
	other.m_flags = regex_constants::normal;
	other.m_pattern.clear();
	other.m_locale = std::locale();
}

// move assignment
template <class CharT, class Traits>
basic_regex<CharT, Traits>& basic_regex<CharT, Traits>::operator=(self_type&& other) noexcept {
	if (this == &other) return *this;

	// free current resource if any (check nullptr)
	if (m_regex) {
		// Use the appropriate Oniguruma API to free compiled regex.
		// If onig_free accepts nullptr safely then check is optional,
		// but checking avoids relying on that guarantee.
		onig_free(m_regex);
		m_regex = nullptr;
	}

	// steal resources
	m_regex = other.m_regex;
	m_encoding = other.m_encoding;
	m_flags = other.m_flags;
	m_pattern = std::move(other.m_pattern);
	m_locale = std::move(other.m_locale);

	// reset other to safe state
	other.m_regex = nullptr;
	other.m_encoding = nullptr;
	other.m_flags = regex_constants::normal;
	other.m_pattern.clear();
	other.m_locale = std::locale();

	return *this;
}

template <class CharT, class Traits>
basic_regex<CharT, Traits>::~basic_regex() {
	if (m_regex) {
		onig_free(m_regex);
		m_regex = nullptr;
	}
}

template <class CharT, class Traits>
unsigned basic_regex<CharT, Traits>::mark_count() const {
	if (!m_regex) return 0;
	return onig_number_of_captures(m_regex);
}

template <class CharT, class Traits>
typename basic_regex<CharT, Traits>::string_type 
basic_regex<CharT, Traits>::_preprocess_pattern_for_locale(const string_type& pattern) const {
	// Conservative POSIX character class expander for locale support
	// Expands [:digit:], [:alpha:], [:alnum:], [:space:], [:upper:], [:lower:],
	// [:punct:], [:xdigit:], [:cntrl:], [:print:], [:graph:] inside bracket expressions
	//
	// Note: Oniguruma natively supports POSIX character classes when using POSIX syntaxes.
	// We only need to preprocess for other syntaxes (like Oniguruma default or ECMAScript).
	//
	// Limitations:
	// - Only implemented for char and wchar_t (not char16_t/char32_t)
	// - For char: enumerates all 256 values (0-255)
	// - For wchar_t: enumerates up to U+0800 (~2K characters) to avoid very large expansions
	//   Covers Basic Latin, Latin-1 Supplement, Latin Extended A/B, and other common blocks
	// - Performance: Expansion happens once during pattern compilation; fast for typical use cases
	//
	// Check if we're using a POSIX syntax that already supports these classes.
	OnigSyntaxType* syntax = _syntax_from_flags(m_flags);
	if (syntax == ONIG_SYNTAX_POSIX_BASIC || syntax == ONIG_SYNTAX_POSIX_EXTENDED || 
	    syntax == ONIG_SYNTAX_GREP) {
		// These syntaxes natively support POSIX character classes, no preprocessing needed
		return pattern;
	}
	
	// Use SFINAE helper to only compile locale-based expansion for char and wchar_t
	return posix_class_expander<CharT>::expand(m_locale, pattern);
}

// Implementation of POSIX class expansion for char and wchar_t
// This function expands POSIX character classes (e.g., [:lower:], [:digit:]) inside bracket
// expressions into explicit character lists based on the imbued locale's ctype facet.
// The expansion is locale-aware and respects the character classification of the given locale.
template <class CharT>
typename posix_class_expander<CharT,
	typename std::enable_if<
		std::is_same<CharT, char>::value || std::is_same<CharT, wchar_t>::value
	>::type>::string_type
posix_class_expander<CharT,
	typename std::enable_if<
		std::is_same<CharT, char>::value || std::is_same<CharT, wchar_t>::value
	>::type>::expand(const std::locale& loc, const string_type& pattern) {
	typedef std::basic_string<CharT> string_type;
	typedef typename string_type::size_type size_type;
	
	string_type result;
	result.reserve(pattern.size());
	
	const std::ctype<CharT>& ct = std::use_facet<std::ctype<CharT>>(loc);
	
	size_type i = 0;
	const size_type len = pattern.size();
	
	// Helper lambda to compare strings
	auto str_equals = [](const string_type& s, const char* literal) -> bool {
		size_t lit_len = std::strlen(literal);
		if (s.size() != lit_len) return false;
		for (size_t j = 0; j < lit_len; ++j) {
			if (s[j] != CharT(literal[j])) return false;
		}
		return true;
	};
	
	while (i < len) {
		if (pattern[i] == CharT('[')) {
			// Start of bracket expression
			result += pattern[i++];
			
			// Check for negation
			if (i < len && pattern[i] == CharT('^')) {
				result += pattern[i++];
			}
			
			// Process inside bracket expression
			while (i < len && pattern[i] != CharT(']')) {
				// Check for POSIX class [:name:]
				if (i + 2 < len && pattern[i] == CharT('[') && pattern[i+1] == CharT(':')) {
					size_type class_start = i;
					i += 2;
					
					// Find the end of POSIX class name
					size_type name_start = i;
					while (i < len && pattern[i] != CharT(':')) {
						i++;
					}
					
					if (i + 1 < len && pattern[i] == CharT(':') && pattern[i+1] == CharT(']')) {
						// Found complete POSIX class
						string_type class_name = pattern.substr(name_start, i - name_start);
						i += 2; // skip ':]'
						
						// Expand the POSIX class based on locale
						string_type expansion;
						
						// Test characters in a reasonable range.
						// For char: all 256 possible values (0-255)
						// For wchar_t: enumerate up to U+0800 (2048 characters) to avoid very large expansions
						// This covers Basic Latin, Latin-1 Supplement, Latin Extended A/B, and other common
						// character blocks sufficient for most locale-aware character classification.
						// Performance: ~2K iterations for wchar_t; fast and practical for typical use cases.
						const int max_char = (sizeof(CharT) == 1) ? 256 : 0x800;
						
						// Determine which class we're dealing with
						std::ctype_base::mask mask = 0;
						bool recognized = false;
						
						if (str_equals(class_name, "digit")) {
							mask = std::ctype_base::digit;
							recognized = true;
						} else if (str_equals(class_name, "alpha")) {
							mask = std::ctype_base::alpha;
							recognized = true;
						} else if (str_equals(class_name, "alnum")) {
							mask = std::ctype_base::alnum;
							recognized = true;
						} else if (str_equals(class_name, "space")) {
							mask = std::ctype_base::space;
							recognized = true;
						} else if (str_equals(class_name, "upper")) {
							mask = std::ctype_base::upper;
							recognized = true;
						} else if (str_equals(class_name, "lower")) {
							mask = std::ctype_base::lower;
							recognized = true;
						} else if (str_equals(class_name, "punct")) {
							mask = std::ctype_base::punct;
							recognized = true;
						} else if (str_equals(class_name, "xdigit")) {
							mask = std::ctype_base::xdigit;
							recognized = true;
						} else if (str_equals(class_name, "cntrl")) {
							mask = std::ctype_base::cntrl;
							recognized = true;
						} else if (str_equals(class_name, "print")) {
							mask = std::ctype_base::print;
							recognized = true;
						} else if (str_equals(class_name, "graph")) {
							mask = std::ctype_base::graph;
							recognized = true;
						}
						
						if (recognized) {
							bool first_char = true;
							for (int c = 0; c < max_char; ++c) {
								CharT ch = static_cast<CharT>(c);
								if (ct.is(mask, ch)) {
									// Need to escape special regex characters inside bracket expressions
									// Backslash, closing bracket always need escaping
									if (ch == CharT('\\') || ch == CharT(']')) {
										expansion += CharT('\\');
										expansion += ch;
									}
									// Opening bracket needs escaping 
									else if (ch == CharT('[')) {
										expansion += CharT('\\');
										expansion += ch;
									}
									// Braces need escaping in extended mode (for repetition syntax)
									else if (ch == CharT('{') || ch == CharT('}')) {
										expansion += CharT('\\');
										expansion += ch;
									}
									// Caret only needs escaping at the beginning
									else if (ch == CharT('^') && first_char) {
										expansion += CharT('\\');
										expansion += ch;
									}
									// Hyphen: place it at the end to avoid range issues
									// We'll collect hyphens separately
									else if (ch != CharT('-')) {
										expansion += ch;
									}
									first_char = false;
								}
							}
							
							// Add hyphen at the end if it was in the character class
							// At the end of a bracket expression, hyphen doesn't need escaping
							for (int c = 0; c < max_char; ++c) {
								CharT ch = static_cast<CharT>(c);
								if (ch == CharT('-') && ct.is(mask, ch)) {
									expansion += ch;
									break;  // Only add one hyphen
								}
							}
							
							// If expansion is empty (no characters match the class),
							// we need to ensure the bracket expression isn't empty to avoid
							// "empty range in char class" errors.
							// Use a non-printing character unlikely to appear in normal text.
							static const CharT UNMATCHABLE_CHAR = CharT('\x7F'); // DEL character (ASCII 127)
							if (expansion.empty()) {
								expansion += UNMATCHABLE_CHAR;
							}
							
							result += expansion;
						} else {
							// Not a recognized POSIX class, restore original
							result += pattern.substr(class_start, i - class_start);
						}
					} else {
						// Not a complete POSIX class, treat literally
						result += pattern.substr(class_start, i - class_start);
					}
				} else {
					// Regular character in bracket expression
					result += pattern[i++];
				}
			}
			
			// Add closing bracket
			if (i < len && pattern[i] == CharT(']')) {
				result += pattern[i++];
			}
		} else {
			// Outside bracket expression
			result += pattern[i++];
		}
	}
	
	return result;
}

template <class CharT, class Traits>
typename basic_regex<CharT, Traits>::string_type 
basic_regex<CharT, Traits>::_preprocess_pattern_for_ecmascript(const string_type& pattern) const {
	// ECMAScript pattern preprocessing for compatibility with std::regex ECMAScript mode
	// Handles: \xHH, \uHHHH, \0, named capture normalization, and anchor semantics
	
	typedef typename string_type::size_type size_type;
	
	// Handle anchor semantics based on multiline flag
	string_type working_pattern = pattern;
	if (m_flags & regex_constants::multiline) {
		// When multiline is set, emulate ECMAScript multiline mode:
		// ^ matches at start of string or after line terminators
		// $ matches at end of string or before line terminators
		working_pattern = _emulate_ecmascript_multiline(working_pattern);
	} else {
		// When multiline is NOT set (default ECMAScript behavior):
		// Convert ^ to \A (absolute start of string)
		// Convert $ to \z (absolute end of string)
		// This ensures behavior matches std::regex ECMAScript mode
		working_pattern = _convert_anchors_to_absolute(working_pattern);
	}
	
	string_type result;
	result.reserve(working_pattern.size());
	
	size_type i = 0;
	const size_type len = working_pattern.size();
	
	// Helper to check if a character is a hex digit
	auto is_hex_digit = [](CharT ch) -> bool {
		return (ch >= CharT('0') && ch <= CharT('9')) ||
		       (ch >= CharT('a') && ch <= CharT('f')) ||
		       (ch >= CharT('A') && ch <= CharT('F'));
	};
	
	// Helper to convert hex character to value
	auto hex_value = [](CharT ch) -> int {
		if (ch >= CharT('0') && ch <= CharT('9')) return ch - CharT('0');
		if (ch >= CharT('a') && ch <= CharT('f')) return ch - CharT('a') + 10;
		if (ch >= CharT('A') && ch <= CharT('F')) return ch - CharT('A') + 10;
		return 0;
	};
	
	// Helper to check if character is an octal digit
	auto is_octal_digit = [](CharT ch) -> bool {
		return ch >= CharT('0') && ch <= CharT('7');
	};
	
	while (i < len) {
		if (working_pattern[i] == CharT('\\') && i + 1 < len) {
			CharT next = working_pattern[i + 1];
			
			// Handle \xHH - two hex digit escape
			if (next == CharT('x') && i + 3 < len &&
			    is_hex_digit(working_pattern[i + 2]) && is_hex_digit(working_pattern[i + 3])) {
				int val = hex_value(working_pattern[i + 2]) * 16 + hex_value(working_pattern[i + 3]);
				result += static_cast<CharT>(val);
				i += 4;
				continue;
			}
			
			// Handle \uHHHH - four hex digit Unicode escape
			if (next == CharT('u') && i + 5 < len &&
			    is_hex_digit(working_pattern[i + 2]) && is_hex_digit(working_pattern[i + 3]) &&
			    is_hex_digit(working_pattern[i + 4]) && is_hex_digit(working_pattern[i + 5])) {
				int val = hex_value(working_pattern[i + 2]) * 4096 +
				         hex_value(working_pattern[i + 3]) * 256 +
				         hex_value(working_pattern[i + 4]) * 16 +
				         hex_value(working_pattern[i + 5]);
				
				// Convert Unicode code point to CharT
				// Note: \uHHHH can only represent U+0000 to U+FFFF (BMP)
				// ECMAScript uses UTF-16 surrogate pairs for code points above U+FFFF,
				// but we handle each \uHHHH independently here (no surrogate pair handling)
				// For char (UTF-8), we encode as UTF-8 (1-3 bytes for BMP range)
				// For char16_t/char32_t/wchar_t, store the value directly
				if (sizeof(CharT) == 1) {
					// UTF-8 encoding for char (BMP range uses 1-3 bytes)
					if (val <= 0x7F) {
						result += static_cast<CharT>(val);
					} else if (val <= 0x7FF) {
						result += static_cast<CharT>(0xC0 | (val >> 6));
						result += static_cast<CharT>(0x80 | (val & 0x3F));
					} else {
						// val is 0x800-0xFFFF, needs 3-byte UTF-8
						result += static_cast<CharT>(0xE0 | (val >> 12));
						result += static_cast<CharT>(0x80 | ((val >> 6) & 0x3F));
						result += static_cast<CharT>(0x80 | (val & 0x3F));
					}
				} else {
					// For wider character types, store directly
					result += static_cast<CharT>(val);
				}
				i += 6;
				continue;
			}
			
			// Handle \0 - null escape (only when NOT followed by octal digit)
			if (next == CharT('0') && (i + 2 >= len || !is_octal_digit(working_pattern[i + 2]))) {
				result += CharT('\0');
				i += 2;
				continue;
			}
			
			// For all other escapes, keep them as-is
			result += working_pattern[i++];
			result += working_pattern[i++];
		} else {
			// Regular character
			result += working_pattern[i++];
		}
	}
	
	return result;
}

template <class CharT, class Traits>
typename basic_regex<CharT, Traits>::string_type 
basic_regex<CharT, Traits>::_emulate_ecmascript_multiline(const string_type& pattern) const {
	// ECMAScript multiline emulation: Rewrite ^ and $ anchors to match at line boundaries
	// without enabling Oniguruma's MULTILINE option (which also changes dot behavior).
	//
	// In ECMAScript, the multiline flag affects only ^ and $ anchors:
	// - ^ matches at start of string OR after any line terminator
	// - $ matches at end of string OR before any line terminator
	// - Dot (.) behavior is NOT affected by multiline (controlled separately by dotall/s)
	//
	// This function rewrites:
	// - ^ to: (?:\A|(?:(?<=\n)|(?<=\r\n)|(?<=\r)|(?<=\u2028)|(?<=\u2029)))
	// - $ to: (?:\z|(?=(?:\r\n|\r|\n|\u2028|\u2029)))
	//
	// Note: The rewrite must only affect unescaped ^ and $ that are outside character classes.
	// Performance: Pattern rewriting adds CPU cost at regex compile time.
	// Limitations: Complex patterns with nested groups or unusual contexts may have edge cases.
	
	typedef typename string_type::size_type size_type;
	string_type result;
	result.reserve(pattern.size() * 2); // Reserve extra space for expansions
	
	size_type i = 0;
	const size_type len = pattern.size();
	bool in_char_class = false;
	int bracket_depth = 0; // Track nesting level for character classes
	
	// Replacement strings as constants for maintainability
	// In ECMAScript multiline: ^ matches at start OR after line terminators
	// Line terminators: LF (\n), CR (\r), CRLF (\r\n), U+2028 (line separator), U+2029 (paragraph separator)
	// Note: (?<=\r\n) is a fixed-length lookbehind (2 bytes) and is supported by Oniguruma
	static constexpr const char* CARET_REPLACEMENT = "(?:\\A|(?:(?<=\\n)|(?<=\\r\\n)|(?<=\\r)|(?<=\\u2028)|(?<=\\u2029)))";
	
	// In ECMAScript multiline: $ matches at end OR before line terminators
	static constexpr const char* DOLLAR_REPLACEMENT = "(?:\\z|(?=(?:\\r\\n|\\r|\\n|\\u2028|\\u2029)))";
	
	// Helper to append the replacement for ^ (start of line)
	auto append_caret_replacement = [&result]() {
		for (const char* p = CARET_REPLACEMENT; *p; ++p) {
			result += CharT(*p);
		}
	};
	
	// Helper to append the replacement for $ (end of line)
	auto append_dollar_replacement = [&result]() {
		for (const char* p = DOLLAR_REPLACEMENT; *p; ++p) {
			result += CharT(*p);
		}
	};
	
	while (i < len) {
		CharT ch = pattern[i];
		
		// Handle escape sequences
		if (ch == CharT('\\') && i + 1 < len) {
			// Copy escape sequence as-is (two characters)
			result += pattern[i++];
			result += pattern[i++];
			continue;
		}
		
		// Track character class boundaries
		if (ch == CharT('[') && !in_char_class) {
			in_char_class = true;
			bracket_depth = 1;
			result += pattern[i++];
			continue;
		}
		
		if (in_char_class) {
			if (ch == CharT('[')) {
				// Nested bracket (e.g., for POSIX classes like [[:digit:]])
				bracket_depth++;
			} else if (ch == CharT(']')) {
				bracket_depth--;
				if (bracket_depth == 0) {
					in_char_class = false;
				}
			}
			result += pattern[i++];
			continue;
		}
		
		// Not in character class and not escaped: check for ^ and $
		if (ch == CharT('^')) {
			// Replace unescaped ^ outside character classes
			append_caret_replacement();
			i++;
			continue;
		}
		
		if (ch == CharT('$')) {
			// Replace unescaped $ outside character classes
			append_dollar_replacement();
			i++;
			continue;
		}
		
		// Regular character
		result += pattern[i++];
	}
	
	return result;
}

template <class CharT, class Traits>
typename basic_regex<CharT, Traits>::string_type 
basic_regex<CharT, Traits>::_convert_anchors_to_absolute(const string_type& pattern) const {
	// Convert ECMAScript anchors to absolute anchors when multiline is NOT set
	// This ensures that ^ and $ match only at the start and end of the entire string,
	// not at line boundaries (which is the default behavior in std::regex ECMAScript mode
	// when multiline flag is not set).
	//
	// Transformation:
	// - ^ (unescaped, outside character classes) -> \A (absolute start)
	// - $ (unescaped, outside character classes) -> \z (absolute end)
	//
	// Note: Escaped anchors (\^ and \$) and anchors inside character classes are preserved.
	
	typedef typename string_type::size_type size_type;
	string_type result;
	result.reserve(pattern.size());
	
	size_type i = 0;
	const size_type len = pattern.size();
	bool in_char_class = false;
	int bracket_depth = 0; // Track nesting level for character classes
	
	while (i < len) {
		CharT ch = pattern[i];
		
		// Handle escape sequences
		if (ch == CharT('\\') && i + 1 < len) {
			// Copy escape sequence as-is (two characters)
			result += pattern[i++];
			result += pattern[i++];
			continue;
		}
		
		// Track character class boundaries
		if (ch == CharT('[') && !in_char_class) {
			in_char_class = true;
			bracket_depth = 1;
			result += pattern[i++];
			continue;
		}
		
		if (in_char_class) {
			if (ch == CharT('[')) {
				// Nested bracket (e.g., for POSIX classes like [[:digit:]])
				bracket_depth++;
			} else if (ch == CharT(']')) {
				bracket_depth--;
				if (bracket_depth == 0) {
					in_char_class = false;
				}
			}
			result += pattern[i++];
			continue;
		}
		
		// Not in character class and not escaped: check for ^ and $
		if (ch == CharT('^')) {
			// Replace unescaped ^ with \A (absolute start of string)
			result += CharT('\\');
			result += CharT('A');
			i++;
			continue;
		}
		
		if (ch == CharT('$')) {
			// Replace unescaped $ with \z (absolute end of string)
			result += CharT('\\');
			result += CharT('z');
			i++;
			continue;
		}
		
		// Regular character
		result += pattern[i++];
	}
	
	return result;
}

template <class CharT, class Traits>
typename basic_regex<CharT, Traits>::locale_type 
basic_regex<CharT, Traits>::imbue(locale_type loc) {
	locale_type old_locale = m_locale;
	m_locale = loc;
	
	// Recompile the regex if we have a pattern
	if (!m_pattern.empty()) {
		// Free existing regex
		if (m_regex) {
			onig_free(m_regex);
			m_regex = nullptr;
		}
		
		// Preprocess pattern for ECMAScript compatibility if needed
		string_type compiled_pattern = m_pattern;
		if (m_flags & regex_constants::ECMAScript) {
			compiled_pattern = _preprocess_pattern_for_ecmascript(compiled_pattern);
		}
		
		// Preprocess pattern with new locale (when collate flag is set)
		if (m_flags & regex_constants::collate) {
			compiled_pattern = _preprocess_pattern_for_locale(compiled_pattern);
		}
		
		// Compile with the preprocessed pattern
		OnigSyntaxType* syntax = _syntax_from_flags(m_flags);
		OnigOptionType options = _options_from_flags(m_flags);
		OnigErrorInfo err_info;
		
		const CharT* s = compiled_pattern.c_str();
		size_type count = compiled_pattern.length();
		
		int err = onig_new(&m_regex, reinterpret_cast<const OnigUChar*>(s), 
		                   reinterpret_cast<const OnigUChar*>(s + count), 
		                   options, m_encoding, syntax, &err_info);
		if (err != ONIG_NORMAL) throw regex_error(regex_constants::map_oniguruma_error(err), err_info);
	}
	
	return old_locale;
}

////////////////////////////////////////////
// regex_search implementation
// Note: Byte offset conversion assumes fixed-width encodings per CharT unit:
//   - char with UTF-8: 1 byte per char
//   - wchar_t/char16_t/char32_t with UTF-16/32: 2/4 bytes per unit

template <class BidirIt, class Alloc, class CharT, class Traits>
bool regex_search(
	BidirIt first, BidirIt last,
	match_results<BidirIt, Alloc>& m,
	const basic_regex<CharT, Traits>& e,
	regex_constants::match_flag_type flags)
{
	// Treat 'first' as both whole_begin and search_start for backward compatibility
	return _regex_search_with_context(first, first, last, m, e, flags);
}

////////////////////////////////////////////
// regex_match implementation

// Internal implementation for non-contiguous iterators (uses buffer copy)
template <class BidirIt, class Alloc, class CharT, class Traits>
typename std::enable_if<
	!is_contiguous_iterator<BidirIt>::value,
	bool
>::type
_regex_match_impl(
	BidirIt first, BidirIt last,
	match_results<BidirIt, Alloc>& m,
	const basic_regex<CharT, Traits>& e,
	regex_constants::match_flag_type flags,
	OnigRegex reg,
	OnigOptionType onig_options,
	size_type len)
{
	// Copy the subject range into a temporary contiguous buffer to support
	// non-contiguous BidirectionalIterators (e.g., std::list, std::deque)
	std::basic_string<CharT> subject_buf(first, last);

	// Use stable static buffer for empty ranges to avoid passing nullptr to C API
	static thread_local CharT empty_char = CharT();
	const CharT* start_ptr = (len > 0) ? subject_buf.c_str() : &empty_char;
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
		
		// Check if nosubs flag is set on the regex - if so, don't populate submatches
		if (e.flags() & regex_constants::nosubs) {
			// nosubs: only indicate match success, don't store submatches
			onig_region_free(region, 1);
			return true;
		}
		
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
		throw regex_error(regex_constants::map_oniguruma_error(r), einfo);
	}
}

// Internal implementation for contiguous iterators (optimized, no buffer copy)
template <class BidirIt, class Alloc, class CharT, class Traits>
typename std::enable_if<
	is_contiguous_iterator<BidirIt>::value,
	bool
>::type
_regex_match_impl(
	BidirIt first, BidirIt last,
	match_results<BidirIt, Alloc>& m,
	const basic_regex<CharT, Traits>& e,
	regex_constants::match_flag_type flags,
	OnigRegex reg,
	OnigOptionType onig_options,
	size_type len)
{
	// Fast path: use direct pointer access for contiguous iterators
	// Use stable static buffer for empty ranges to avoid passing nullptr to C API
	static thread_local CharT empty_char = CharT();
	const CharT* start_ptr = (len > 0) ? get_contiguous_pointer(first) : &empty_char;
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
		
		// Check if nosubs flag is set on the regex - if so, don't populate submatches
		if (e.flags() & regex_constants::nosubs) {
			// nosubs: only indicate match success, don't store submatches
			onig_region_free(region, 1);
			return true;
		}
		
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
		throw regex_error(regex_constants::map_oniguruma_error(r), einfo);
	}
}

// Public wrapper that dispatches to the appropriate implementation
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
	size_type len = std::distance(first, last);

	// Dispatch to the appropriate implementation based on iterator type
	return _regex_match_impl(first, last, m, e, flags, reg, onig_options, len);
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
	using iterator_t = regex_iterator<BidirIt, CharT, Traits>;

	BidirIt cur = first;
	bool first_only = (flags & regex_constants::format_first_only) != 0;
	bool no_copy = (flags & regex_constants::format_no_copy) != 0;
	bool literal = (flags & regex_constants::format_literal) != 0;

	// Use regex_iterator to enumerate matches (it already handles zero-width advancement)
	for (iterator_t it(first, last, e, flags), end; it != end; ++it) {
		const auto& m = *it; // match_results<BidirIt>
		// copy text from cur to match start
		if (!no_copy) {
			std::copy(cur, m[0].first, out);
		}

		// produce replacement for this match
		if (literal) {
			std::copy(fmt.begin(), fmt.end(), out);
		} else {
			for (size_type i = 0; i < fmt.size(); ++i) {
				CharT c = fmt[i];
				if (c == CharT('$') && i + 1 < fmt.size()) {
					CharT nx = fmt[i + 1];
					if (nx == CharT('$')) {
						*out++ = CharT('$'); ++i;
					} else if (nx == CharT('&')) {
						std::copy(m[0].first, m[0].second, out); ++i;
					} else if (nx == CharT('`')) {
						std::copy(first, m[0].first, out); ++i;
					} else if (nx == CharT('\'')) {
						std::copy(m[0].second, last, out); ++i;
					} else if (std::isdigit(static_cast<unsigned char>(nx))) {
						int num = 0;
						size_type j = i + 1;
						while (j < fmt.size() && std::isdigit(static_cast<unsigned char>(fmt[j]))) {
							num = num * 10 + (fmt[j] - CharT('0'));
							++j;
						}
						if (num >= 0 && static_cast<size_type>(num) < m.size()) {
							std::copy(m[num].first, m[num].second, out);
						}
						i = j - 1;
					} else {
						*out++ = CharT('$');
					}
				} else {
					*out++ = c;
				}
			}
		}

		// move cur to end of matched region
		cur = m[0].second;

		// If first_only requested, copy rest and finish
		if (first_only) {
			if (!no_copy) {
				std::copy(cur, last, out);
			}
			return out;
		}
	}

	// No more matches: copy tail if required
	if (!no_copy) {
		std::copy(cur, last, out);
	}
	return out;
}

template <class OutputIt, class BidirIt, class CharT, class Traits>
OutputIt regex_replace(
	OutputIt out,
	BidirIt first, BidirIt last,
	const basic_regex<CharT, Traits>& e,
	const CharT* fmt,
	regex_constants::match_flag_type flags)
{
	// Convert the C-string to a basic_string and call the other overload
	return regex_replace(out, first, last, e, basic_string<CharT>(fmt), flags);
}

////////////////////////////////////////////
// Implementation of regex_iterator

template <class BidirIt, class CharT, class Traits>
void regex_iterator<BidirIt, CharT, Traits>::do_search(BidirIt first, BidirIt last) {
	// If no match found, or end iterator reached
	if (first == last || !_regex_search_with_context(m_begin, first, last, m_results, *m_regex, m_flags)) {
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
	: m_begin(first), m_end(last), m_regex(&re), m_flags(flags)
{
	// Execute the first search
	do_search(first, last);
}

template <class BidirIt, class CharT, class Traits>
bool regex_iterator<BidirIt, CharT, Traits>::operator==(const regex_iterator& other) const {
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

	// Zero-width match handling
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

// Convenience constructor with std::initializer_list
template <class BidirIt, class CharT, class Traits>
regex_token_iterator<BidirIt, CharT, Traits>::regex_token_iterator(
	BidirIt first, BidirIt last,
	const regex_type& re,
	std::initializer_list<int> subs,
	match_flag_type flags)
	: regex_token_iterator(first, last, re, std::vector<int>(subs), flags)
{
	// Delegate to vector constructor
}

// Convenience constructor with single int
template <class BidirIt, class CharT, class Traits>
regex_token_iterator<BidirIt, CharT, Traits>::regex_token_iterator(
	BidirIt first, BidirIt last,
	const regex_type& re,
	int sub,
	match_flag_type flags)
	: regex_token_iterator(first, last, re, std::vector<int>{sub}, flags)
{
	// Delegate to vector constructor
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

////////////////////////////////////////////
// onigpp::init

int init(const OnigEncoding *encodings, size_type encodings_count) {
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

void uninit() { onig_end(); }

////////////////////////////////////////////
// onigpp::version

const char* version() { return onig_version(); }

// -------------------- Explicit template instantiations --------------------
// Instantiates for: char, wchar_t, char16_t, char32_t
// ---------------------------------------------------------------------------

// Helper aliases to reduce verbosity
using s_iter   = std::basic_string<char>::const_iterator;
using ws_iter  = std::basic_string<wchar_t>::const_iterator;
using u16_iter = std::basic_string<char16_t>::const_iterator;
using u32_iter = std::basic_string<char32_t>::const_iterator;

using s_sub_alloc   = std::allocator< sub_match<s_iter> >;
using ws_sub_alloc  = std::allocator< sub_match<ws_iter> >;
using u16_sub_alloc = std::allocator< sub_match<u16_iter> >;
using u32_sub_alloc = std::allocator< sub_match<u32_iter> >;

// basic_regex instantiations
template class basic_regex<char, regex_traits<char>>;
template class basic_regex<wchar_t, regex_traits<wchar_t>>;
template class basic_regex<char16_t, regex_traits<char16_t>>;
template class basic_regex<char32_t, regex_traits<char32_t>>;

// regex_iterator instantiations
template class regex_iterator<s_iter, char, regex_traits<char>>;
template class regex_iterator<ws_iter, wchar_t, regex_traits<wchar_t>>;
template class regex_iterator<u16_iter, char16_t, regex_traits<char16_t>>;
template class regex_iterator<u32_iter, char32_t, regex_traits<char32_t>>;

// regex_token_iterator instantiations
template class regex_token_iterator<s_iter, char, regex_traits<char>>;
template class regex_token_iterator<ws_iter, wchar_t, regex_traits<wchar_t>>;
template class regex_token_iterator<u16_iter, char16_t, regex_traits<char16_t>>;
template class regex_token_iterator<u32_iter, char32_t, regex_traits<char32_t>>;

// match_results is a template alias-like type used in function templates;
// we explicitly instantiate function templates with allocator types used above.

// regex_search instantiations
template bool regex_search<s_iter, s_sub_alloc, char, regex_traits<char>>(
	s_iter, s_iter, match_results<s_iter, s_sub_alloc>&, const basic_regex<char, regex_traits<char>>&, regex_constants::match_flag_type);

template bool regex_search<ws_iter, ws_sub_alloc, wchar_t, regex_traits<wchar_t>>(
	ws_iter, ws_iter, match_results<ws_iter, ws_sub_alloc>&, const basic_regex<wchar_t, regex_traits<wchar_t>>&, regex_constants::match_flag_type);

template bool regex_search<u16_iter, u16_sub_alloc, char16_t, regex_traits<char16_t>>(
	u16_iter, u16_iter, match_results<u16_iter, u16_sub_alloc>&, const basic_regex<char16_t, regex_traits<char16_t>>&, regex_constants::match_flag_type);

template bool regex_search<u32_iter, u32_sub_alloc, char32_t, regex_traits<char32_t>>(
	u32_iter, u32_iter, match_results<u32_iter, u32_sub_alloc>&, const basic_regex<char32_t, regex_traits<char32_t>>&, regex_constants::match_flag_type);

// regex_match instantiations
template bool regex_match<s_iter, s_sub_alloc, char, regex_traits<char>>(
	s_iter, s_iter, match_results<s_iter, s_sub_alloc>&, const basic_regex<char, regex_traits<char>>&, regex_constants::match_flag_type);

template bool regex_match<ws_iter, ws_sub_alloc, wchar_t, regex_traits<wchar_t>>(
	ws_iter, ws_iter, match_results<ws_iter, ws_sub_alloc>&, const basic_regex<wchar_t, regex_traits<wchar_t>>&, regex_constants::match_flag_type);

template bool regex_match<u16_iter, u16_sub_alloc, char16_t, regex_traits<char16_t>>(
	u16_iter, u16_iter, match_results<u16_iter, u16_sub_alloc>&, const basic_regex<char16_t, regex_traits<char16_t>>&, regex_constants::match_flag_type);

template bool regex_match<u32_iter, u32_sub_alloc, char32_t, regex_traits<char32_t>>(
	u32_iter, u32_iter, match_results<u32_iter, u32_sub_alloc>&, const basic_regex<char32_t, regex_traits<char32_t>>&, regex_constants::match_flag_type);

// regex_replace instantiations (OutputIt = back_insert_iterator<std::basic_string<CharT>>)
template std::back_insert_iterator<std::basic_string<char>> regex_replace<
	std::back_insert_iterator<std::basic_string<char>>, s_iter, char, regex_traits<char>>(
	std::back_insert_iterator<std::basic_string<char>>, s_iter, s_iter,
	const basic_regex<char, regex_traits<char>>&,
	const basic_string<char>&, regex_constants::match_flag_type);

template std::back_insert_iterator<std::basic_string<wchar_t>> regex_replace<
	std::back_insert_iterator<std::basic_string<wchar_t>>, ws_iter, wchar_t, regex_traits<wchar_t>>(
	std::back_insert_iterator<std::basic_string<wchar_t>>, ws_iter, ws_iter,
	const basic_regex<wchar_t, regex_traits<wchar_t>>&,
	const basic_string<wchar_t>&, regex_constants::match_flag_type);

template std::back_insert_iterator<std::basic_string<char16_t>> regex_replace<
	std::back_insert_iterator<std::basic_string<char16_t>>, u16_iter, char16_t, regex_traits<char16_t>>(
	std::back_insert_iterator<std::basic_string<char16_t>>, u16_iter, u16_iter,
	const basic_regex<char16_t, regex_traits<char16_t>>&,
	const basic_string<char16_t>&, regex_constants::match_flag_type);

template std::back_insert_iterator<std::basic_string<char32_t>> regex_replace<
	std::back_insert_iterator<std::basic_string<char32_t>>, u32_iter, char32_t, regex_traits<char32_t>>(
	std::back_insert_iterator<std::basic_string<char32_t>>, u32_iter, u32_iter,
	const basic_regex<char32_t, regex_traits<char32_t>>&,
	const basic_string<char32_t>&, regex_constants::match_flag_type);

// regex_replace instantiations with const CharT* format parameter
template std::back_insert_iterator<std::basic_string<char>> regex_replace<
	std::back_insert_iterator<std::basic_string<char>>, s_iter, char, regex_traits<char>>(
	std::back_insert_iterator<std::basic_string<char>>, s_iter, s_iter,
	const basic_regex<char, regex_traits<char>>&,
	const char*, regex_constants::match_flag_type);

template std::back_insert_iterator<std::basic_string<wchar_t>> regex_replace<
	std::back_insert_iterator<std::basic_string<wchar_t>>, ws_iter, wchar_t, regex_traits<wchar_t>>(
	std::back_insert_iterator<std::basic_string<wchar_t>>, ws_iter, ws_iter,
	const basic_regex<wchar_t, regex_traits<wchar_t>>&,
	const wchar_t*, regex_constants::match_flag_type);

template std::back_insert_iterator<std::basic_string<char16_t>> regex_replace<
	std::back_insert_iterator<std::basic_string<char16_t>>, u16_iter, char16_t, regex_traits<char16_t>>(
	std::back_insert_iterator<std::basic_string<char16_t>>, u16_iter, u16_iter,
	const basic_regex<char16_t, regex_traits<char16_t>>&,
	const char16_t*, regex_constants::match_flag_type);

template std::back_insert_iterator<std::basic_string<char32_t>> regex_replace<
	std::back_insert_iterator<std::basic_string<char32_t>>, u32_iter, char32_t, regex_traits<char32_t>>(
	std::back_insert_iterator<std::basic_string<char32_t>>, u32_iter, u32_iter,
	const basic_regex<char32_t, regex_traits<char32_t>>&,
	const char32_t*, regex_constants::match_flag_type);

} // namespace onigpp

// -------------------- Explicit instantiations for non-contiguous iterators --------------------
// Add explicit instantiations for std::list and std::deque to support non-contiguous containers
// ---------------------------------------------------------------------------

#include <list>
#include <deque>
#include <vector>

namespace onigpp {

// Aliases for pointer types (const char*)
using cchar_ptr = const char*;
using cchar_ptr_sub_alloc = ::std::allocator<sub_match<cchar_ptr>>;

// Aliases for std::list iterators
using list_char_iter = ::std::list<char>::iterator;
using list_char_sub_alloc = ::std::allocator<sub_match<list_char_iter>>;

using list_char_const_iter = ::std::list<char>::const_iterator;
using list_char_const_sub_alloc = ::std::allocator<sub_match<list_char_const_iter>>;

// Aliases for std::deque iterators
using deque_char_iter = ::std::deque<char>::iterator;
using deque_char_sub_alloc = ::std::allocator<sub_match<deque_char_iter>>;

// Aliases for std::vector iterators (non-const)
using vector_char_iter = ::std::vector<char>::iterator;
using vector_char_sub_alloc = ::std::allocator<sub_match<vector_char_iter>>;

// regex_search instantiations for const char* (pointer type - optimized)
template bool regex_search<cchar_ptr, cchar_ptr_sub_alloc, char, regex_traits<char>>(
	cchar_ptr, cchar_ptr, match_results<cchar_ptr, cchar_ptr_sub_alloc>&, 
	const basic_regex<char, regex_traits<char>>&, regex_constants::match_flag_type);

// regex_search instantiations for std::list<char>::iterator
template bool regex_search<list_char_iter, list_char_sub_alloc, char, regex_traits<char>>(
	list_char_iter, list_char_iter, match_results<list_char_iter, list_char_sub_alloc>&, 
	const basic_regex<char, regex_traits<char>>&, regex_constants::match_flag_type);

// regex_search instantiations for std::list<char>::const_iterator
template bool regex_search<list_char_const_iter, list_char_const_sub_alloc, char, regex_traits<char>>(
	list_char_const_iter, list_char_const_iter, match_results<list_char_const_iter, list_char_const_sub_alloc>&, 
	const basic_regex<char, regex_traits<char>>&, regex_constants::match_flag_type);

// regex_match instantiations for std::deque<char>::iterator
template bool regex_match<deque_char_iter, deque_char_sub_alloc, char, regex_traits<char>>(
	deque_char_iter, deque_char_iter, match_results<deque_char_iter, deque_char_sub_alloc>&, 
	const basic_regex<char, regex_traits<char>>&, regex_constants::match_flag_type);

// regex_search instantiations for std::deque<char>::iterator
template bool regex_search<deque_char_iter, deque_char_sub_alloc, char, regex_traits<char>>(
	deque_char_iter, deque_char_iter, match_results<deque_char_iter, deque_char_sub_alloc>&, 
	const basic_regex<char, regex_traits<char>>&, regex_constants::match_flag_type);

// regex_search instantiations for std::vector<char>::iterator (non-const)
template bool regex_search<vector_char_iter, vector_char_sub_alloc, char, regex_traits<char>>(
	vector_char_iter, vector_char_iter, match_results<vector_char_iter, vector_char_sub_alloc>&, 
	const basic_regex<char, regex_traits<char>>&, regex_constants::match_flag_type);

// regex_iterator instantiations for std::list<char>::iterator
template class regex_iterator<list_char_iter, char, regex_traits<char>>;

// _regex_search_with_context instantiation for std::list (needed by regex_iterator)
template bool _regex_search_with_context<list_char_iter, list_char_sub_alloc, char, regex_traits<char>>(
	list_char_iter, list_char_iter, list_char_iter, 
	match_results<list_char_iter, list_char_sub_alloc>&, 
	const basic_regex<char, regex_traits<char>>&, regex_constants::match_flag_type);

// _regex_search_with_context instantiation for const char* (optimized path)
template bool _regex_search_with_context<cchar_ptr, cchar_ptr_sub_alloc, char, regex_traits<char>>(
	cchar_ptr, cchar_ptr, cchar_ptr, 
	match_results<cchar_ptr, cchar_ptr_sub_alloc>&, 
	const basic_regex<char, regex_traits<char>>&, regex_constants::match_flag_type);

} // namespace onigpp
