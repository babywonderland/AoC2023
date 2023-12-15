// c++ -Wall -Wextra -pedantic -O3 -std=c++20 dayNN.cpp -o dayNN && time ./dayNN dayNNin.txt 1
// vi:set noet ts=4 sw=4:
#include <algorithm>
#include <compare>
#include <concepts>
#include <fstream>
#include <functional>
#include <iostream>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>
#include <utility>

// beware: our origin is top-left and our y-axis is inverted
// from the usual Cartesian (our positive is down)
class Position {
	public:
	uint64_t x;
	uint64_t y;
	Position() : x(0), y(0) { }
	Position(uint64_t xx, uint64_t yy) : x(xx), y(yy) { }
	auto operator<=>(const Position&) const = default;
	std::string str() const;
};
std::string Position::str() const {
	std::stringstream out;
	out << "(" << x << ", " << y << ")";
	return out.str();
}

template<typename TT>
concept has_str = requires(const TT& obj) { obj.str(); };

template<typename TT>
concept printable = requires(std::ostream& out, const TT& obj) { out << obj; };

template<typename CC>
concept container = requires(const CC& cobj) {
	{ cobj.begin() } -> std::same_as<typename CC::const_iterator>;
	{ cobj.end() } -> std::same_as<typename CC::const_iterator>;
};

template<typename CC, typename VV = typename CC::value_type>
concept str_container = requires(CC cobj, VV vobj) {
	requires container<CC>;
	requires has_str<VV>;
	requires std::same_as<std::remove_cvref_t<typename CC::value_type>, VV>;
	requires std::same_as<std::remove_cvref_t<decltype(cobj[0])>, VV>;
};

template<typename CC, typename VV = typename CC::value_type>
concept printable_container = requires(CC cobj, VV vobj) {
	requires container<CC>;
	requires printable<VV>;
	requires std::same_as<std::remove_cvref_t<typename CC::value_type>, VV>;
	requires std::same_as<std::remove_cvref_t<decltype(cobj[0])>, VV>;
};

void dump(const printable_container auto& input) {
	for (size_t aa=0; aa<input.size(); aa++) {
		std::cout << aa << ": " << input[aa] << std::endl;
	}
}
void dump(const str_container auto& input) {
	for (size_t aa=0; aa<input.size(); aa++) {
		std::cout << aa << ": " << input[aa].str() << std::endl;
	}
}

std::string join(const str_container auto& input, const std::string& delim=" ") {
	std::stringstream out;
	bool first = true;
	for (const auto& aa : input) {
		if (first) { first = false; }
		else { out << delim; }
		out << aa.str();
	}
	return out.str();
}
std::string join(const printable_container auto& input, const std::string& delim=" ") {
	std::stringstream out;
	bool first = true;
	for (const auto& aa : input) {
		if (first) { first = false; }
		else { out << delim; }
		out << aa;
	}
	return out.str();
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



