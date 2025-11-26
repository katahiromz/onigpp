[Japanese](README_ja.md) | [License](LICENSE.txt) | [Changelog](CHANGELOG.md)

---

# Oniguruma++

![CI](https://github.com/katahiromz/onigpp/actions/workflows/ci.yml/badge.svg)

## What's this?

A C++11 wrapper around the Oniguruma regular expression engine that provides a `std::regex`-like interface. This library emphasizes ease of use and compatibility.

It offers an API style similar to the standard library's `<regex>`, while exposing Oniguruma's powerful features.

## Features

- Access Oniguruma features with a `std::regex`-like interface
- Easy build using CMake
- Compatibility tests against the standard library implementation (CI tests multiple configurations)

## Requirements

- C++11-compatible compiler (Visual Studio 2015+, GCC, Clang, etc.)
- CMake >= 3.10 (required by the build scripts)
- git (for submodule support)

## Supported platforms / Compilers

This project is intended to work with modern C++11-compatible compilers (MSVC 2015+, GCC 5+, Clang 3.8+). CI tests commonly include Windows, Linux, and recent macOS toolchains. If you rely on a different toolchain, please open an issue or test locally.

### MSYS2 Support

CI also tests builds under MSYS2 on Windows with both MINGW64 (x86_64) and MINGW32 (i686) environments using the MinGW-w64 toolchain.

## Quick start (usage)

```cpp
#define USE_ONIGPP // toggle between onigpp and std (for example/demo only)

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

> **Note:** The `#define USE_ONIGPP` toggle in the example above is only for demonstration purposes to show how code can switch between onigpp and `std::regex`. In actual projects, simply include `onigpp.h` and link to the onigpp library.

## Build

Recommended steps (building from source):

```bash
git clone https://github.com/katahiromz/onigpp
cd onigpp
git submodule update --init --recursive
```

**Out-of-source build (portable style):**

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

**CMake shorthand (CMake 3.13+):**

```bash
cmake -S . -B build
cmake --build build
```

Common CMake options:
- `-DCMAKE_BUILD_TYPE=Release` or `-DCMAKE_BUILD_TYPE=Debug`
- `-DUSE_STD_FOR_TESTS=ON` or `-DUSE_STD_FOR_TESTS=OFF` (useful to run the same compatibility checks locally as CI)
- `-DBUILD_SHARED_LIBS=ON` or `-DBUILD_SHARED_LIBS=OFF`

On Windows (MSVC), specify the Visual Studio generator when running cmake.

## Testing

Run the project's tests:

```bash
cmake --build build --target test
```

Or run CTest directly from the build directory:

```bash
cd build
ctest --output-on-failure
```

CI runs tests with both `USE_STD_FOR_TESTS=ON` and `OFF` to verify compatibility.

### Using `USE_STD_FOR_TESTS`

The `USE_STD_FOR_TESTS` CMake option allows you to run the compatibility tests using `std::regex` instead of onigpp. This is useful for verifying that the test patterns themselves are valid and that onigpp behaves consistently with the standard library.

```bash
# Build with USE_STD_FOR_TESTS enabled
cmake -S . -B build -DUSE_STD_FOR_TESTS=ON
cmake --build build

# Run the ecmascript compatibility test
./build/ecmascript_compat_test
```

## Migration from std::regex

Onigpp provides an ECMAScript compatibility mode to make migrating from `std::regex` easier. Here's what you need to know:

### Default Grammar

**ECMAScript is the default grammar** when no flags are specified, matching the behavior of `std::regex`. This means:

```cpp
#include "onigpp.h"
namespace rex = onigpp;
rex::auto_init g_auto_init;

// Both of these are equivalent - ECMAScript is the default
rex::regex r1(R"(\d+)");                           // default: ECMAScript
rex::regex r2(R"(\d+)", rex::regex::ECMAScript);   // explicit: ECMAScript
```

### Using ECMAScript Mode

Since ECMAScript is the default, you typically don't need to specify it explicitly:

```cpp
#include "onigpp.h"
namespace rex = onigpp;
rex::auto_init g_auto_init;

// ECMAScript mode (default, same as std::regex)
rex::regex pattern(R"(\d+)");
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
   - The `multiline` flag emulates ECMAScript semantics when combined with `ECMAScript` mode
   - When both `ECMAScript` and `multiline` flags are set, onigpp rewrites the pattern at compile time to make `^` and `$` match at line boundaries. The recognized line separators are: `\n`, `\r`, `\r\n`, U+2028 (Line Separator), and U+2029 (Paragraph Separator)
   - This emulation preserves ECMAScript semantics: dot (`.`) still does NOT match newlines (controlled separately by a potential future dotall flag)
   - **Performance note**: Pattern rewriting adds a small CPU cost at regex construction time, but has no runtime matching overhead
   - **Limitations**: The rewrite handles common cases (unescaped `^` and `$` outside character classes). Complex or unusual patterns may have edge cases; please report issues if you find any

2. **Named Captures**:
   - Oniguruma uses `(?<name>...)` syntax
   - ECMAScript also supports this syntax

3. **Lookahead/Lookbehind**:
   - Both positive and negative lookahead are supported
   - Lookbehind is also available (an extension beyond basic ECMAScript)

### Oniguruma flags and options

onigpp exposes a set of syntax and compilation flags that let you control matching behavior and compilation options. These are available as enum flags on the wrapper (use bitwise OR to combine). Commonly used flags include:

- `ECMAScript` (default): Use ECMAScript grammar (same as `std::regex` default).
- `icase`: Case-insensitive matching (similar to `std::regex_constants::icase`).
- `nosubs`: Do not store submatch results (can be used when only a boolean match is needed).
- `multiline`: Emulate ECMAScript multiline behavior so that `^` and `$` match at line boundaries (see "Multiline Mode").
- `oniguruma`: Enable Oniguruma's native syntax and behavior. When this flag is set, Oniguruma's default regex syntax is used instead of ECMAScript.
- `optimize` / compilation hints: Some wrappers provide hints to request optimized compilation; behavior may be implementation-specific.

Usage example:

```cpp
#include "onigpp.h"
namespace rex = onigpp;
rex::auto_init g_auto_init;

// Combine flags with bitwise OR:
rex::regex r(R"(^line\d+)", rex::regex::ECMAScript | rex::regex::multiline | rex::regex::icase);
```

Notes:
- The exact set and names of flags exposed by the onigpp wrapper are defined in `onigpp.h`. For advanced, engine-specific Oniguruma options and encoding-related settings, consult `onigpp.h` and the Oniguruma documentation.
- Flags affect how patterns are compiled and matched; some Oniguruma-specific behaviors may not have direct equivalents in `std::regex`.

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

## Contributing

Contributions are welcome! Here's how to get started:

1. **Run tests before submitting**: Ensure all tests pass locally.
   ```bash
   cmake -S . -B build
   cmake --build build
   cd build && ctest --output-on-failure
   ```

2. **Run compatibility tests**: The project includes a compatibility harness to verify behavior against `std::regex`.
   ```bash
   cmake -S . -B build -DUSE_STD_FOR_TESTS=ON
   cmake --build build
   cd build && ctest --output-on-failure
   ```

3. **General guidelines**:
   - Keep changes focused and minimal
   - Update `CHANGELOG.md` if adding significant features or fixes
   - Add or update tests for new functionality

## License

- BSD-2-Clause (see LICENSE file for details)

## Contact / Support

- Maintainer: Katayama Hirofumi MZ <katayama.hirofumi.mz@gmail.com>
- Issues: https://github.com/katahiromz/onigpp/issues
