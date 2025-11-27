# Changelog

## 20XX-YY-ZZ Ver.6.9.16

- Added `format_literal` flag for `regex_replace` (similar to Boost.Regex):
  - When set, the replacement string is treated as literal text without any escape processing.
  - `$n`, `$&`, `\n`, etc. in the replacement string are kept as-is instead of being expanded.
  - Useful when the replacement string contains `$` or `\` characters that should not be interpreted.

## 2025-11-26 Ver.6.9.15

- Added missing `std::regex_traits` methods to `onigpp::regex_traits` for improved standard library compatibility:
  - `translate_nocase(char_type c)`: Returns the lowercase version of a character for case-insensitive matching.
  - `transform_primary(const char_type*, const char_type*)`: Returns a primary collation key (case-insensitive) for sorting.
  - `lookup_classname(const char_type*, const char_type*, bool icase = false)`: Looks up POSIX character class names and returns a `char_class_type` bitmask.
    - Supports: alnum, alpha, blank, cntrl, digit, graph, lower, print, punct, space, upper, xdigit, d, w, s
    - The 'w' class (word characters) correctly matches alphanumeric characters AND underscore.
    - When `icase=true`, lower/upper classes return alpha to match both cases.
  - All new methods work with char, wchar_t, char16_t, and char32_t types.
  - `isctype` for char16_t and char32_t now provides full ASCII character class support instead of returning false.
  - Added comprehensive tests in `regex_traits_test.cpp`.
- Refactored `regex_replace` to use `match_results::format` for replacement string processing:
  - Separated responsibilities: `regex_replace` handles matching iteration, `match_results::format` handles format expansion.
  - Added extended `match_results::format` overload with name resolver callback for named group support.
  - Added `${n}` safe numbered reference support to `match_results::format` (e.g., `${1}0` for "group 1 followed by literal 0").
  - Named groups (`${name}`, `\k<name>`, `\k'name'`) still work via the extended format API used by `regex_replace`.
  - Improved code maintainability and testability by consolidating format logic in `match_results::format`.
- Added `current_match_results()` method to `regex_token_iterator` as an extension API:
  - Returns a const reference to the underlying `match_results` for the current match.
  - Enables access to `prefix()`, `suffix()`, and all capture groups during token iteration.
  - Added `match_results_type` typedef to `regex_token_iterator` for type convenience.
  - Note: This is an extension to the standard C++ `regex_token_iterator` API.
  - Added comprehensive test suite (`regex_token_iterator_match_results_test.cpp`).
- Added comprehensive test suite for `regex_iterator` zero-width match handling:
  - `regex_iterator_zero_width_test.cpp` tests empty patterns, lookaheads, word boundaries, anchors, and optional patterns.
  - Verifies std::regex_iterator compatibility for consecutive zero-width matches at string boundaries.
  - Tests iterator equality and safe increment past end behavior.
- Added comprehensive test suite for exception handling (`regex_exception_test.cpp`):
  - Tests that invalid patterns (unclosed parentheses, brackets, leading quantifiers) throw `regex_error`.
  - Tests that valid operations (search, match, iterator) do not throw on no-match scenarios.
  - Verifies exception behavior matches std::regex expectations.
- Added zero-width match test cases to `patterns.json` for compatibility testing:
  - Empty pattern search and replace
  - Lookahead, word boundary, and anchor search
  - Optional and star pattern zero-width matches
  - Word boundary replacement at word edges
- Enhanced `sub_match` class for full `std::sub_match` compatibility:
  - Added `compare()` member function for comparing with other `sub_match`, `string_type`, and C-strings.
  - Added comparison operators (`==`, `!=`, `<`, `<=`, `>`, `>=`) for:
    - `sub_match` vs `sub_match`
    - `sub_match` vs `string_type` (and vice versa)
    - `sub_match` vs `const value_type*` C-strings (and vice versa)
  - Added stream output operator (`operator<<`) for inserting `sub_match` contents to `std::basic_ostream`.
  - All comparison operations use `str()` semantics: unmatched sub_matches compare as empty strings.
  - Added comprehensive test suite in `sub_match_compat_test.cpp` for new functionality.
- Added `match_results::ready()` member function to check if match results have been populated.
  - Returns `true` after a successful or unsuccessful `regex_match` or `regex_search` operation.
  - Returns `false` for default-constructed `match_results`.
  - This matches `std::match_results::ready()` semantics for compatibility.
- Added `match_results::format` member function for template-based string formatting using sub-matches.
  - Supports `$n` and `$nn` for numbered capture groups (e.g., `$0`, `$1`, `$12`)
  - Supports `$&` for full match (equivalent to `$0`)
  - Supports `$`` for prefix (text before match)
  - Supports `$'` for suffix (text after match)
  - Supports `$$` for literal `$`
  - Supports escape sequences: `\n`, `\t`, `\r`, `\\`
  - Unmatched submatches are replaced with an empty string
  - Compatible with `std::match_results::format` interface
