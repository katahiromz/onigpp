// benchmark_optimization.cpp --- Simple benchmark to verify optimization
// Author: katahiromz
// License: BSD-2-Clause
#include "tests.h"
#include <list>
#include <chrono>

template<typename Container>
long long benchmark_regex_search(const Container& subject, const rex::regex& re, int iterations) {
	using iter_type = typename Container::const_iterator;
	rex::match_results<iter_type> m;
	
	auto start = std::chrono::high_resolution_clock::now();
	
	for (int i = 0; i < iterations; ++i) {
		rex::regex_search(subject.begin(), subject.end(), m, re);
	}
	
	auto end = std::chrono::high_resolution_clock::now();
	return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
}

int main() {
	TESTS_OUTPUT_INIT();

	// Oniguruma initialization (no-op for std::regex)
	ONIGPP_TEST_INIT;

	std::cout << "===== Optimization Benchmark =====" << std::endl;
	
	// Create test data
	std::string subject_str = "Hello World 123 test 456 more 789";
	
	// Test with C-string (pointer - should use fast path)
	std::vector<char> subject_vec(subject_str.begin(), subject_str.end());
	std::list<char> subject_list(subject_str.begin(), subject_str.end());
	
	rex::regex re("\\d+");
	const int iterations = 10000;
	
	// Benchmark with pointer (const char* - optimized path)
	{
		rex::match_results<const char*> m;
		auto start = std::chrono::high_resolution_clock::now();
		for (int i = 0; i < iterations; ++i) {
			rex::regex_search(subject_str.c_str(), subject_str.c_str() + subject_str.size(), m, re);
		}
		auto end = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
		std::cout << "Pointer (const char*): " << duration << " μs for " << iterations << " iterations" << std::endl;
		std::cout << "  -> Should use FAST PATH (no buffer copy)" << std::endl;
	}
	
	// Benchmark with std::list (non-contiguous - buffer copy path)
	{
		auto duration = benchmark_regex_search(subject_list, re, iterations);
		std::cout << "std::list: " << duration << " μs for " << iterations << " iterations" << std::endl;
		std::cout << "  -> Uses buffer copy (expected to be slower)" << std::endl;
	}
	
	std::cout << "\n===== Benchmark Complete =====" << std::endl;
	std::cout << "Note: Pointer-based search should be significantly faster" << std::endl;
	std::cout << "      as it avoids the buffer copy overhead." << std::endl;
	
	return 0;
}
