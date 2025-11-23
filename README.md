# Oniguruma++

## What's this?

This is a C++ wrapper of the Oniguruma regular expression engine.

## Regular expression reference

- [RE.md](RE.md) — Oniguruma Regular Expressions reference (English)

## API reference

- [API.md](API.md) — Oniguruma C API reference (English)

## Build

```bash
git clone https://github.com/katahiromz/onigpp
git submodule update --init --recursive
cd onigpp
mkdir build
cmake -B build
cmake --build build
```

## Testing

```bash
cmake --build build -t test
```

## Contact us

- Katayama Hirofumi MZ <katayama.hirofumi.mz@gmail.com>