// Regression test for compat_test partial_match_fail
// Reference: CI failure at ref 7da3004c3d01b00f3298bcfc74156be2ff27388e
// This test reproduces pattern "^hello$" on input "hello world" and ensures
// compat_test reports the partial_match_fail subtest as PASS.

#include <iostream>
#include <regex>
#include <string>
#include <cstdlib>
#include <cstdio>

int main() {
    const std::string pattern = "^hello$";
    const std::string input = "hello world";

    // Check std::regex behavior first
    // Note: Using regex_match to match the behavior of compat_test's "match" operation
    std::regex re(pattern);
    std::smatch sm;
    bool std_match = std::regex_match(input, sm, re);

    std::cout << "std::regex_match result: " << (std_match ? "matched" : "no match") << "\n";
    std::cout << "std::regex capture count: " << (std_match ? sm.size() : 0) << "\n";
    if (std_match) {
        for (size_t i = 0; i < sm.size(); ++i) {
            std::cout << "  capture[" << i << "]=\"" << sm[i].str() << "\"\n";
        }
    }

    // Invoke compat_test binary and check for partial_match_fail result.
    // The test expects to run from the build directory where compat_test is located
#ifdef _WIN32
    const char* cmd = ".\\compat_test.exe 2>&1";
    std::FILE* fp = _popen(cmd, "r");
#else
    const char* cmd = "./compat_test 2>&1";
    std::FILE* fp = popen(cmd, "r");
#endif
    if (!fp) {
        std::cerr << "Failed to run compat_test binary using command: " << cmd << "\n";
        std::cerr << "Note: This test expects to run from the build directory.\n";
        return 2;
    }

    bool found_partial = false;
    bool partial_passed = false;
    bool in_partial_test = false;
    char buf[4096];
    while (fgets(buf, sizeof(buf), fp)) {
        std::string line(buf);
        std::cout << line; // forward compat_test output to CI logs
        
        // Check if we found the partial_match_fail test
        if (line.find("partial_match_fail") != std::string::npos) {
            found_partial = true;
            in_partial_test = true;
        } else if (in_partial_test) {
            // If we're in the partial_match_fail test section, check for PASS/FAIL
            if (line.find("✅ PASS") != std::string::npos) {
                partial_passed = true;
                in_partial_test = false; // Done checking this test
            } else if (line.find("❌ FAIL") != std::string::npos) {
                in_partial_test = false; // Done checking this test, it failed
            } else if (line.find("=== Test:") != std::string::npos) {
                // We've moved to a different test without seeing a PASS/FAIL
                // This means partial_match_fail didn't output a result, which is an error
                in_partial_test = false;
            }
        }
    }
#ifdef _WIN32
    int rc = _pclose(fp);
#else
    int rc = pclose(fp);
#endif
    if (rc == -1) {
        std::cerr << "Error while waiting for compat_test to finish\n";
        return 3;
    }

    if (!found_partial) {
        std::cerr << "compat_test did not mention partial_match_fail; ensure compat_test binary is present and prints the test name.\n";
        return 4;
    }
    if (!partial_passed) {
        std::cerr << "partial_match_fail did not pass in compat_test output; regression detected.\n";
        return 5;
    }

    std::cout << "compat_regression_partial_match: OK\n";
    return 0;
}
