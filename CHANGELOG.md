# Changelog

## 2025-XX-YY Ver.6.9.13

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
