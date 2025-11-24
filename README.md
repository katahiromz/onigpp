# Oniguruma++

## What's this?

This is a C++11 wrapper of the Oniguruma regular expression engine, with standard `<regex>` style.

## Regular expression reference

- [RE.md](RE.md) — Oniguruma Regular Expressions reference (English)

## Build

```bash
git clone https://github.com/katahiromz/onigpp
cd onigpp
git submodule update --init --recursive
mkdir build
cmake -B build
cmake --build build
```

## Testing

```bash
cmake --build build -t test
```

The CI automatically tests both `USE_STD_FOR_TESTS=ON` and `USE_STD_FOR_TESTS=OFF` configurations to ensure compatibility with both standard library and custom implementations.

## Header

- [onigpp.h](onigpp.h)

## Platforms

- Windows (Visual Studio 2015 and later)
- Linux (Ubuntu)
- macOS

## License

- BSD-2-Clause

## API reference

- [API.md](API.md) — Oniguruma C API reference (English)

## Contact us

- Katayama Hirofumi MZ <katayama.hirofumi.mz@gmail.com>
