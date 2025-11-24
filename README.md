[Japanese](README_ja.md) | [License](LICENSE.txt) | [Changelog](CHANGELOG.md)

---

# Oniguruma++

A C++11 wrapper around the Oniguruma regular expression engine that provides a std::regex-like interface. This library emphasizes ease of use and compatibility.

![CI](https://github.com/katahiromz/onigpp/actions/workflows/ci.yml/badge.svg)

## What's this?

This repository provides a wrapper to make Oniguruma easy to use from C++. It offers an API style similar to the standard library's <regex>, while exposing Oniguruma's powerful features.

## Features

- Access Oniguruma features with a std::regex-like interface
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
    namespace re = onigpp;
#else
    #include <regex>
    namespace re = std;
#endif

#include <iostream>
#include <string>

int main() {
    std::string s = "hello 123 world";
    re::regex r(R"(\d+)");
    re::smatch m;
    if (re::regex_search(s, m, r)) {
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

## API / Reference

- [RE.md](RE.md) — Oniguruma regular expression reference (English)
- [API.md](API.md) — Oniguruma C API reference (English)

## Header

- [onigpp.h](onigpp.h) — main header

## Platforms

- Windows (Visual Studio 2015 and later)
- Linux (Ubuntu, etc.)
- macOS

## License

- BSD-2-Clause (see LICENSE file for details)

## Contact / Support

- Maintainer: Katayama Hirofumi MZ <katayama.hirofumi.mz@gmail.com>
- Issues: https://github.com/katahiromz/onigpp/issues
