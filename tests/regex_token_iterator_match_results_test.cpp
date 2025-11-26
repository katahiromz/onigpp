// regex_token_iterator_match_results_test.cpp --- Test for regex_token_iterator::current_match_results()
// Author: katahiromz
// License: BSD-2-Clause

#include "onigpp.h"
#include <iostream>
#include <string>
#include <vector>
#include <cassert>

// Test 1: Basic current_match_results() access
void test_basic_current_match_results() {
    std::cout << "Test 1: Basic current_match_results() access..." << std::endl;
    
    std::string text = "apple,banana,cherry";
    onigpp::regex re(",");
    
    onigpp::sregex_token_iterator it(text.begin(), text.end(), re, -1);
    onigpp::sregex_token_iterator end;
    
    std::vector<std::string> tokens;
    while (it != end) {
        tokens.push_back(it->str());
        
        // Access current_match_results to get match information
        const auto& mr = it.current_match_results();
        
        // For splitting (submatch -1), the match_results still gives us the delimiter match info
        // The iterator may be at end-of-matches for some tokens
        (void)mr; // Just verify we can access it
        
        ++it;
    }
    
    assert(tokens.size() == 3);
    assert(tokens[0] == "apple");
    assert(tokens[1] == "banana");
    assert(tokens[2] == "cherry");
    
    std::cout << "  PASSED" << std::endl;
}

// Test 2: Access capture groups via current_match_results()
void test_capture_groups_via_current_match_results() {
    std::cout << "Test 2: Access capture groups via current_match_results()..." << std::endl;
    
    std::string text = "John:25,Jane:30,Bob:35";
    onigpp::regex re("(\\w+):(\\d+)");
    
    // Iterate over full matches (submatch 0)
    onigpp::sregex_token_iterator it(text.begin(), text.end(), re, 0);
    onigpp::sregex_token_iterator end;
    
    std::vector<std::string> names;
    std::vector<std::string> ages;
    
    while (it != end) {
        // Access the full match_results to get all capture groups
        const auto& mr = it.current_match_results();
        
        // Verify we can access all capture groups
        assert(mr.size() == 3); // Full match + 2 capture groups
        
        names.push_back(mr[1].str());  // Name capture group
        ages.push_back(mr[2].str());   // Age capture group
        
        ++it;
    }
    
    assert(names.size() == 3);
    assert(names[0] == "John");
    assert(names[1] == "Jane");
    assert(names[2] == "Bob");
    
    assert(ages[0] == "25");
    assert(ages[1] == "30");
    assert(ages[2] == "35");
    
    std::cout << "  PASSED" << std::endl;
}

// Test 3: Access prefix() and suffix() via current_match_results()
void test_prefix_suffix_via_current_match_results() {
    std::cout << "Test 3: Access prefix() and suffix() via current_match_results()..." << std::endl;
    
    std::string text = "Hello World Foo";
    onigpp::regex re("\\bWorld\\b");
    
    // Iterate over full matches
    onigpp::sregex_token_iterator it(text.begin(), text.end(), re, 0);
    onigpp::sregex_token_iterator end;
    
    assert(it != end); // Should have one match
    
    const auto& mr = it.current_match_results();
    
    // Access prefix and suffix
    auto prefix = mr.prefix();
    auto suffix = mr.suffix();
    
    assert(prefix.str() == "Hello ");
    assert(suffix.str() == " Foo");
    
    ++it;
    assert(it == end); // No more matches
    
    std::cout << "  PASSED" << std::endl;
}

// Test 4: Multiple submatches with current_match_results()
void test_multiple_submatches_with_current_match_results() {
    std::cout << "Test 4: Multiple submatches with current_match_results()..." << std::endl;
    
    std::string text = "key1=val1;key2=val2";
    onigpp::regex re("(\\w+)=(\\w+)");
    
    // Request both capture groups as tokens
    std::vector<int> submatches = {1, 2};
    onigpp::sregex_token_iterator it(text.begin(), text.end(), re, submatches);
    onigpp::sregex_token_iterator end;
    
    std::vector<std::string> tokens;
    while (it != end) {
        tokens.push_back(it->str());
        
        // Can still access full match_results for each token
        const auto& mr = it.current_match_results();
        assert(mr.size() >= 3); // Full match + 2 capture groups
        
        ++it;
    }
    
    // Should have 4 tokens: key1, val1, key2, val2
    assert(tokens.size() == 4);
    assert(tokens[0] == "key1");
    assert(tokens[1] == "val1");
    assert(tokens[2] == "key2");
    assert(tokens[3] == "val2");
    
    std::cout << "  PASSED" << std::endl;
}

