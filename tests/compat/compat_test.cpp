// compat_test.cpp --- Compatibility test between onigpp and std::regex
// Author: katahiromz
// License: BSD-2-Clause

#include "onigpp.h"
#include <regex>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <cassert>

// Simple JSON parser for our specific format
// This is a minimal parser that handles our patterns.json structure
class SimpleJSONParser {
public:
    struct TestCase {
        std::string id;
        std::string description;
        std::string pattern;
        std::string flags;
        std::string input;
        std::string operation;
        std::string replace_template;
        std::string encoding_hint;
    };

    static std::vector<TestCase> parseFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file: " + filename);
        }

        std::string content((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
        
        return parseContent(content);
    }

private:
    static std::string trim(const std::string& str) {
        size_t first = str.find_first_not_of(" \t\n\r");
        if (first == std::string::npos) return "";
        size_t last = str.find_last_not_of(" \t\n\r");
        return str.substr(first, last - first + 1);
    }

    static std::string unquote(const std::string& str) {
        std::string s = trim(str);
        if (s.size() >= 2 && s.front() == '"' && s.back() == '"') {
            // Handle escape sequences
            std::string result;
            for (size_t i = 1; i < s.size() - 1; ++i) {
                if (s[i] == '\\' && i + 1 < s.size() - 1) {
                    switch (s[i + 1]) {
                        case 'n': result += '\n'; break;
                        case 't': result += '\t'; break;
                        case 'r': result += '\r'; break;
                        case '\\': result += '\\'; break;
                        case '"': result += '"'; break;
                        default: result += s[i + 1]; break;
                    }
                    ++i;
                } else {
                    result += s[i];
                }
            }
            return result;
        }
        return s;
    }

    static std::vector<TestCase> parseContent(const std::string& content) {
        std::vector<TestCase> cases;
        size_t pos = 0;

        // Skip to the opening '['
        pos = content.find('[', pos);
        if (pos == std::string::npos) return cases;
        pos++;

        while (pos < content.size()) {
            // Find next object start
            pos = content.find('{', pos);
            if (pos == std::string::npos) break;
            pos++;

            TestCase tc;
            
            // Parse object fields
            while (pos < content.size()) {
                // Find next field or end of object
                size_t fieldStart = content.find('"', pos);
                size_t objEnd = content.find('}', pos);
                
                if (objEnd != std::string::npos && (fieldStart == std::string::npos || objEnd < fieldStart)) {
                    pos = objEnd + 1;
                    break;
                }
                
                if (fieldStart == std::string::npos) break;
                
                size_t fieldEnd = content.find('"', fieldStart + 1);
                if (fieldEnd == std::string::npos) break;
                
                std::string fieldName = content.substr(fieldStart + 1, fieldEnd - fieldStart - 1);
                
                // Find the colon
                size_t colonPos = content.find(':', fieldEnd);
                if (colonPos == std::string::npos) break;
                
                // Find the value
                size_t valueStart = content.find('"', colonPos);
                if (valueStart == std::string::npos) break;
                
                size_t valueEnd = content.find('"', valueStart + 1);
                if (valueEnd == std::string::npos) break;
                
                std::string value = content.substr(valueStart + 1, valueEnd - valueStart - 1);
                
                // Assign to appropriate field
                if (fieldName == "id") tc.id = value;
                else if (fieldName == "description") tc.description = value;
                else if (fieldName == "pattern") tc.pattern = value;
                else if (fieldName == "flags") tc.flags = value;
                else if (fieldName == "input") tc.input = value;
                else if (fieldName == "operation") tc.operation = value;
                else if (fieldName == "replace_template") tc.replace_template = value;
                else if (fieldName == "encoding_hint") tc.encoding_hint = value;
                
                pos = valueEnd + 1;
            }
            
            cases.push_back(tc);
        }

        return cases;
    }
};

// Map flags string to std::regex_constants and onigpp::regex_constants
class FlagsMapper {
public:
    static std::regex_constants::syntax_option_type getStdFlags(const std::string& flags) {
        if (flags == "ECMAScript") return std::regex_constants::ECMAScript;
        if (flags == "nosubs") return std::regex_constants::ECMAScript | std::regex_constants::nosubs;
        if (flags == "icase") return std::regex_constants::ECMAScript | std::regex_constants::icase;
        return std::regex_constants::ECMAScript;
    }

    static onigpp::regex_constants::syntax_option_type getOnigppFlags(const std::string& flags) {
        if (flags == "ECMAScript") return onigpp::regex_constants::ECMAScript;
        if (flags == "nosubs") return onigpp::regex_constants::ECMAScript | onigpp::regex_constants::nosubs;
        if (flags == "icase") return onigpp::regex_constants::ECMAScript | onigpp::regex_constants::icase;
        return onigpp::regex_constants::ECMAScript;
    }

