// match_results_aliases_test.cpp --- Test for match_results type aliases
// Author: katahiromz
// License: MIT
#include "tests.h"

// Test that the new match_results aliases are defined and usable
int main() {
	TESTS_OUTPUT_INIT();

	std::cout << "Testing match_results type aliases...\n";

    // Initialize onigpp (no-op for std::regex)
    ONIGPP_TEST_INIT;

    // Test cmatch (const char*)
    {
        const char* text = "Hello World 123";
        myns::regex re("World (\\d+)");
        myns::cmatch m;

        bool found = myns::regex_search(text, m, re);
        assert(found);
        assert(m.size() == 2);
        assert(m[0].str() == "World 123");
        assert(m[1].str() == "123");
        std::cout << "✅ cmatch test passed\n";
    }

    // Test smatch (string::const_iterator)
    {
        std::string text = "Test smatch 456";
        myns::regex re("smatch (\\d+)");
        myns::smatch m;

        bool found = myns::regex_search(text, m, re);
        assert(found);
        assert(m.size() == 2);
        assert(m[0].str() == "smatch 456");
        assert(m[1].str() == "456");
        std::cout << "✅ smatch test passed\n";
    }

    // Test wsmatch (wstring::const_iterator)
    {
        std::wstring text = L"wsmatch test 789";
        myns::wregex re(L"test (\\d+)");
        myns::wsmatch m;

        bool found = myns::regex_search(text, m, re);
        assert(found);
        assert(m.size() == 2);
        assert(m[0].str() == L"test 789");
        assert(m[1].str() == L"789");
        std::cout << "✅ wsmatch test passed\n";
    }

#ifndef USE_STD_FOR_TESTS
    // Test u16smatch (u16string::const_iterator)
    {
        std::u16string text = u"u16smatch 999";
        myns::u16regex re(u"u16smatch (\\d+)");
        myns::u16smatch m;

        bool found = myns::regex_search(text, m, re);
        assert(found);
        assert(m.size() == 2);
        assert(m[0].str() == u"u16smatch 999");
        assert(m[1].str() == u"999");
        std::cout << "✅ u16smatch test passed\n";
    }

    // Test u32smatch (u32string::const_iterator)
    {
        std::u32string text = U"u32smatch 777";
        myns::u32regex re(U"u32smatch (\\d+)");
        myns::u32smatch m;

        bool found = myns::regex_search(text, m, re);
        assert(found);
        assert(m.size() == 2);
        assert(m[0].str() == U"u32smatch 777");
        assert(m[1].str() == U"777");
        std::cout << "✅ u32smatch test passed\n";
    }
#endif

    std::cout << "\n✅ All match_results alias tests passed!\n";
    return 0;
}
