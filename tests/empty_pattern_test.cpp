// tests/empty_pattern_test.cpp
// Verify behaviour for an empty regex pattern ("") against std::regex
// Consistent with the test style used in this repository.

#include "tests.h"
#include <regex>
#include <vector>
#include <iostream>
#include <cassert>

int main() {
    TESTS_OUTPUT_INIT();
    ONIGPP_TEST_INIT;

    // ---------- Test 1: regex_search with empty pattern on non-empty input ----------
    {
        std::string pattern = "";
        std::string input = "abc";

        // std::regex
        std::regex std_re(pattern);
        std::smatch std_m;
        bool std_found = std::regex_search(input, std_m, std_re);
        assert(std_found && "std::regex_search should find empty match in 'abc' with empty pattern");
        assert(std_m.size() >= 1);
        assert(std_m[0].str() == "" && "std::regex first match should be empty string");

        // onigpp
        myns::regex onig_re(pattern);
        myns::smatch onig_m;
        bool onig_found = myns::regex_search(input, onig_m, onig_re);
        assert(onig_found && "onigpp regex_search should find empty match in 'abc' with empty pattern");
        assert(onig_m.size() >= 1);
        assert(onig_m[0].str() == "" && "onigpp first match should be empty string");

        std::cout << "Test 1 passed\n";
    }

    // ---------- Test 2: regex_match behavior for empty/non-empty subjects ----------
    {
        std::string pattern = "";
        std::string input_nonempty = "abc";
        std::string input_empty = "";

        // std::regex_match
        std::regex std_re(pattern);
        std::smatch std_m;
        bool std_match_nonempty = std::regex_match(input_nonempty, std_m, std_re);
        bool std_match_empty = std::regex_match(input_empty, std_m, std_re);
        assert(!std_match_nonempty && "std::regex_match('abc','') must be false");
        assert(std_match_empty && "std::regex_match('', '') must be true");

        // onigpp
        myns::regex onig_re(pattern);
        myns::smatch onig_m;
        bool onig_match_nonempty = myns::regex_match(input_nonempty, onig_m, onig_re);
        bool onig_match_empty = myns::regex_match(input_empty, onig_m, onig_re);
        assert(!onig_match_nonempty && "onigpp regex_match('abc','') must be false");
        assert(onig_match_empty && "onigpp regex_match('', '') must be true");

        std::cout << "Test 2 passed\n";
    }

    // ---------- Test 3: regex_iterator basic comparison (counts & first positions) ----------
    {
        std::string pattern = "";
        std::string input = "ab";

        // std::iterator
        std::regex std_re(pattern);
        std::sregex_iterator std_it(input.begin(), input.end(), std_re);
        std::sregex_iterator std_end;
        std::vector<std::string> std_matches;
        for (; std_it != std_end; ++std_it) {
            std_matches.push_back((*std_it)[0].str());
        }

        // onigpp iterator
        myns::regex onig_re(pattern);
        auto onig_begin = myns::sregex_iterator(input.begin(), input.end(), onig_re);
        auto onig_end = myns::sregex_iterator();
        std::vector<std::string> onig_matches;
        for (auto it = onig_begin; it != onig_end; ++it) {
            onig_matches.push_back((*it)[0].str());
        }

        // Compare basic properties
        assert(std_matches.size() == onig_matches.size() && "match count with empty pattern should match between implementations");
        for (size_t i = 0; i < std_matches.size(); ++i) {
            assert(std_matches[i] == onig_matches[i] && "match contents should be identical");
        }

        std::cout << "Test 3 passed (found " << std_matches.size() << " matches)\n";
    }

    std::cout << "All empty-pattern tests passed\n";
    return 0;
}
