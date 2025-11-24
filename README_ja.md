[English](README.md) [License](LICENSE.txt)

---

# Oniguruma++

Oniguruma の正規表現エンジンを C++11 向けに `std::regex` ライクなスタイルでラップしたライブラリです。使いやすさと互換性を重視しています。

![CI](https://github.com/katahiromz/onigpp/actions/workflows/ci.yml/badge.svg)

## What's this?

このリポジトリは Oniguruma を C++ から使いやすくするためのラッパーです。標準ライブラリの `<regex>` と似たAPIスタイルで利用でき、Oniguruma の強力な正規表現機能を活かせます。

## Features

- Oniguruma の機能へ `std::regex` ライクなインターフェースでアクセス
- CMake による簡単ビルド
- 標準ライブラリ実装との互換性をテスト（CIで複数設定を検証）

## Requirements

- C++11 互換コンパイラ（Visual Studio 2015+, GCC, Clang 等）
- CMake 3.x
- git（サブモジュール対応のため）

## Quick start (usage)

簡易例

```cpp
// 例: 実際のシグネチャは API.md を参照してください
#include "onigpp.h"
#include <iostream>
#include <string>

namespace re = onigpp;
//namespace re = std; // 切り替え可能

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

推奨手順（ソースからビルド）:

```bash
git clone https://github.com/katahiromz/onigpp
cd onigpp
git submodule update --init --recursive

mkdir build
cmake -B build
cmake --build build
```

よく使う CMake オプション例:
- `-DCMAKE_BUILD_TYPE=Release / Debug`
- `-DUSE_STD_FOR_TESTS=ON/OFF` （CI と同じ検証をローカルでも行う場合）
- `-DBUILD_SHARED_LIBS=ON/OFF`

Windows (MSVC) の場合は Visual Studio の generator を指定して cmake を実行してください。

## Testing

プロジェクト内のテストを実行:

```bash
cmake --build build -t test
# または CTest を直接:
cd build
ctest --output-on-failure
```

CI では `USE_STD_FOR_TESTS=ON` と `OFF` の両方でテストが回っています（互換性確認のため）。

## API / Reference

- [RE.md](RE.md) — Oniguruma 正規表現リファレンス（英語）
- [API.md](API.md) — Oniguruma C API リファレンス（英語）

## Header

- [onigpp.h](onigpp.h) — 主要ヘッダ

## Platforms

- Windows (Visual Studio 2015 以降)
- Linux (Ubuntu 等)
- macOS

## License

- BSD-2-Clause（詳細は LICENSE ファイルを参照）

## Contact / Support

- Maintainer: Katayama Hirofumi MZ <katayama.hirofumi.mz@gmail.com>
- Issues: https://github.com/katahiromz/onigpp/issues

## Changelog

主要な変更点は [CHANGELOG.md](CHANGELOG.md) にまとめることを推奨します。