- Added `RE_ja.md` — Japanese translation of `RE.md` (Oniguruma++ regular expression specification).
- Added `dialog`.
- Added `empty_pattern_test` to verify empty regex pattern compatibility with std::regex.
- Fixed `regex_iterator` to correctly handle zero-width pattern matching at all positions including the end of string.
  - Zero-width patterns (e.g., empty patterns, lookaheads) now correctly match at the end position.
  - This matches std::regex behavior where empty patterns produce N+1 matches for a string of length N.
- Deleted `regex_constants::oniguruma`.
- Restored `regex_constants::oniguruma` flag to enable Oniguruma's native syntax and behavior.
- Added MSYS2 CI support (MINGW64/MINGW32) to GitHub Actions workflow.
- Added named reference expansion in replacement strings for `regex_replace`:
  - `${name}` syntax: Works in both ECMAScript and oniguruma modes
  - `\k<name>` syntax: Works only when `oniguruma` flag is set
  - `\k'name'` syntax: Works only when `oniguruma` flag is set
  - This enables users to reference named capture groups in replacement strings
- Fixed `${n}` safe numbered reference in replacement strings to correctly expand to capture group contents:
  - `${1}`, `${2}`, etc. now correctly expand to the corresponding capture group
  - This allows safe disambiguation when a number is followed by more digits (e.g., `${1}0` expands to group 1 followed by literal "0")
- Refactored `src/onigpp.cpp` to consolidate duplicated OnigRegion processing logic:
  - Added `_adjust_region_offsets_prefix()` helper function for region offset adjustment when handling match_not_bow prefix.
  - Added `_onig_region_to_match_results()` helper function for regex_match implementations (with full-match checking).
  - Reduced code duplication in `_regex_search_with_context_impl` and `_regex_match_impl` SFINAE variants.
  - Improved maintainability and readability of match result processing code.
- Enhanced `match_results` class for improved `std::match_results` compatibility:
  - Added `allocator_type` type alias.
  - Added `get_allocator()` member function to retrieve the allocator.
  - Added `swap()` member function for exchanging contents with another `match_results`.
  - Added non-member `swap()` function for `match_results`.
  - Added comparison operators: `operator==` and `operator!=`.
  - Added explicit default, copy, and move constructors.
  - Added constructor accepting an allocator parameter.
  - Added `noexcept` specifications to `empty()`, `swap()`, and `get_allocator()`.
  - Added comprehensive test suite (`match_results_compat_test.cpp`) for new functionality.

## 2025-11-25 Ver.6.9.14

- Renamed SHIFT_JIS as SJIS.
- Added `supported_encodings.h` to toggle the supported encodings.

## 2025-11-25 Ver.6.9.13

- Added support for `match_not_bow`, `match_not_eow`, and `match_continuous` match flags in C++ wrapper.
  - `match_not_bow`: First position is not treated as beginning-of-word for word boundary assertions.
  - `match_not_eow`: Last position is not treated as end-of-word for word boundary assertions.
  - `match_continuous`: Match must start exactly at the search position (uses `onig_match` instead of `onig_search`).
- Added compatibility tests for the new match flags in `tests/compat/patterns.json`.
- Updated `compat_test.cpp` to support `match_flags` field in test cases.
- Added tools/re_replace.
- Documented ECMAScript as the default grammar when no flags are specified (matching `std::regex` behavior).
- Added nested flag constants to `basic_regex` class (e.g., `onigpp::regex::ECMAScript`, `onigpp::regex::icase`) for std::regex compatibility.
- Added `oniguruma` flag to explicitly enable/document Oniguruma-style backreference support.
  - Numeric backreferences: `\1`, `\2`, ..., `\9` (and multi-digit like `\10` when 10+ groups defined)
  - Named backreferences: `\k<name>`, `\k'name'`
  - Warning: Backreferences can introduce exponential-time backtracking for certain patterns.
- Added replacement-side Oniguruma-style backreference support when `oniguruma` flag is set:
  - Whole match: `\0` (equivalent to `$&`)
  - Numeric replacement backreferences: `\1`, `\2`, ... (in addition to existing `$1`, `$2`, ...)
  - Literal backslash: `\\` produces a single backslash
  - Multi-digit backreferences: `\10`, `\11`, ... when 10+ capture groups defined
  - Note: Only enabled when regex is constructed with `oniguruma` flag
- Added `regex_access::get_flags` helper to read flags from `basic_regex`.
- Added comprehensive backreference tests (`backref_test.cpp`) covering:
  - Basic numeric and named backreferences
  - Case-insensitive backreferences
  - Replacement-side capture expansion (`$1`, `$2`, etc.)
  - Replacement-side Oniguruma-style backreferences (`\1`, `\2`, etc.)
  - Ambiguous digit resolution (octal vs backreference)

## 2025-11-24 Ver.6.9.12

- Bumped project version to 6.9.12.
- Minor bug fixes and test improvements.
- Documentation updates (version headers in RE/RE.txt/RE_ja.txt).
- Build configuration: update project version in CMakeLists.
- No API-breaking changes; compatibility maintained.

## 2025-11-23 Ver.6.9.11

- First release.