// Test 5: Verify match_results_type typedef exists
void test_match_results_type_typedef() {
    std::cout << "Test 5: Verify match_results_type typedef exists..." << std::endl;
    
    // Verify the typedef compiles and is the correct type
    using iter_type = onigpp::sregex_token_iterator;
    using expected_type = onigpp::match_results<std::string::const_iterator>;
    
    // Check if the types match (compile-time check)
    static_assert(std::is_same<iter_type::match_results_type, expected_type>::value,
                  "match_results_type should be match_results<BidirIt>");
    
    std::cout << "  PASSED" << std::endl;
}

// Test 6: position() and length() via current_match_results()
void test_position_length_via_current_match_results() {
    std::cout << "Test 6: position() and length() via current_match_results()..." << std::endl;
    
    std::string text = "abc 123 def";
    onigpp::regex re("\\d+");
    
    onigpp::sregex_token_iterator it(text.begin(), text.end(), re, 0);
    onigpp::sregex_token_iterator end;
    
    assert(it != end);
    
    const auto& mr = it.current_match_results();
    
    // Check position and length
    assert(mr.position(0) == 4); // "123" starts at position 4
    assert(mr.length(0) == 3);   // "123" has length 3
    
    ++it;
    assert(it == end);
    
    std::cout << "  PASSED" << std::endl;
}

// Test 7: format() via current_match_results()
void test_format_via_current_match_results() {
    std::cout << "Test 7: format() via current_match_results()..." << std::endl;
    
    std::string text = "John:25, Jane:30";
    onigpp::regex re("(\\w+):(\\d+)");
    
    onigpp::sregex_token_iterator it(text.begin(), text.end(), re, 0);
    onigpp::sregex_token_iterator end;
    
    std::vector<std::string> formatted;
    while (it != end) {
        const auto& mr = it.current_match_results();
        formatted.push_back(mr.format("Name: $1, Age: $2"));
        ++it;
    }
    
    assert(formatted.size() == 2);
    assert(formatted[0] == "Name: John, Age: 25");
    assert(formatted[1] == "Name: Jane, Age: 30");
    
    std::cout << "  PASSED" << std::endl;
}

// Test 8: Wide string support
void test_wide_string_support() {
    std::cout << "Test 8: Wide string support..." << std::endl;
    
    std::wstring text = L"apple,banana";
    onigpp::wregex re(L",");
    
    onigpp::wsregex_token_iterator it(text.begin(), text.end(), re, -1);
    onigpp::wsregex_token_iterator end;
    
    std::vector<std::wstring> tokens;
    while (it != end) {
        tokens.push_back(it->str());
        
        // Access current_match_results for wide strings
        const auto& mr = it.current_match_results();
        (void)mr; // Just verify we can access it
        
        ++it;
    }
    
    assert(tokens.size() == 2);
    assert(tokens[0] == L"apple");
    assert(tokens[1] == L"banana");
    
    std::cout << "  PASSED" << std::endl;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "regex_token_iterator::current_match_results() Tests" << std::endl;
    std::cout << "========================================" << std::endl;
    
    // Initialize oniguruma
    onigpp::auto_init onig_init;
    
    test_basic_current_match_results();
    test_capture_groups_via_current_match_results();
    test_prefix_suffix_via_current_match_results();
    test_multiple_submatches_with_current_match_results();
    test_match_results_type_typedef();
    test_position_length_via_current_match_results();
    test_format_via_current_match_results();
    test_wide_string_support();
    
    std::cout << "========================================" << std::endl;
    std::cout << "All tests PASSED!" << std::endl;
    std::cout << "========================================" << std::endl;
    
    return 0;
}
