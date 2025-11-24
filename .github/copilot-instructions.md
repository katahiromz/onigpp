# Copilot 指示書

## 技術スタック

- C++11 and later

## リポジトリの概要

- `oniguruma/` - サブモジュール
- `src/` - ソース
- `tests/` - テスト群
- `tests/compat/` - `compat_test.cpp`がある場所
- `tests/compat/patterns.json` - `compat_test`でテストするパターン項目を書いたJSON
- `onigpp.h` - 主要ヘッダ。

## 指示

- コメントは英語でお願いします。
- JSONでは、`\xXX` 形式は使わず、`\uXXXX`を使用してください。
- `compat_test`でテスト項目が失敗したときは、`compat_test`の実行を失敗させてください。
- 可能な限り、レポジトリ内のmarkdownファイルの記述に従ってください。
