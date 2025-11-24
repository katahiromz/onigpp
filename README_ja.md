[English](README.md) | [License](LICENSE.txt) | [Changelog](CHANGELOG.md)

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

```cpp
#define USE_ONIGPP // onigpp と std を切り替え可能

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

## std::regex からの移行

onigpp は ECMAScript 互換モードを提供しており、`std::regex` からの移行を容易にします。以下に知っておくべき情報を示します。

### ECMAScript モードの使用

ECMAScript 互換の構文を使用するには、正規表現を作成する際に `ECMAScript` フラグを指定します：

```cpp
#include "onigpp.h"
namespace rex = onigpp;
rex::auto_init g_auto_init;

// ECMAScript モード（std::regex のデフォルトに類似）
rex::regex pattern(R"(\d+)", rex::regex::ECMAScript);
```

### サポートされている機能

onigpp の ECMAScript モードでは以下をサポートしています：

1. **エスケープシーケンス**:
   - `\xHH` - 16進数エスケープ（例: `\x41` で 'A'）
   - `\uHHHH` - Unicode エスケープ（例: `\u00E9` で 'é'）
   - `\0` - ヌル文字（数字が後続しない場合）

2. **ドット（.）の振る舞い**: 
   - デフォルトで、ドット（`.`）は改行文字にマッチしません（std::regex と同じ）

3. **置換テンプレート**:
   - `$&` - マッチ全体
   - `$1`, `$2`, ... - キャプチャグループ
   - `$`` - マッチ前のテキスト（プレフィックス）
   - `$'` - マッチ後のテキスト（サフィックス）
   - `$$` - リテラルのドル記号
   - `${name}` - 名前付きキャプチャグループ

### 既知の違い

いくつかの機能は `std::regex` と異なる動作をする場合があります：

1. **マルチラインモード**: 
   - `multiline` フラグは利用可能ですが、`^` と `$` アンカーについて ECMAScript のセマンティクスと完全には一致しない場合があります
   - ECMAScript では、multiline は `^` と `$` が行境界にマッチするかどうかに影響します
   - Oniguruma の MULTILINE オプションはドットとアンカーの両方に影響します

2. **名前付きキャプチャ**:
   - Oniguruma は `(?<name>...)` 構文を使用します
   - ECMAScript もこの構文をサポートしています

3. **先読み/後読み**:
   - 肯定・否定の先読みがサポートされています
   - 後読みも利用可能です（基本的な ECMAScript を超えた拡張）

### 移行例

**移行前（std::regex）:**
```cpp
#include <regex>
std::string text = "価格: $100";
std::regex rex(R"(\$(\d+))");
std::string result = std::regex_replace(text, rex, "$$$1.00");
// 結果: "価格: $100.00"
```

**移行後（onigpp の ECMAScript モード）:**
```cpp
#include "onigpp.h"
onigpp::auto_init init;
std::string text = "価格: $100";
onigpp::regex rex(R"(\$(\d+))", onigpp::regex::ECMAScript);
std::string result = onigpp::regex_replace(text, rex, "$$$1.00");
// 結果: "価格: $100.00"
```

### ECMAScript の複数行の例

複数行エミュレーションでは、`^` と `$` は行の境界で一致します:

```cpp
#include "onigpp.h"
onigpp::auto_init init;

std::string text = "line1\nline2\nline3";
onigpp::regex rex("^line\\d", onigpp::regex::ECMAScript | onigpp::regex::multiline);

// 「line」で始まり、その後に数字が続くすべての行を検索します
auto begin = onigpp::sregex_iterator(text.begin(), text.end(), rex);
auto end = onigpp::sregex_iterator();

for (auto it = begin; it != end; ++it) {
    std::cout << "Match: " << it->str() << "\n";
}
// 出力:
// Match: line1
// Match: line2
// Match: line3
```

注意: 複数行モードの場合でも、ドット (`.`) は改行と一致しないため、ECMAScript のセマンティクスは保持されます。

### 移行のテスト

テストスイートには包括的な ECMAScript 互換性テストが含まれています。以下のコマンドで実行できます：

```bash
cmake --build build --target ecmascript_compat_test
./build/ecmascript_compat_test
```

## API / Reference

- [RE.md](RE.md) — Oniguruma 正規表現リファレンス（英語）
- [API.md](API.md) — Oniguruma C API リファレンス（英語）

## Header

- [onigpp.h](onigpp.h) — 主要ヘッダ

## License

- BSD-2-Clause（詳細は LICENSE ファイルを参照）

## Contact / Support

- Maintainer: Katayama Hirofumi MZ <katayama.hirofumi.mz@gmail.com>
- Issues: https://github.com/katahiromz/onigpp/issues
