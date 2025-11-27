// onigpp_test.cpp --- Tests for Oniguruma++ (onigpp)
// Author: katahiromz
// License: BSD-2-Clause
#include "tests.h"

// Structure to distinguish the type of pattern specified by the user
// Changed from string -> wstring, regex -> wregex
struct PatternKey {
	std::wstring pattern;
	bool is_regex;
	rex::wregex compiled_regex;

	// For string literals
	static PatternKey Literal(const std::wstring& s) {
		return {s, false, rex::wregex()};
	}

	// For regular expressions
	static PatternKey Regex(const std::wstring& s) {
		return {s, true, rex::wregex(s)};
	}
};

// Function equivalent to Python's re.escape (wide character version)
std::wstring regex_escape(const std::wstring& s) {
	// Use wide string literal L"..."
	static const rex::wregex special_chars(LR"([.^$|()\[\]{}*+?\\])");
	return rex::regex_replace(s, special_chars, LR"(\$&)");
}

/**
 * Replaces multiple strings or patterns simultaneously in a string (Unicode-compatible version).
 */
std::wstring multi_replace(const std::wstring& input, const std::vector<std::pair<PatternKey, std::wstring>>& mapping) {
	if (mapping.empty()) {
		return input;
	}

	// 1. Create combined pattern
	std::wstring combined_pattern_str;
	std::vector<size_t> group_indices;
	size_t current_group_idx = 1;

	for (size_t i = 0; i < mapping.size(); ++i) {
		if (i > 0) combined_pattern_str += L"|"; // L prefix is required

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

	rex::wregex catch_all_re(combined_pattern_str);

	// 2. Perform search and replace using iterators
	std::wstring result;
	std::wstring::const_iterator last_pos = input.begin();

	// Use rex::regex_iterator<std::wstring::const_iterator, wchar_t>
	auto begin = rex::regex_iterator<std::wstring::const_iterator, wchar_t>(input.begin(), input.end(), catch_all_re);
	auto end = rex::regex_iterator<std::wstring::const_iterator, wchar_t>();

	for (auto i = begin; i != end; ++i) {
		rex::match_results<std::wstring::const_iterator> match = *i;

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
	// Example 1: Swapping Japanese words
	// Use L prefix for wide string
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

	// Example 2: Regular expressions with Japanese
	// Replace Japanese date format (YYYY年MM月DD日) with ISO format
	std::wstring text2 = L"今日の日付: 2023年11月22日";
	std::vector<std::pair<PatternKey, std::wstring>> mapping2 = {
		// Pattern: (YYYY)年(MM)月(DD)日 -> $1/$2/$3
		{PatternKey::Regex(LR"((\d{4})年(\d{1,2})月(\d{1,2})日)"), L"$1/$2/$3"},
		{PatternKey::Literal(L"今日の日付"), L"Date"}
	};

	auto result2 = multi_replace(text2, mapping2);
	assert(result2 == L"Date: 2023/11/22");

	std::wcout << L"Original: " << text2 << L"\n";
	std::wcout << L"Replaced: " << result2 << L"\n";
}

int main() {
	TESTS_OUTPUT_INIT(true);

	// Oniguruma initialization (no-op for std::regex)
	ONIGPP_TEST_INIT;

	// Do tests
	test_cyclic();

	return 0;
}
