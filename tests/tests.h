// use_std_for_tests.h --- Common header for USE_STD_FOR_TESTS support
// Author: katahiromz
// License: BSD-2-Clause
#pragma once

// This header centralizes the USE_STD_FOR_TESTS conditional logic
// to minimize code duplication across test files.

#include <iostream>
#include <string>
#include <vector>
#include <locale>
#include <algorithm>
#include <cassert>

#ifdef USE_STD_FOR_TESTS
	#include <regex>

	// When USE_STD_FOR_TESTS is defined, use std:: types
	namespace myns = std;
	
	// No initialization needed for std::regex
	#define ONIGPP_TEST_INIT ((void)0)
#else
	#include "onigpp.h"

	// When USE_STD_FOR_TESTS is not defined, use onigpp:: types
	namespace myns = onigpp;
	
	// Oniguruma initialization is required
	#define ONIGPP_TEST_INIT myns::auto_init onigpp_test_init
#endif

// --- Additional headers for Windows ---
#ifdef _WIN32
	#include <windows.h>
	#include <io.h>
	#include <fcntl.h>

	inline void TESTS_OUTPUT_INIT(bool use_wcout = false) {
		if (use_wcout)
			_setmode(_fileno(stdout), _O_U8TEXT); // Use std::cout instead of std::wcout
		// Ensure console uses UTF-8 code page for interoperability
		SetConsoleOutputCP(CP_UTF8);
	}
#else
	// For Linux/Mac, setting the locale is usually sufficient
	inline void TESTS_OUTPUT_INIT(bool use_wcout = false) {
		std::setlocale(LC_ALL, "");
	}
#endif
