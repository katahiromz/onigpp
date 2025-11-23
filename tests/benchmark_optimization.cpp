// benchmark_optimization.cpp --- Simple benchmark to verify optimization
// Author: katahiromz
// License: BSD-2-Clause
#include "onigpp.h"
#include <iostream>
#include <string>
#include <list>
#include <vector>
#include <chrono>
#include <regex>

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

template<typename Container>
long long benchmark_regex_search(const Container& subject, const op::regex& re, int iterations) {
	using iter_type = typename Container::const_iterator;
	op::match_results<iter_type> m;
	
	auto start = std::chrono::high_resolution_clock::now();
	
	for (int i = 0; i < iterations; ++i) {
		op::regex_search(subject.begin(), subject.end(), m, re);
	}
	
	auto end = std::chrono::high_resolution_clock::now();
	return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
}

int main() {
	// --- Measures to avoid garbled characters on Windows consoles ---
#ifdef _WIN32
	// Switch to UTF-8 mode
	//_setmode(_fileno(stdout), _O_U8TEXT);
	// Ensure console uses UTF-8 code page for interoperability
	SetConsoleOutputCP(CP_UTF8);
#else
	// For Linux/Mac, setting the locale is usually sufficient
	std::setlocale(LC_ALL, "");
#endif

#ifndef USE_STD_FOR_TESTS
	// Oniguruma initialization
	op::auto_init init;
#endif

	std::cout << "===== Optimization Benchmark =====" << std::endl;
	
	// Create test data
	std::string subject_str = "Hello World 123 test 456 more 789";
	
	// Test with C-string (pointer - should use fast path)
	std::vector<char> subject_vec(subject_str.begin(), subject_str.end());
	std::list<char> subject_list(subject_str.begin(), subject_str.end());
	
	op::regex re("\\d+");
	const int iterations = 10000;
	
	// Benchmark with pointer (const char* - optimized path)
	{
		op::match_results<const char*> m;
		auto start = std::chrono::high_resolution_clock::now();
		for (int i = 0; i < iterations; ++i) {
			op::regex_search(subject_str.c_str(), subject_str.c_str() + subject_str.size(), m, re);
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
