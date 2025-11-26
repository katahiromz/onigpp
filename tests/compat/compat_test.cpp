// compat_test.cpp --- Compatibility test between onigpp and std::regex
// Author: katahiromz
// License: BSD-2-Clause

#include "onigpp.h"
#include <regex>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cassert>

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

// Simple JSON parser for our specific format
// This is a minimal parser that handles our patterns.json structure
class SimpleJSONParser {
public:
	struct TestCase {
		std::string id;
		std::string description;
		std::string pattern;
		std::string flags;
		std::string match_flags;  // New field for match-time flags
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
	// Find the end of a JSON string value, handling escaped quotes
	static size_t findStringEnd(const std::string& content, size_t start) {
		size_t pos = start;
		while (pos < content.size()) {
			if (content[pos] == '"') {
				// Check if it's escaped
				size_t backslashCount = 0;
				size_t checkPos = pos;
				while (checkPos > start && content[checkPos - 1] == '\\') {
					backslashCount++;
					checkPos--;
				}
				// If even number of backslashes (or zero), the quote is not escaped
				if (backslashCount % 2 == 0) {
					return pos;
				}
			}
			pos++;
		}
		return std::string::npos;
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
				
				size_t fieldEnd = findStringEnd(content, fieldStart + 1);
				if (fieldEnd == std::string::npos) break;
				
				std::string fieldName = content.substr(fieldStart + 1, fieldEnd - fieldStart - 1);
				
				// Find the colon
				size_t colonPos = content.find(':', fieldEnd);
				if (colonPos == std::string::npos) break;
				
				// Find the value
				size_t valueStart = content.find('"', colonPos);
				if (valueStart == std::string::npos) break;
				
				size_t valueEnd = findStringEnd(content, valueStart + 1);
				if (valueEnd == std::string::npos) break;
				
				std::string value = content.substr(valueStart + 1, valueEnd - valueStart - 1);
				
				// Assign to appropriate field
				if (fieldName == "id") tc.id = value;
				else if (fieldName == "description") tc.description = value;
				else if (fieldName == "pattern") tc.pattern = value;
				else if (fieldName == "flags") tc.flags = value;
				else if (fieldName == "match_flags") tc.match_flags = value;
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
	
	// Parse match flags from a comma-separated string (e.g., "match_not_bow,match_continuous")
	static std::regex_constants::match_flag_type getStdMatchFlags(const std::string& match_flags) {
		std::regex_constants::match_flag_type result = std::regex_constants::match_default;
		if (match_flags.empty()) return result;
		
		// Simple comma-separated parsing
		size_t start = 0;
		while (start < match_flags.size()) {
			size_t end = match_flags.find(',', start);
			if (end == std::string::npos) end = match_flags.size();
			
			std::string flag = match_flags.substr(start, end - start);
			// Trim whitespace
			while (!flag.empty() && flag.front() == ' ') flag.erase(0, 1);
			while (!flag.empty() && flag.back() == ' ') flag.pop_back();
			
			if (flag == "match_not_bol") result |= std::regex_constants::match_not_bol;
			else if (flag == "match_not_eol") result |= std::regex_constants::match_not_eol;
			else if (flag == "match_not_bow") result |= std::regex_constants::match_not_bow;
			else if (flag == "match_not_eow") result |= std::regex_constants::match_not_eow;
			else if (flag == "match_continuous") result |= std::regex_constants::match_continuous;
			
			start = end + 1;
		}
		return result;
	}
	
	static onigpp::regex_constants::match_flag_type getOnigppMatchFlags(const std::string& match_flags) {
		onigpp::regex_constants::match_flag_type result = onigpp::regex_constants::match_default;
		if (match_flags.empty()) return result;
		
		// Simple comma-separated parsing
		size_t start = 0;
		while (start < match_flags.size()) {
			size_t end = match_flags.find(',', start);
			if (end == std::string::npos) end = match_flags.size();
			
			std::string flag = match_flags.substr(start, end - start);
			// Trim whitespace
			while (!flag.empty() && flag.front() == ' ') flag.erase(0, 1);
			while (!flag.empty() && flag.back() == ' ') flag.pop_back();
			
			if (flag == "match_not_bol") result |= onigpp::regex_constants::match_not_bol;
			else if (flag == "match_not_eol") result |= onigpp::regex_constants::match_not_eol;
			else if (flag == "match_not_bow") result |= onigpp::regex_constants::match_not_bow;
			else if (flag == "match_not_eow") result |= onigpp::regex_constants::match_not_eow;
			else if (flag == "match_continuous") result |= onigpp::regex_constants::match_continuous;
			
			start = end + 1;
		}
		return result;
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
		if (!tc.match_flags.empty()) {
			std::cout << "Match flags: " << tc.match_flags << std::endl;
		}

		MatchResult stdResult, onigppResult;
		bool stdSuccess = false, onigppSuccess = false;
		bool stdRegexNotSupported = false;
		std::string stdExceptionMsg;
		
		// Get match flags
		auto stdMatchFlags = FlagsMapper::getStdMatchFlags(tc.match_flags);
		auto onigppMatchFlags = FlagsMapper::getOnigppMatchFlags(tc.match_flags);

		// Run std::regex test
		try {
			std::regex stdRe(tc.pattern, FlagsMapper::getStdFlags(tc.flags));
			
			if (tc.operation == "match") {
				stdResult = runStdMatch(stdRe, tc.input, stdMatchFlags);
				stdSuccess = true;
			} else if (tc.operation == "search") {
				stdResult = runStdSearch(stdRe, tc.input, stdMatchFlags);
				stdSuccess = true;
			} else if (tc.operation == "replace") {
				stdResult = runStdReplace(stdRe, tc.input, tc.replace_template, stdMatchFlags);
				stdSuccess = true;
			}
		} catch (const std::regex_error& e) {
			stdExceptionMsg = e.what();
			stdRegexNotSupported = true;
		} catch (const std::exception& e) {
			// Generic exception from std::regex (not a regex_error)
			std::cout << "std::regex exception: " << e.what() << std::endl;
		}

		// Run onigpp test
		try {
			OnigEncoding enc = FlagsMapper::getEncoding(tc.encoding_hint);
			onigpp::basic_regex<char> onigppRe(tc.pattern, FlagsMapper::getOnigppFlags(tc.flags), enc);
			
			if (tc.operation == "match") {
				onigppResult = runOnigppMatch(onigppRe, tc.input, onigppMatchFlags);
				onigppSuccess = true;
			} else if (tc.operation == "search") {
				onigppResult = runOnigppSearch(onigppRe, tc.input, onigppMatchFlags);
				onigppSuccess = true;
			} else if (tc.operation == "replace") {
				onigppResult = runOnigppReplace(onigppRe, tc.input, tc.replace_template, onigppMatchFlags);
				onigppSuccess = true;
			}
		} catch (const std::exception& e) {
			std::cout << "onigpp exception: " << e.what() << std::endl;
		}

		// Compare results
		if (!stdSuccess || !onigppSuccess) {
			if (stdSuccess != onigppSuccess) {
				// One implementation threw exception
				if (stdRegexNotSupported && onigppSuccess) {
					// std::regex doesn't support the feature, but onigpp does - this is expected
					if (!stdExceptionMsg.empty()) {
						std::cout << "⚠️  SKIP: std::regex does not support this pattern (" << stdExceptionMsg << ")" << std::endl;
					} else {
						std::cout << "⚠️  SKIP: std::regex does not support this pattern" << std::endl;
					}
				} else {
					// Unexpected exception mismatch
					std::cout << "❌ FAIL: One implementation threw exception" << std::endl;
					failed++;
				}
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
	MatchResult runStdMatch(const std::regex& re, const std::string& input,
							std::regex_constants::match_flag_type match_flags = std::regex_constants::match_default) {
		MatchResult result;
		std::smatch match;
		result.matched = std::regex_match(input, match, re, match_flags);
		
		if (result.matched) {
			for (size_t i = 0; i < match.size(); ++i) {
				result.captures.push_back(match[i].str());
				result.positions.push_back({match.position(i), match.length(i)});
			}
		}
		
		return result;
	}

	MatchResult runStdSearch(const std::regex& re, const std::string& input,
							 std::regex_constants::match_flag_type match_flags = std::regex_constants::match_default) {
		MatchResult result;
		std::smatch match;
		result.matched = std::regex_search(input, match, re, match_flags);
		
		if (result.matched) {
			for (size_t i = 0; i < match.size(); ++i) {
				result.captures.push_back(match[i].str());
				result.positions.push_back({match.position(i), match.length(i)});
			}
		}
		
		return result;
	}

	MatchResult runStdReplace(const std::regex& re, const std::string& input, 
							  const std::string& replace_template,
							  std::regex_constants::match_flag_type match_flags = std::regex_constants::match_default) {
		MatchResult result;
		result.matched = true;
		result.replace_result = std::regex_replace(input, re, replace_template, match_flags);
		return result;
	}

	MatchResult runOnigppMatch(const onigpp::basic_regex<char>& re, const std::string& input,
							   onigpp::regex_constants::match_flag_type match_flags = onigpp::regex_constants::match_default) {
		MatchResult result;
		onigpp::match_results<std::string::const_iterator> match;
		result.matched = onigpp::regex_match(input, match, re, match_flags);
		
		// Verify ready() is true after match operation
		if (!match.ready()) {
			std::cerr << "ERROR: match_results not ready after regex_match!" << std::endl;
			std::abort();
		}
		
		if (result.matched) {
			for (size_t i = 0; i < match.size(); ++i) {
				result.captures.push_back(match[i].str());
				result.positions.push_back({match.position(i), match.length(i)});
			}
		}
		
		return result;
	}

	MatchResult runOnigppSearch(const onigpp::basic_regex<char>& re, const std::string& input,
								onigpp::regex_constants::match_flag_type match_flags = onigpp::regex_constants::match_default) {
		MatchResult result;
		onigpp::match_results<std::string::const_iterator> match;
		result.matched = onigpp::regex_search(input, match, re, match_flags);
		
		// Verify ready() is true after search operation
		if (!match.ready()) {
			std::cerr << "ERROR: match_results not ready after regex_search!" << std::endl;
			std::abort();
		}
		
		if (result.matched) {
			for (size_t i = 0; i < match.size(); ++i) {
				result.captures.push_back(match[i].str());
				result.positions.push_back({match.position(i), match.length(i)});
			}
		}
		
		return result;
	}

	MatchResult runOnigppReplace(const onigpp::basic_regex<char>& re, const std::string& input,
								  const std::string& replace_template,
								  onigpp::regex_constants::match_flag_type match_flags = onigpp::regex_constants::match_default) {
		MatchResult result;
		result.matched = true;
		result.replace_result = onigpp::regex_replace(input, re, replace_template, match_flags);
		return result;
	}

	bool compareResults(const MatchResult& stdResult, const MatchResult& onigppResult, 
					   const std::string& operation) {
		if (operation == "replace") {
			if (stdResult.replace_result != onigppResult.replace_result) {
				std::cout << "  Difference in replace result:" << std::endl;
				std::cout << "	std::regex:  \"" << stdResult.replace_result << "\"" << std::endl;
				std::cout << "	onigpp:	  \"" << onigppResult.replace_result << "\"" << std::endl;
				return false;
			}
			return true;
		}

		// For match and search operations
		if (stdResult.matched != onigppResult.matched) {
			std::cout << "  Difference in match result:" << std::endl;
			std::cout << "	std::regex:  " << (stdResult.matched ? "matched" : "not matched") << std::endl;
			std::cout << "	onigpp:	  " << (onigppResult.matched ? "matched" : "not matched") << std::endl;
			return false;
		}

		if (!stdResult.matched) {
			// Both didn't match, which is consistent
			return true;
		}

		// Compare capture count
		if (stdResult.captures.size() != onigppResult.captures.size()) {
			std::cout << "  Difference in capture count:" << std::endl;
			std::cout << "	std::regex:  " << stdResult.captures.size() << " captures" << std::endl;
			std::cout << "	onigpp:	  " << onigppResult.captures.size() << " captures" << std::endl;
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
				std::cout << "	std::regex:  \"" << stdResult.captures[i] << "\"" << std::endl;
				std::cout << "	onigpp:	  \"" << onigppResult.captures[i] << "\"" << std::endl;
				allMatch = false;
			}
			
			if (stdResult.positions[i] != onigppResult.positions[i]) {
				std::cout << "  Difference in position/length[" << i << "]:" << std::endl;
				std::cout << "	std::regex:  pos=" << stdResult.positions[i].first 
						 << " len=" << stdResult.positions[i].second << std::endl;
				std::cout << "	onigpp:	  pos=" << onigppResult.positions[i].first 
						 << " len=" << onigppResult.positions[i].second << std::endl;
				allMatch = false;
			}
		}

		return allMatch;
	}
};

int main(int argc, char* argv[]) {
	TESTS_OUTPUT_INIT();

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

	// Return non-zero if there are failures
	return tester.getFailedCount() > 0 ? 1 : 0;
}
