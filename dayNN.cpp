// c++ -Wall -Wextra -pedantic -O3 -std=c++20 dayNN.cpp -o dayNN && time ./dayNN dayNNin.txt
// vi:set noet ts=4 sw=4:
#include <algorithm>
#include <compare>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>
#include <utility>


void dump(const std::vector<std::string>& input) {
	for (size_t aa=0; aa<input.size(); aa++) {
		std::cout << aa << ": >>" << input[aa] << "<<" << std::endl;
	}
}
template <typename TT>
typename std::enable_if_t<! std::is_integral<TT>::value> dump(const std::vector<TT>& input) {
	for (size_t aa=0; aa<input.size(); aa++) {
		std::cout << aa << ": " << input[aa].str() << std::endl;
	}
}
template <typename TT>
typename std::enable_if_t<std::is_integral<TT>::value> dump(const std::vector<TT>& input) {
	for (size_t aa=0; aa<input.size(); aa++) {
		std::cout << aa << ": " << input[aa] << std::endl;
	}
}

template <typename TT>
typename std::enable_if_t<! std::is_integral<TT>::value, std::string>
join(const std::vector<TT>& input, const std::string& delim=" ") {
	std::string result;
	bool first = true;
	for (const auto& aa : input) {
		if (first) {
			first = false;
		}
		else {
			result += delim;
		}
		result += aa.str();
	}
	return result;
}
template <typename TT>
typename std::enable_if_t<std::is_integral<TT>::value, std::string>
join(const std::vector<TT>& input, const std::string& delim=" ") {
	std::string result;
	bool first = true;
	for (const auto& aa : input) {
		if (first) {
			first = false;
		}
		else {
			result += delim;
		}
		result += std::to_string(aa);
	}
	return result;
}
template <>
std::string join(const std::vector<std::string>& input, const std::string& delim) {
	std::string result;
	bool first = true;
	for (const auto& aa : input) {
		if (first) {
			first = false;
		}
		else {
			result += delim;
		}
		result += aa;
	}
	return result;
}

std::vector<std::string> split(const std::string& input, const std::string& delim=" ") {
	std::vector<std::string> results;
	size_t pos = 0;
	size_t ppos = pos;
	while ((pos = input.find(delim, ppos)) != input.npos) {
		results.emplace_back(input.substr(ppos, pos-ppos));
		ppos = pos + delim.size();
	}
	if (ppos < input.size()) {
		results.emplace_back(input.substr(ppos));
	}
	return results;
}

std::string str_tolower(std::string input) {
	std::transform(input.begin(), input.end(), input.begin(),
		[](unsigned char cc) { return std::tolower(cc); });
	return input;
}

std::vector<std::string> get_input() {
	std::vector<std::string> input;
	while (std::cin.good()) {
		std::string one_line;
		std::getline(std::cin, one_line);
		input.push_back(std::move(one_line));
	}
	return input;
}

std::vector<std::string> get_input(const std::string& file) {
	std::vector<std::string> input;
	std::ifstream instream(file);
	if (! instream.is_open()) {
		std::cerr << "file " << file << " did not open" << std::endl;
		return {};
	}
	while (instream.good()) {
		std::string one_line;
		std::getline(instream, one_line);
		input.push_back(std::move(one_line));
	}
	return input;
}

int part1_answer() {
	return 0;
}

int part2_answer() {
	return 0;
}

int main(int argc, char* argv[]) {
	std::vector<std::string> input;
	int part = 2;
	if (argc > 2) {
		try {
			part = std::stoi(argv[2]);
		}
		catch (...) {
			std::cerr << "Error: invalid part " << argv[2] << "; must be 1 or 2" << std::endl;
			return 1;
		}
		input = get_input(argv[1]);
	}
	else if (argc > 1) {
		try {
			part = std::stoi(argv[1]);
		}
		catch (...) {
			std::cerr << "Error: invalid part " << argv[1] << "; must be 1 or 2" << std::endl;
			return 1;
		}
		input = get_input();
	}
	else {
		std::cerr << "Usage: " << argv[0] << " [input-file] <1|2>" << std::endl;
		return 1;
	}
	if (! ((part == 1) || (part == 2))) {
		std::cerr << "Error: invalid part " << part << "; must be 1 or 2" << std::endl;
		return 1;
	}
	if (part == 2) {
		std::cout << part2_answer() << std::endl;
	}
	else {
		std::cout << part1_answer() << std::endl;
	}
	std::cout << "done" << std::endl;
	return 0;
}



