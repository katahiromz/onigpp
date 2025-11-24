[Japanese](README_ja.md) | [License](LICENSE.txt) | [Changelog](CHANGELOG.md)

---

# Oniguruma++

A C++11 wrapper around the Oniguruma regular expression engine that provides a `std::regex`-like interface. This library emphasizes ease of use and compatibility.

![CI](https://github.com/katahiromz/onigpp/actions/workflows/ci.yml/badge.svg)

## What's this?

This repository provides a wrapper to make Oniguruma easy to use from C++. It offers an API style similar to the standard library's `<regex>`, while exposing Oniguruma's powerful features.

## Features

- Access Oniguruma features with a `std::regex`-like interface
- Easy build using CMake
- Compatibility tests against the standard library implementation (CI tests multiple configurations)

## Requirements

- C++11-compatible compiler (Visual Studio 2015+, GCC, Clang, etc.)
- CMake 3.x
- git (for submodule support)

## Quick start (usage)

```cpp
#define USE_ONIGPP // toggle between onigpp and std

#ifdef USE_ONIGPP
    #include "onigpp.h"
    namespace rex = onigpp;
    rex::auto_init g_auto_init;
#else
    #include <regex>
    namespace rex = std;
#endif

#include <iostream>
#include <string>

int main() {
    std::string s = "hello 123 world";
    rex::regex r(R"(\d+)");
    rex::smatch m;
    if (rex::regex_search(s, m, r)) {
        std::cout << "matched: " << m.str() << std::endl;
    }
    return 0;
}
```

## Build

Recommended steps (building from source):

```bash
git clone https://github.com/katahiromz/onigpp
cd onigpp
git submodule update --init --recursive

mkdir build
cmake -B build
cmake --build build
```

Common CMake options:
- `-DCMAKE_BUILD_TYPE=Release / Debug`
- `-DUSE_STD_FOR_TESTS=ON/OFF` (useful to run the same compatibility checks locally as CI)
- `-DBUILD_SHARED_LIBS=ON/OFF`

On Windows (MSVC), specify the Visual Studio generator when running cmake.

## Testing

Run the project's tests:

```bash
cmake --build build -t test
# or run CTest directly:
cd build
ctest --output-on-failure
```

CI runs tests with both `USE_STD_FOR_TESTS=ON` and `OFF` to verify compatibility.

## Migration from std::regex

Onigpp provides an ECMAScript compatibility mode to make migrating from `std::regex` easier. Here's what you need to know:

### Using ECMAScript Mode

To use ECMAScript-compatible syntax, specify the `ECMAScript` flag when creating a regex:

```cpp
#include "onigpp.h"
namespace rex = onigpp;
rex::auto_init g_auto_init;

// ECMAScript mode (similar to std::regex default)
rex::regex pattern(R"(\d+)", rex::regex::ECMAScript);
```

### What's Supported

The ECMAScript mode in onigpp provides:

1. **Escape Sequences**:
   - `\xHH` - Hexadecimal escape (e.g., `\x41` for 'A')
   - `\uHHHH` - Unicode escape (e.g., `\u00E9` for 'é')
   - `\0` - Null character (when not followed by digit)

2. **Dot Behavior**: 
   - By default, dot (`.`) does NOT match newline characters (same as std::regex)

3. **Replacement Templates**:
   - `$&` - Whole match
   - `$1`, `$2`, ... - Capture groups
   - `$`` - Text before match (prefix)
   - `$'` - Text after match (suffix)
   - `$$` - Literal dollar sign
   - `${name}` - Named capture groups

### Known Differences

Some features may behave differently from `std::regex`:

1. **Multiline Mode**: 
   - ✨ **NEW**: The `multiline` flag now emulates ECMAScript semantics when combined with `ECMAScript` mode
   - When both `ECMAScript` and `multiline` flags are set, onigpp rewrites the pattern at compile time to make `^` and `$` match at line boundaries (after/before `\n`, `\r`, `\r\n`, U+2028, U+2029) without affecting dot behavior
   - This emulation preserves ECMAScript semantics: dot (`.`) still does NOT match newlines (controlled separately by a potential future dotall flag)
   - **Performance note**: Pattern rewriting adds a small CPU cost at regex construction time, but has no runtime matching overhead
   - **Limitations**: The rewrite handles common cases (unescaped `^` and `$` outside character classes). Complex or unusual patterns may have edge cases; please report issues if you find any

2. **Named Captures**:
   - Oniguruma uses `(?<name>...)` syntax
   - ECMAScript also supports this syntax

3. **Lookahead/Lookbehind**:
   - Both positive and negative lookahead are supported
   - Lookbehind is also available (an extension beyond basic ECMAScript)

### Example Migration

**Before (std::regex):**
```cpp
#include <regex>
std::string text = "Price: $100";
std::regex rex(R"(\$(\d+))");
std::string result = std::regex_replace(text, rex, "$$$1.00");
// Result: "Price: $100.00"
```

**After (onigpp with ECMAScript mode):**
```cpp
#include "onigpp.h"
onigpp::auto_init init;
std::string text = "Price: $100";
onigpp::regex rex(R"(\$(\d+))", onigpp::regex::ECMAScript);
std::string result = onigpp::regex_replace(text, rex, "$$$1.00");
// Result: "Price: $100.00"
```

### ECMAScript Multiline Example

With the multiline emulation, `^` and `$` match at line boundaries:

```cpp
#include "onigpp.h"
onigpp::auto_init init;

std::string text = "line1\nline2\nline3";
onigpp::regex rex("^line\\d", onigpp::regex::ECMAScript | onigpp::regex::multiline);

// Find all lines starting with "line" followed by a digit
auto begin = onigpp::sregex_iterator(text.begin(), text.end(), rex);
auto end = onigpp::sregex_iterator();

for (auto it = begin; it != end; ++it) {
    std::cout << "Match: " << it->str() << "\n";
}
// Output:
// Match: line1
// Match: line2
// Match: line3
```

Note: Even with multiline mode, the dot (`.`) still does NOT match newlines, preserving ECMAScript semantics.

### Testing Your Migration

The test suite includes comprehensive ECMAScript compatibility tests. You can run them with:

```bash
cmake --build build --target ecmascript_compat_test
./build/ecmascript_compat_test
```

## API / Reference

- [RE.md](RE.md) — Oniguruma regular expression reference (English)
- [API.md](API.md) — Oniguruma C API reference (English)

## Header

- [onigpp.h](onigpp.h) — main header

## License

- BSD-2-Clause (see LICENSE file for details)

## Contact / Support

- Maintainer: Katayama Hirofumi MZ <katayama.hirofumi.mz@gmail.com>
- Issues: https://github.com/katahiromz/onigpp/issues
