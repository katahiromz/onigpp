// use_std_for_tests.h --- Common header for USE_STD_FOR_TESTS support
// Author: katahiromz
// License: BSD-2-Clause
#pragma once

// This header centralizes the USE_STD_FOR_TESTS conditional logic
// to minimize code duplication across test files.

#ifdef USE_STD_FOR_TESTS
	// When USE_STD_FOR_TESTS is defined, use std:: types
	namespace myns = std;
	
	// No initialization needed for std::regex
	#define ONIGPP_TEST_INIT ((void)0)
#else
	// When USE_STD_FOR_TESTS is not defined, use onigpp:: types
	namespace myns = onigpp;
	
	// Oniguruma initialization is required
	#define ONIGPP_TEST_INIT myns::auto_init onigpp_test_init
#endif
