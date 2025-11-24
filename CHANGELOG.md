# Changelog

## 2025-XX-YY Ver.6.9.13

- Added tools/re_replace.
- Added nested flag constants to `basic_regex` class (e.g., `onigpp::regex::ECMAScript`, `onigpp::regex::icase`) for std::regex compatibility.
- Added `oniguruma` flag to explicitly enable/document Oniguruma-style backreference support.
  - Numeric backreferences: `\1`, `\2`, ..., `\9` (and multi-digit like `\10` when 10+ groups defined)
  - Named backreferences: `\k<name>`, `\k'name'`
  - Warning: Backreferences can introduce exponential-time backtracking for certain patterns.
- Added comprehensive backreference tests (`backref_test.cpp`) covering:
  - Basic numeric and named backreferences
  - Case-insensitive backreferences
  - Replacement-side capture expansion (`$1`, `$2`, etc.)
  - Ambiguous digit resolution (octal vs backreference)

## 2025-11-24 Ver.6.9.12

- Bumped project version to 6.9.12.
- Minor bug fixes and test improvements.
- Documentation updates (version headers in RE/RE.txt/RE_ja.txt).
- Build configuration: update project version in CMakeLists.
- No API-breaking changes; compatibility maintained.

## 2025-11-23 Ver.6.9.11

- First release.
