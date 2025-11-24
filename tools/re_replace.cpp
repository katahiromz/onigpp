// re_replace.cpp --- simple regex replacement tool
//
// Usage:
//   re_replace [-i] [-w] PATTERN REPLACEMENT [FILE...]
//
// If no FILE is provided, reads from stdin and writes to stdout.
// -i : case-insensitive matching
// -w : write changes back to each file (in-place)
// The program uses std::regex (ECMAScript) and std::regex_replace.

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#ifdef USE_STD_FOR_TOOLS
	#include <regex>
	namespace rex = std;
#else
	#include "onigpp.h"
	namespace rex = onigpp;
#endif

static void print_usage(const char* prog) {
	std::cerr << "Usage: " << prog << " [-i] [-w] PATTERN REPLACEMENT [FILE...]\n"
			  << "  -i	Case-insensitive matching\n"
			  << "  -w	Write changes in-place to each FILE (if FILE specified)\n"
			  << "If no FILE is given, read from stdin and write to stdout.\n";
}

static bool read_all(std::istream& in, std::string& out) {
	std::ostringstream ss;
	ss << in.rdbuf();
	if (!ss.good() && !in.eof()) return false;
	out = ss.str();
	return true;
}

int main(int argc, char* argv[]) {
#ifndef USE_STD_FOR_TOOLS
	rex::auto_init auto_init;
#endif

	if (argc < 3) {
		print_usage(argv[0]);
		return 2;
	}

	bool opt_icase = false;
	bool opt_inplace = false;
	int idx = 1;
	for (; idx < argc; ++idx) {
		std::string a = argv[idx];
		if (a.size() > 0 && a[0] == '-') {
			if (a == "-i") {
				opt_icase = true;
				continue;
			} else if (a == "-w") {
				opt_inplace = true;
				continue;
			} else if (a == "--") {
				++idx;
				break;
			} else {
				// Unknown flag
				std::cerr << "Unknown option: " << a << "\n";
				print_usage(argv[0]);
				return 2;
			}
		}
		break;
	}

	if (idx + 1 >= argc) {
		// Need at least PATTERN and REPLACEMENT
		print_usage(argv[0]);
		return 2;
	}

	const std::string pattern = argv[idx++];
	const std::string replacement = argv[idx++];

	std::vector<std::string> files;
	for (; idx < argc; ++idx) files.emplace_back(argv[idx]);

	rex::regex::flag_type flags = rex::regex::ECMAScript;
	if (opt_icase) flags = static_cast<rex::regex::flag_type>(flags | rex::regex::icase);

	rex::regex re;
	try {
		re = rex::regex(pattern, flags);
	} catch (const rex::regex_error& e) {
		std::cerr << "Failed to compile pattern: " << e.what() << "\n";
		return 3;
	}

	auto process_one = [&](const std::string& input, std::string* out_str) -> bool {
		try {
			*out_str = rex::regex_replace(input, re, replacement);
			return true;
		} catch (const rex::regex_error& e) {
			std::cerr << "Replacement failed: " << e.what() << "\n";
			return false;
		}
	};

	if (files.empty()) {
		// Read from stdin
		std::string input;
		if (!read_all(std::cin, input)) {
			std::cerr << "Failed to read from stdin\n";
			return 4;
		}
		std::string result;
		if (!process_one(input, &result)) return 5;
		std::cout << result;
		return 0;
	}

	int exit_code = 0;
	for (const auto& path : files) {
		std::ifstream ifs(path, std::ios::binary);
		if (!ifs) {
			std::cerr << "Failed to open file for reading: " << path << "\n";
			exit_code = 6;
			continue;
		}
		std::string input;
		if (!read_all(ifs, input)) {
			std::cerr << "Failed to read file: " << path << "\n";
			exit_code = 7;
			continue;
		}
		std::string result;
		if (!process_one(input, &result)) {
			exit_code = 8;
			continue;
		}
		if (opt_inplace) {
			std::ofstream ofs(path, std::ios::binary | std::ios::trunc);
			if (!ofs) {
				std::cerr << "Failed to open file for writing: " << path << "\n";
				exit_code = 9;
				continue;
			}
			ofs << result;
			if (!ofs.good()) {
				std::cerr << "Failed to write file: " << path << "\n";
				exit_code = 10;
				continue;
			}
		} else {
			// Write a separator between files? We'll just write the transformed content to stdout.
			std::cout << result;
		}
	}

	return exit_code;
}
