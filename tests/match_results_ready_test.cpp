// match_results_ready_test.cpp --- Test for ready() method
// Author: katahiromz
// License: BSD-2-Clause
#include "tests.h"

int main() {
	TESTS_OUTPUT_INIT();

	std::cout << "Testing match_results ready() method...\n";

	// Initialize onigpp
	ONIGPP_TEST_INIT;

	// Test 1: Default-constructed match_results should not be ready
	{
		rex::smatch m;
		#ifndef USE_STD_FOR_TESTS
		assert(!m.ready());
		std::cout << "  Default-constructed smatch ready() = false ✓\n";
		#else
		// Note: For std::regex comparison, we just print the value
		// Per C++ standard, default-constructed match_results should have ready() == false
		std::cout << "  Default-constructed smatch ready() = " << (m.ready() ? "true" : "false") << " (std::regex)\n";
		#endif
		std::cout << "✅ Default construction test passed\n";
	}

	// Test 2: After successful regex_search, match_results should be ready
	{
		std::string text = "Hello World";
		rex::regex re("World");
		rex::smatch m;
		
		bool found = rex::regex_search(text, m, re);
		assert(found);
		assert(m.ready());
		std::cout << "  smatch ready() after successful search = true ✓\n";
		std::cout << "✅ Successful search test passed\n";
	}

	// Test 3: After unsuccessful regex_search, match_results should still be ready
	{
		std::string text = "Hello World";
		rex::regex re("NotFound");
		rex::smatch m;
		
		bool found = rex::regex_search(text, m, re);
		assert(!found);
		assert(m.ready());
		std::cout << "  smatch ready() after unsuccessful search = true ✓\n";
		std::cout << "✅ Unsuccessful search test passed\n";
	}

	// Test 4: After successful regex_match, match_results should be ready
	{
		std::string text = "Hello";
		rex::regex re("Hello");
		rex::smatch m;
		
		bool found = rex::regex_match(text, m, re);
		assert(found);
		assert(m.ready());
		std::cout << "  smatch ready() after successful match = true ✓\n";
		std::cout << "✅ Successful match test passed\n";
	}

	// Test 5: After unsuccessful regex_match, match_results should still be ready
	{
		std::string text = "Hello World";
		rex::regex re("Hello");  // Pattern doesn't match entire string
		rex::smatch m;
		
		bool found = rex::regex_match(text, m, re);
		assert(!found);
		assert(m.ready());
		std::cout << "  smatch ready() after unsuccessful match = true ✓\n";
		std::cout << "✅ Unsuccessful match test passed\n";
	}

	// Test 6: Test with cmatch (pointer-based)
	{
		const char* text = "Test String";
		rex::regex re("String");
		rex::cmatch m;
		
		#ifndef USE_STD_FOR_TESTS
		assert(!m.ready());
		#endif
		
		bool found = rex::regex_search(text, m, re);
		assert(found);
		assert(m.ready());
		std::cout << "  cmatch ready() after search = true ✓\n";
		std::cout << "✅ cmatch test passed\n";
	}

	// Test 7: Test with wsmatch (wide string)
	{
		std::wstring text = L"Wide Test";
		rex::wregex re(L"Test");
		rex::wsmatch m;
		
		#ifndef USE_STD_FOR_TESTS
		assert(!m.ready());
		#endif
		
		bool found = rex::regex_search(text, m, re);
		assert(found);
		assert(m.ready());
		std::cout << "  wsmatch ready() after search = true ✓\n";
		std::cout << "✅ wsmatch test passed\n";
	}

	// Test 8: Verify ready() is noexcept (onigpp-specific, std::match_results::ready() is not noexcept)
	#ifndef USE_STD_FOR_TESTS
	{
		rex::smatch m;
		static_assert(noexcept(m.ready()), "ready() must be noexcept");
		std::cout << "  ready() is noexcept ✓\n";
		std::cout << "✅ noexcept test passed\n";
	}
	#endif

	// Test 9: Test that empty() and ready() have different semantics
	{
		std::string text = "Test";
		rex::regex re("NotFound");
		rex::smatch m;
		
		bool found = rex::regex_search(text, m, re);
		assert(!found);
		// After unsuccessful search, match_results is ready but empty
		assert(m.ready());
		assert(m.empty());
		std::cout << "  After unsuccessful search: ready()=true, empty()=true ✓\n";
		std::cout << "✅ ready() vs empty() semantics test passed\n";
	}

	std::cout << "\n✅ All ready() tests passed!\n";
	return 0;
}
