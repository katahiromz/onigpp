# Compatibility Test Harness

This directory contains a minimal test harness for automatically comparing the compatibility between `onigpp` and `std::regex`.

## Purpose

The goal is to automatically detect differences in behavior between onigpp and std::regex by:
1. Running the same regex patterns and operations on both implementations
2. Comparing match results, capture groups, positions, and replacement outputs
3. Reporting detailed differences to help identify compatibility issues

## Files

- **patterns.json**: Test case definitions in JSON format
- **compat_test.cpp**: Test harness implementation
- **CMakeLists.txt**: Build configuration
- **README.md**: This file

## Test Case Format

Each test case in `patterns.json` contains:
- `id`: Unique identifier for the test
- `description`: Human-readable description
- `pattern`: Regular expression pattern to test
- `flags`: Syntax option flags (e.g., "ECMAScript", "icase", "nosubs")
- `input`: Input string to test against
- `operation`: Type of operation ("match", "search", or "replace")
- `replace_template`: (Optional) Replacement template for "replace" operations
- `encoding_hint`: (Optional) Encoding hint ("UTF-8", "SJIS", "UTF-16LE")

## Building

The compat tests are automatically built as part of the main project build:

```bash
mkdir build
cd build
cmake ..
make
```

## Running

### Run via CMake/CTest

```bash
cd build
ctest -R compat_test --verbose
```

### Run directly

```bash
cd build
./compat_test
```

Or specify a custom patterns.json path:

```bash
./compat_test path/to/patterns.json
```

## Interpreting Results

Each test case will output:
- ✅ **PASS**: Both implementations produced identical results
- ❌ **FAIL**: Implementations produced different results (differences are shown)
- ⚠️ **SKIP**: Both implementations threw exceptions (not considered a failure)

For failed tests, the output shows:
- Which aspect differed (match result, capture count, capture content, position/length)
- The value from std::regex
- The value from onigpp

## Test Cases

The initial test suite includes 12 representative cases:

1. **simple_match**: Basic exact match with anchors
2. **partial_match_fail**: Match that should fail with extra text
3. **search_basic**: Simple substring search
4. **capture_groups**: Capture group extraction
5. **zero_width_boundary**: Word boundary assertions
6. **replace_dollar_refs**: Replacement with $ references
7. **nosubs_flag**: nosubs flag verification
8. **icase_flag**: Case-insensitive matching
9. **utf8_multibyte**: UTF-8 multibyte character support
10. **sjis_bytes**: SJIS encoding support
11. **lookahead**: Lookahead assertions
12. **backreference**: Backreference support

## Adding New Tests

To add new test cases:

1. Edit `patterns.json`
2. Add a new object with the required fields
3. Rebuild and run the tests

## Notes

- This harness focuses on **detection** of differences, not fixing them
- Compatibility improvements to the onigpp library should be handled in separate PRs
- The JSON parser is minimal and purpose-built; it can be replaced with a full JSON library if needed
- Some differences are expected due to implementation-specific features or limitations

## Related Tests

In addition to this compat test harness, the following focused unit tests validate specific semantics and edge cases:

- **sub_match_behavior_test**: Comprehensive test suite for `match_results` and `sub_match` behavior
  - Tests `position()` and `length()` for matched/unmatched groups and out-of-range indices
  - Tests `prefix()` and `suffix()` behavior for empty and non-empty match results
  - Tests `sub_match::str()` and implicit string conversion for matched and unmatched submatches
  - Tests nosubs flag behavior (constructor flag honored, size=1, no capture groups stored)
  - Tests with non-random-access iterators (std::list) to ensure position/length correctness
  - Validates npos sentinel value (-1) semantics matching std::match_results

These tests ensure that onigpp maintains std::regex-compatible semantics for match results and submatches across various scenarios and iterator categories.