    static OnigEncoding getEncoding(const std::string& hint) {
        if (hint == "UTF-8" || hint.empty()) return ONIG_ENCODING_UTF8;
        if (hint == "SJIS") return ONIG_ENCODING_SJIS;
        if (hint == "UTF-16LE") return ONIG_ENCODING_UTF16_LE;
        return ONIG_ENCODING_UTF8;
    }
};

// Result comparison structure
struct MatchResult {
    bool matched;
    std::vector<std::string> captures;
    std::vector<std::pair<size_t, size_t>> positions; // position and length
    std::string replace_result;
};

// Test executor
class CompatibilityTester {
private:
    int passed = 0;
    int failed = 0;

public:
    void runTest(const SimpleJSONParser::TestCase& tc) {
        std::cout << "\n=== Test: " << tc.id << " ===" << std::endl;
        std::cout << "Description: " << tc.description << std::endl;
        std::cout << "Pattern: " << tc.pattern << std::endl;
        std::cout << "Input: " << tc.input << std::endl;
        std::cout << "Operation: " << tc.operation << std::endl;

        MatchResult stdResult, onigppResult;
        bool stdSuccess = false, onigppSuccess = false;

        // Run std::regex test
        try {
            std::regex stdRe(tc.pattern, FlagsMapper::getStdFlags(tc.flags));
            
            if (tc.operation == "match") {
                stdResult = runStdMatch(stdRe, tc.input);
                stdSuccess = true;
            } else if (tc.operation == "search") {
                stdResult = runStdSearch(stdRe, tc.input);
                stdSuccess = true;
            } else if (tc.operation == "replace") {
                stdResult = runStdReplace(stdRe, tc.input, tc.replace_template);
                stdSuccess = true;
            }
        } catch (const std::exception& e) {
            std::cout << "std::regex exception: " << e.what() << std::endl;
        }

        // Run onigpp test
        try {
            OnigEncoding enc = FlagsMapper::getEncoding(tc.encoding_hint);
            onigpp::basic_regex<char> onigppRe(tc.pattern, FlagsMapper::getOnigppFlags(tc.flags), enc);
            
            if (tc.operation == "match") {
                onigppResult = runOnigppMatch(onigppRe, tc.input);
                onigppSuccess = true;
            } else if (tc.operation == "search") {
                onigppResult = runOnigppSearch(onigppRe, tc.input);
                onigppSuccess = true;
            } else if (tc.operation == "replace") {
                onigppResult = runOnigppReplace(onigppRe, tc.input, tc.replace_template);
                onigppSuccess = true;
            }
        } catch (const std::exception& e) {
            std::cout << "onigpp exception: " << e.what() << std::endl;
        }

        // Compare results
        if (!stdSuccess || !onigppSuccess) {
            if (stdSuccess != onigppSuccess) {
                std::cout << "❌ FAIL: One implementation threw exception" << std::endl;
                failed++;
            } else {
                std::cout << "⚠️  SKIP: Both implementations threw exceptions" << std::endl;
            }
            return;
        }

        bool testPassed = compareResults(stdResult, onigppResult, tc.operation);
        if (testPassed) {
            std::cout << "✅ PASS" << std::endl;
            passed++;
        } else {
            std::cout << "❌ FAIL" << std::endl;
            failed++;
        }
    }

    void printSummary() {
        std::cout << "\n========================================" << std::endl;
        std::cout << "Test Summary" << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << "Passed: " << passed << std::endl;
        std::cout << "Failed: " << failed << std::endl;
        std::cout << "Total:  " << (passed + failed) << std::endl;
        std::cout << "========================================" << std::endl;
    }

    int getFailedCount() const { return failed; }

private:
    MatchResult runStdMatch(const std::regex& re, const std::string& input) {
        MatchResult result;
        std::smatch match;
        result.matched = std::regex_match(input, match, re);
        
        if (result.matched) {
            for (size_t i = 0; i < match.size(); ++i) {
                result.captures.push_back(match[i].str());
                result.positions.push_back({match.position(i), match.length(i)});
            }
        }
        
        return result;
    }

    MatchResult runStdSearch(const std::regex& re, const std::string& input) {
        MatchResult result;
        std::smatch match;
        result.matched = std::regex_search(input, match, re);
        
        if (result.matched) {
            for (size_t i = 0; i < match.size(); ++i) {
                result.captures.push_back(match[i].str());
                result.positions.push_back({match.position(i), match.length(i)});
            }
        }
        
        return result;
    }

    MatchResult runStdReplace(const std::regex& re, const std::string& input, 
                              const std::string& replace_template) {
        MatchResult result;
        result.matched = true;
        result.replace_result = std::regex_replace(input, re, replace_template);
        return result;
    }

