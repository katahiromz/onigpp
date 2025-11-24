// compat_regression_partial_match.cpp --- Regression test for partial_match_fail
// Reference: CI failure at ref 7da3004c3d01b00f3298bcfc74156be2ff27388e
// Author: katahiromz
// License: BSD-2-Clause
//
// This test reproduces the previously failing case where compat_test showed
// a mismatch in capture counts between std::regex and onigpp for the test
// named "partial_match_fail" with pattern "^hello$" and input "hello world".
//
// The test verifies that both std::regex and onigpp produce consistent
// results when attempting to match a pattern with anchors (^hello$) against
// input that has extra text ("hello world"). The regex_match operation should
// fail since the pattern is anchored to match the entire string, and the input
// has extra text.

#include "onigpp.h"
#include <regex>
#include <iostream>
#include <string>
#include <cassert>
#include <clocale>

#ifdef _WIN32
	#include <windows.h>
	#include <io.h>
	#include <fcntl.h>

	inline void TESTS_OUTPUT_INIT() {
		SetConsoleOutputCP(CP_UTF8);
	}
#else
	inline void TESTS_OUTPUT_INIT() {
		std::setlocale(LC_ALL, "");
	}
#endif

int main() {
	TESTS_OUTPUT_INIT();

	// Initialize oniguruma
	onigpp::auto_init onig_init;

	std::cout << "========================================" << std::endl;
	std::cout << "Regression Test: partial_match_fail" << std::endl;
	std::cout << "========================================" << std::endl;
	std::cout << "Reference: CI failure at ref 7da3004c3d01b00f3298bcfc74156be2ff27388e" << std::endl;
	std::cout << std::endl;

	// Test case from compat_test patterns.json
	const std::string pattern = "^hello$";
	const std::string input = "hello world";
	
	std::cout << "Pattern: " << pattern << std::endl;
	std::cout << "Input:   " << input << std::endl;
	std::cout << "Operation: regex_match (should fail due to anchors and extra text)" << std::endl;
	std::cout << std::endl;

	// Test with std::regex
	std::regex stdRe(pattern, std::regex_constants::ECMAScript);
	std::smatch stdMatch;
	bool stdMatched = std::regex_match(input, stdMatch, stdRe);

	std::cout << "std::regex result:" << std::endl;
	std::cout << "  Matched: " << (stdMatched ? "true" : "false") << std::endl;
	std::cout << "  Capture count: " << stdMatch.size() << std::endl;
	
	// Test with onigpp
	onigpp::basic_regex<char> onigppRe(pattern, onigpp::regex_constants::ECMAScript, onigpp::encoding_constants::UTF8);
	onigpp::match_results<std::string::const_iterator> onigppMatch;
	bool onigppMatched = onigpp::regex_match(input, onigppMatch, onigppRe);

	std::cout << "onigpp result:" << std::endl;
	std::cout << "  Matched: " << (onigppMatched ? "true" : "false") << std::endl;
	std::cout << "  Capture count: " << onigppMatch.size() << std::endl;
	std::cout << std::endl;

	// Verify consistency
	bool testPassed = true;
	
	// Both should report the same match status
	if (stdMatched != onigppMatched) {
		std::cout << "❌ FAIL: Match status differs" << std::endl;
		std::cout << "  std::regex matched: " << stdMatched << std::endl;
		std::cout << "  onigpp matched: " << onigppMatched << std::endl;
		testPassed = false;
	}

	// Both should report the same capture count
	if (stdMatch.size() != onigppMatch.size()) {
		std::cout << "❌ FAIL: Capture count differs" << std::endl;
		std::cout << "  std::regex: " << stdMatch.size() << " captures" << std::endl;
		std::cout << "  onigpp: " << onigppMatch.size() << " captures" << std::endl;
		testPassed = false;
	}

	// If both matched, verify the captures are the same
	if (stdMatched && onigppMatched) {
		for (size_t i = 0; i < std::min(stdMatch.size(), onigppMatch.size()); ++i) {
			if (stdMatch[i].str() != onigppMatch[i].str()) {
				std::cout << "❌ FAIL: Capture[" << i << "] differs" << std::endl;
				std::cout << "  std::regex: \"" << stdMatch[i].str() << "\"" << std::endl;
				std::cout << "  onigpp: \"" << onigppMatch[i].str() << "\"" << std::endl;
				testPassed = false;
			}
		}
	}

	if (testPassed) {
		std::cout << "✅ PASS: Both implementations produce consistent results" << std::endl;
		return 0;
	} else {
		std::cout << "❌ Test failed - implementations are inconsistent" << std::endl;
		return 1;
	}
}
