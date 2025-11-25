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
- CMake >= 3.10（ビルドスクリプトに必要）
- git（サブモジュール対応のため）

## 対応プラットフォーム / コンパイラ

このプロジェクトはモダンな C++11 互換コンパイラ（MSVC 2015+、GCC 5+、Clang 3.8+）で動作するよう設計されています。CI は Windows、Linux、最近の macOS ツールチェーンでテストを実行しています。異なるツールチェーンを使用する場合は、issue を開くかローカルでテストしてください。

## Quick start (usage)

```cpp
#define USE_ONIGPP // onigpp と std を切り替え可能（デモ/例示用のみ）

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

> **注意:** 上記例の `#define USE_ONIGPP` は onigpp と `std::regex` の切り替えを示すためのものです。実際のプロジェクトでは、単に `onigpp.h` をインクルードし、onigpp ライブラリにリンクしてください。

## Build

推奨手順（ソースからビルド）:

```bash
git clone https://github.com/katahiromz/onigpp
cd onigpp
git submodule update --init --recursive
```

**アウトオブソース ビルド（ポータブル方式）:**

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

**CMake ショートハンド（CMake 3.13+）:**

```bash
cmake -S . -B build
cmake --build build
```

よく使う CMake オプション例:
- `-DCMAKE_BUILD_TYPE=Release` または `-DCMAKE_BUILD_TYPE=Debug`
- `-DUSE_STD_FOR_TESTS=ON` または `-DUSE_STD_FOR_TESTS=OFF`（CI と同じ検証をローカルでも行う場合）
- `-DBUILD_SHARED_LIBS=ON` または `-DBUILD_SHARED_LIBS=OFF`

Windows (MSVC) の場合は Visual Studio の generator を指定して cmake を実行してください。

## Testing

プロジェクト内のテストを実行:

```bash
cmake --build build --target test
```

または CTest を直接 build ディレクトリから実行:

```bash
cd build
ctest --output-on-failure
```

CI では `USE_STD_FOR_TESTS=ON` と `OFF` の両方でテストが回っています（互換性確認のため）。

### `USE_STD_FOR_TESTS` の使用

`USE_STD_FOR_TESTS` CMake オプションを使用すると、onigpp の代わりに `std::regex` を使用して互換性テストを実行できます。これはテストパターン自体が有効であることを確認し、onigpp が標準ライブラリと一貫した動作をすることを検証するのに便利です。

```bash
# USE_STD_FOR_TESTS を有効にしてビルド
cmake -S . -B build -DUSE_STD_FOR_TESTS=ON
cmake --build build

# ecmascript 互換性テストを実行
./build/ecmascript_compat_test
```

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
   - `ECMAScript` モードと組み合わせた場合、`multiline` フラグは ECMAScript のセマンティクスをエミュレートします
   - `ECMAScript` と `multiline` フラグの両方が設定されている場合、onigpp はコンパイル時にパターンを書き換え、`^` と `$` が行境界でマッチするようにします。認識される行区切り文字は: `\n`、`\r`、`\r\n`、U+2028（行区切り）、U+2029（段落区切り）です
   - このエミュレーションは ECMAScript のセマンティクスを保持します: ドット（`.`）は改行にマッチしません（将来の dotall フラグで個別に制御）
   - **パフォーマンス注記**: パターンの書き換えは正規表現構築時に小さな CPU コストを追加しますが、実行時のマッチングには影響しません
   - **制限**: 書き換えは一般的なケース（文字クラス外のエスケープされていない `^` と `$`）を処理します。複雑または特殊なパターンにはエッジケースがある可能性があります。問題を発見した場合は報告してください

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

## Contributing

コントリビューションを歓迎します！以下の手順で開始してください：

1. **提出前にテストを実行**: すべてのテストがローカルでパスすることを確認してください。
   ```bash
   cmake -S . -B build
   cmake --build build
   cd build && ctest --output-on-failure
   ```

2. **互換性テストを実行**: プロジェクトには `std::regex` との動作を検証する互換性ハーネスが含まれています。
   ```bash
   cmake -S . -B build -DUSE_STD_FOR_TESTS=ON
   cmake --build build
   cd build && ctest --output-on-failure
   ```

3. **一般的なガイドライン**:
   - 変更は焦点を絞り、最小限に保つ
   - 重要な機能や修正を追加する場合は `CHANGELOG.md` を更新する
   - 新機能にはテストを追加または更新する

## License

- BSD-2-Clause（詳細は LICENSE ファイルを参照）

## Contact / Support

- Maintainer: Katayama Hirofumi MZ <katayama.hirofumi.mz@gmail.com>
- Issues: https://github.com/katahiromz/onigpp/issues
