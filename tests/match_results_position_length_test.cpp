// match_results_position_length_test.cpp --- Test for position() and length() methods
// Author: katahiromz
// License: MIT
#include "onigpp.h"
#include <iostream>
#include <regex>
#include <cassert>

// --- Additional headers for Windows ---
#ifdef _WIN32
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#endif

// Alias namespace for ease of use
#ifdef USE_STD_FOR_TESTS
	namespace op = std;
#else
	namespace op = onigpp;
#endif

int main() {
	// --- Measures to avoid garbled characters on Windows consoles ---
#ifdef _WIN32
	SetConsoleOutputCP(CP_UTF8);
#else
	std::setlocale(LC_ALL, "");
#endif

	std::cout << "Testing match_results position() and length() methods...\n";

#ifndef USE_STD_FOR_TESTS
    // Initialize onigpp
    onigpp::auto_init init;
#endif

    // Test 1: Basic position() and length() test with cmatch
    {
        const char* text = "Hello World 123";
        op::regex re("World (\\d+)");
        op::cmatch m;

        bool found = op::regex_search(text, m, re);
        assert(found);
        assert(m.size() == 2);
        
        // Test position of full match (m[0])
        auto pos0 = m.position(0);
        assert(pos0 == 6); // "World 123" starts at index 6
        std::cout << "  position(0) = " << pos0 << " ✓\n";
        
        // Test length of full match (m[0])
        auto len0 = m.length(0);
        assert(len0 == 9); // "World 123" has length 9
        std::cout << "  length(0) = " << len0 << " ✓\n";
        
        // Test position of first capture group (m[1])
        auto pos1 = m.position(1);
        assert(pos1 == 12); // "123" starts at index 12
        std::cout << "  position(1) = " << pos1 << " ✓\n";
        
        // Test length of first capture group (m[1])
        auto len1 = m.length(1);
        assert(len1 == 3); // "123" has length 3
        std::cout << "  length(1) = " << len1 << " ✓\n";
        
        std::cout << "✅ Basic cmatch test passed\n";
    }

    // Test 2: Test with smatch (string iterator)
    {
        std::string text = "Test smatch 456";
        op::regex re("smatch (\\d+)");
        op::smatch m;

        bool found = op::regex_search(text, m, re);
        assert(found);
        assert(m.size() == 2);
        
        // Test position of full match
        auto pos0 = m.position(0);
        assert(pos0 == 5); // "smatch 456" starts at index 5
        std::cout << "  smatch position(0) = " << pos0 << " ✓\n";
        
        // Test length of full match
        auto len0 = m.length(0);
        assert(len0 == 10); // "smatch 456" has length 10
        std::cout << "  smatch length(0) = " << len0 << " ✓\n";
        
        // Test position of capture group
        auto pos1 = m.position(1);
        assert(pos1 == 12); // "456" starts at index 12
        std::cout << "  smatch position(1) = " << pos1 << " ✓\n";
        
        // Test length of capture group
        auto len1 = m.length(1);
        assert(len1 == 3); // "456" has length 3
        std::cout << "  smatch length(1) = " << len1 << " ✓\n";
        
        std::cout << "✅ smatch test passed\n";
    }

    // Test 3: Test npos constant for invalid submatch
    {
        const char* text = "Hello World";
        op::regex re("World");
        op::cmatch m;

        bool found = op::regex_search(text, m, re);
        assert(found);
        
        // Test position() for out-of-bounds submatch index
        auto pos_invalid = m.position(5);
        assert(pos_invalid == op::cmatch::npos);
        std::cout << "  position(5) = npos ✓\n";
        
        // Test length() for out-of-bounds submatch index
        auto len_invalid = m.length(5);
        assert(len_invalid == 0);
        std::cout << "  length(5) = 0 ✓\n";
        
        std::cout << "✅ npos test passed\n";
    }

    // Test 4: Test with multiple capture groups
    {
        std::string text = "Date: 2024-11-24";
        op::regex re("(\\d{4})-(\\d{2})-(\\d{2})");
        op::smatch m;

        bool found = op::regex_search(text, m, re);
        assert(found);
        assert(m.size() == 4); // Full match + 3 capture groups
        
        // Full match
        assert(m.position(0) == 6);
        assert(m.length(0) == 10);
        
        // Year
        assert(m.position(1) == 6);
        assert(m.length(1) == 4);
        assert(m[1].str() == "2024");
        
        // Month
        assert(m.position(2) == 11);
        assert(m.length(2) == 2);
        assert(m[2].str() == "11");
        
        // Day
        assert(m.position(3) == 14);
        assert(m.length(3) == 2);
        assert(m[3].str() == "24");
        
        std::cout << "✅ Multiple capture groups test passed\n";
    }

    // Test 5: Test with wstring
    {
        std::wstring text = L"wsmatch test 789";
        op::wregex re(L"test (\\d+)");
        op::wsmatch m;

        bool found = op::regex_search(text, m, re);
        assert(found);
        assert(m.size() == 2);
        
        // Test position of full match
        auto pos0 = m.position(0);
        assert(pos0 == 8); // "test 789" starts at index 8
        std::cout << "  wsmatch position(0) = " << pos0 << " ✓\n";
        
        // Test length of full match
        auto len0 = m.length(0);
        assert(len0 == 8); // "test 789" has length 8
        std::cout << "  wsmatch length(0) = " << len0 << " ✓\n";
        
        std::cout << "✅ wsmatch test passed\n";
    }

    std::cout << "\n✅ All position() and length() tests passed!\n";
    return 0;
}
