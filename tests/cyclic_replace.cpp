// onigpp_test.cpp --- Tests for Oniguruma++ (onigpp)
// Author: katahiromz
// License: BSD-2-Clause
#include "onigpp.h"
#include <iostream>
#include <string>
#include <vector>
#include <regex>
#include <cassert>
#include <locale>
#include <algorithm>

// --- Additional headers for Windows ---
#ifdef _WIN32
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#endif

// Alias namespace for ease of use
#ifdef USE_STD_FOR_TESTS
	namespace myns = std;
#else
	namespace myns = onigpp;
#endif

// ユーザーが指定するパターンの種類を区別するための構造体
// string -> wstring, regex -> wregex に変更
struct PatternKey {
	std::wstring pattern;
	bool is_regex;
	myns::wregex compiled_regex;

	// 文字列リテラルの場合
	static PatternKey Literal(const std::wstring& s) {
		return {s, false, myns::wregex()};
	}

	// 正規表現の場合
	static PatternKey Regex(const std::wstring& s) {
		return {s, true, myns::wregex(s)};
	}
};

// Pythonのre.escape相当の関数 (ワイド文字版)
std::wstring regex_escape(const std::wstring& s) {
	// ワイド文字列リテラル L"..." を使用
	static const myns::wregex special_chars(LR"([.^$|()\[\]{}*+?\\])");
	return myns::regex_replace(s, special_chars, LR"(\$&)");
}

/**
 * 文字列中の複数の文字列やパターンを同時に置換します (Unicode対応版)。
 */
std::wstring multi_replace(const std::wstring& input, const std::vector<std::pair<PatternKey, std::wstring>>& mapping) {
	if (mapping.empty()) {
		return input;
	}

	// 1. 結合パターンの作成
	std::wstring combined_pattern_str;
	std::vector<size_t> group_indices;
	size_t current_group_idx = 1;

	for (size_t i = 0; i < mapping.size(); ++i) {
		if (i > 0) combined_pattern_str += L"|"; // Lプレフィックスが必要

		combined_pattern_str += L"(";
		group_indices.push_back(current_group_idx);

		size_t internal_groups = 0;
		if (mapping[i].first.is_regex) {
			combined_pattern_str += mapping[i].first.pattern;
			internal_groups = mapping[i].first.compiled_regex.mark_count();
		} else {
			combined_pattern_str += regex_escape(mapping[i].first.pattern);
			internal_groups = 0;
		}
		combined_pattern_str += L")";

		current_group_idx += 1 + internal_groups;
	}

	myns::wregex catch_all_re(combined_pattern_str);

	// 2. イテレータを使って検索と置換を実行
	std::wstring result;
	std::wstring::const_iterator last_pos = input.begin();

	// myns::regex_iterator<std::wstring::const_iterator, wchar_t> を使用
	auto begin = myns::regex_iterator<std::wstring::const_iterator, wchar_t>(input.begin(), input.end(), catch_all_re);
	auto end = myns::regex_iterator<std::wstring::const_iterator, wchar_t>();

	for (auto i = begin; i != end; ++i) {
		myns::match_results<std::wstring::const_iterator> match = *i;

		result.append(last_pos, match[0].first);
		last_pos = match[0].second;

		int matched_mapping_idx = -1;

		for (size_t k = 0; k < group_indices.size(); ++k) {
			size_t g_idx = group_indices[k];
			if (g_idx < match.size() && match[g_idx].matched) {
				matched_mapping_idx = static_cast<int>(k);
				break;
			}
		}

		if (matched_mapping_idx != -1) {
			const auto& key = mapping[matched_mapping_idx].first;
			const auto& replacement = mapping[matched_mapping_idx].second;

			if (key.is_regex) {
				std::wstring src = match.str(0);
				result += regex_replace(src, key.compiled_regex, replacement);
			} else {
				result += replacement;
			}
		}
	}

	result.append(last_pos, input.end());

	return result;
}

void test_cyclic() {
	// 例1: 日本語の単語入れ替え
	// Lプレフィックスをつけてワイド文字列にする
	std::wstring text1 = L"私はリンゴとバナナが好きです";
	std::vector<std::pair<PatternKey, std::wstring>> mapping1 = {
		{PatternKey::Literal(L"リンゴ"), L"バナナ"},
		{PatternKey::Literal(L"バナナ"), L"リンゴ"}
	};

	auto result1 = multi_replace(text1, mapping1);
	assert(result1 == L"私はバナナとリンゴが好きです");

	std::wcout << L"Original: " << text1 << L"\n";
	std::wcout << L"Replaced: " << result1 << L"\n";

	std::wcout << L"-------------------\n";

	// 例2: 正規表現と日本語
	// 「日付: 2023年11月22日」のような形式を置換
	std::wstring text2 = L"今日の日付: 2023年11月22日";
	std::vector<std::pair<PatternKey, std::wstring>> mapping2 = {
		// (\d{4})年(\d{1,2})月(\d{1,2})日 -> $1/$2/$3
		{PatternKey::Regex(LR"((\d{4})年(\d{1,2})月(\d{1,2})日)"), L"$1/$2/$3"},
		{PatternKey::Literal(L"今日の日付"), L"Date"}
	};

	auto result2 = multi_replace(text2, mapping2);
	assert(result2 == L"Date: 2023/11/22");

	std::wcout << L"Original: " << text2 << L"\n";
	std::wcout << L"Replaced: " << result2 << L"\n";
}

int main() {
	// --- Measures to avoid garbled characters on Windows consoles ---
#ifdef _WIN32
	// Switch to UTF-8 mode
	_setmode(_fileno(stdout), _O_U8TEXT);
	// Ensure console uses UTF-8 code page for interoperability
	SetConsoleOutputCP(CP_UTF8);
#else
	// For Linux/Mac, setting the locale is usually sufficient
	std::setlocale(LC_ALL, "");
#endif

#ifndef USE_STD_FOR_TESTS
	// Oniguruma initialization
	myns::auto_init init;
#endif

	// Do tests
	test_cyclic();

	return 0;
}