    MatchResult runOnigppMatch(const onigpp::basic_regex<char>& re, const std::string& input) {
        MatchResult result;
        onigpp::match_results<std::string::const_iterator> match;
        result.matched = onigpp::regex_match(input, match, re);
        
        if (result.matched) {
            for (size_t i = 0; i < match.size(); ++i) {
                result.captures.push_back(match[i].str());
                result.positions.push_back({match.position(i), match.length(i)});
            }
        }
        
        return result;
    }

    MatchResult runOnigppSearch(const onigpp::basic_regex<char>& re, const std::string& input) {
        MatchResult result;
        onigpp::match_results<std::string::const_iterator> match;
        result.matched = onigpp::regex_search(input, match, re);
        
        if (result.matched) {
            for (size_t i = 0; i < match.size(); ++i) {
                result.captures.push_back(match[i].str());
                result.positions.push_back({match.position(i), match.length(i)});
            }
        }
        
        return result;
    }

    MatchResult runOnigppReplace(const onigpp::basic_regex<char>& re, const std::string& input,
                                  const std::string& replace_template) {
        MatchResult result;
        result.matched = true;
        result.replace_result = onigpp::regex_replace(input, re, replace_template);
        return result;
    }

    bool compareResults(const MatchResult& stdResult, const MatchResult& onigppResult, 
                       const std::string& operation) {
        if (operation == "replace") {
            if (stdResult.replace_result != onigppResult.replace_result) {
                std::cout << "  Difference in replace result:" << std::endl;
                std::cout << "    std::regex:  \"" << stdResult.replace_result << "\"" << std::endl;
                std::cout << "    onigpp:      \"" << onigppResult.replace_result << "\"" << std::endl;
                return false;
            }
            return true;
        }

        // For match and search operations
        if (stdResult.matched != onigppResult.matched) {
            std::cout << "  Difference in match result:" << std::endl;
            std::cout << "    std::regex:  " << (stdResult.matched ? "matched" : "not matched") << std::endl;
            std::cout << "    onigpp:      " << (onigppResult.matched ? "matched" : "not matched") << std::endl;
            return false;
        }

        if (!stdResult.matched) {
            // Both didn't match, which is consistent
            return true;
        }

        // Compare capture count
        if (stdResult.captures.size() != onigppResult.captures.size()) {
            std::cout << "  Difference in capture count:" << std::endl;
            std::cout << "    std::regex:  " << stdResult.captures.size() << " captures" << std::endl;
            std::cout << "    onigpp:      " << onigppResult.captures.size() << " captures" << std::endl;
            return false;
        }

        // Verify positions array sizes match captures array sizes
        assert(stdResult.positions.size() == stdResult.captures.size());
        assert(onigppResult.positions.size() == onigppResult.captures.size());

        // Compare each capture
        bool allMatch = true;
        for (size_t i = 0; i < stdResult.captures.size(); ++i) {
            if (stdResult.captures[i] != onigppResult.captures[i]) {
                std::cout << "  Difference in capture[" << i << "]:" << std::endl;
                std::cout << "    std::regex:  \"" << stdResult.captures[i] << "\"" << std::endl;
                std::cout << "    onigpp:      \"" << onigppResult.captures[i] << "\"" << std::endl;
                allMatch = false;
            }
            
            if (stdResult.positions[i] != onigppResult.positions[i]) {
                std::cout << "  Difference in position/length[" << i << "]:" << std::endl;
                std::cout << "    std::regex:  pos=" << stdResult.positions[i].first 
                         << " len=" << stdResult.positions[i].second << std::endl;
                std::cout << "    onigpp:      pos=" << onigppResult.positions[i].first 
                         << " len=" << onigppResult.positions[i].second << std::endl;
                allMatch = false;
            }
        }

        return allMatch;
    }
};

int main(int argc, char* argv[]) {
    // Initialize oniguruma
    onigpp::auto_init onig_init;

    std::cout << "========================================" << std::endl;
    std::cout << "onigpp vs std::regex Compatibility Test" << std::endl;
    std::cout << "========================================" << std::endl;

    // Determine the path to patterns.json
    std::string jsonPath = "tests/compat/patterns.json";
    if (argc > 1) {
        jsonPath = argv[1];
    }

    // Parse test cases
    std::vector<SimpleJSONParser::TestCase> testCases;
    try {
        testCases = SimpleJSONParser::parseFile(jsonPath);
        std::cout << "Loaded " << testCases.size() << " test cases from " << jsonPath << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error parsing JSON: " << e.what() << std::endl;
        return 1;
    }

    // Run tests
    CompatibilityTester tester;
    for (const auto& tc : testCases) {
        tester.runTest(tc);
    }

    // Print summary
    tester.printSummary();

    return tester.getFailedCount() > 0 ? 1 : 0;
}
