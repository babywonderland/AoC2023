// c++ -Wall -Wextra -pedantic -O3 -std=c++20 day01.cpp -o day01 && time ./day01 day01in.txt
// vi:set noet ts=4 sw=4:
#include <algorithm>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <numeric>
// #include <ranges> // my clang doesn't support :-(
#include <string>
#include <vector>
#include <utility>

void dump(const std::vector<std::string>& input) {
	for (size_t aa=0; aa<input.size(); aa++) {
		std::cout << aa << ": >>" << input[aa] << "<<" << std::endl;
	}
}
void dump(const std::vector<std::pair<int, int>>& input) {
	for (size_t aa=0; aa<input.size(); aa++) {
		std::cout << aa << ": " << input[aa].first << " " << input[aa].second << std::endl;
	}
}
void dump(const std::vector<int>& input) {
	for (size_t aa=0; aa<input.size(); aa++) {
		std::cout << aa << ": " << input[aa] << std::endl;
	}
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

int digit_to_int(const char cc) {
	int result = -1;
	switch (cc) {
		case '1': return 1;
		case '2': return 2;
		case '3': return 3;
		case '4': return 4;
		case '5': return 5;
		case '6': return 6;
		case '7': return 7;
		case '8': return 8;
		case '9': return 9;
		case '0': return 0;
	}
	return result;
}

std::vector<std::pair<int, int>> first_last_digit_v1(const std::vector<std::string>& input) {
	std::vector<std::pair<int, int>> result;
	for (const std::string& line : input) {
		if (line.empty()) { continue; }
		std::pair<int, int> first_last;
		bool have_first = false;
		for (const char& cc : line) {
			if (std::isdigit(static_cast<unsigned char>(cc))) {
				if (have_first) {
					first_last.second = digit_to_int(cc);
				}
				else {
					first_last.first = digit_to_int(cc);
					first_last.second = digit_to_int(cc);
					have_first = true;
				}
			}
		}
		result.push_back(std::move(first_last));
	}
	return result;
}

template <typename TT>
std::vector<size_t> key_lengths(const TT& input_map) {
	std::vector<size_t> lengths;
	for (const auto& [k,v] : input_map) {
		lengths.push_back(k.size());
	}
	std::sort(lengths.begin(), lengths.end());
	lengths.erase(std::unique(lengths.begin(), lengths.end()), lengths.end());
	return lengths;
}

std::vector<std::pair<int, int>> first_last_digit_v2(const std::vector<std::string>& input) {
	std::vector<std::pair<int, int>> result;
	static std::map<std::string, int, std::less<>> digitmap;
	static std::vector<size_t> digit_lengths;
	if (digitmap.empty()) {
		// ought to make this constexpr
		digitmap["one"] = 1;
		digitmap["two"] = 2;
		digitmap["three"] = 3;
		digitmap["four"] = 4;
		digitmap["five"] = 5;
		digitmap["six"] = 6;
		digitmap["seven"] = 7;
		digitmap["eight"] = 8;
		digitmap["nine"] = 9;
		digit_lengths = key_lengths(digitmap);
	}
	for (const std::string& line : input) {
		std::pair<int, int> first_last(0, 0);
		bool have_first = false;
		for (size_t cc=0; cc<line.size(); cc++) {
			int digit = -1;
			// std::cout << "cc=" << cc << " " << line.c_str()+cc << std::endl;
			// all the number-words are 3, 4, or 5 chars
			const char* line_c = line.c_str();
			for (auto len : digit_lengths) {
				if (auto diter = digitmap.find(
							std::string_view(line_c+cc, std::min(len, line.size()-cc)));
						diter != digitmap.end())
				{
					digit = diter->second;
					break;
				}
			}
			if (digit < 0) {
				digit = digit_to_int(line[cc]);
			}
			if (digit > 0) {
				if (have_first) {
					first_last.second = digit;
				}
				else {
					first_last.first = digit;
					first_last.second = digit;
					have_first = true;
				}
			}
		}
		if (have_first) {
			result.push_back(std::move(first_last));
		}
	}
	return result;
}

std::vector<int> combine_digits(const std::vector<std::pair<int, int>>& input) {
	std::vector<int> result;
	std::transform(input.begin(), input.end(), std::back_inserter(result),
		[](const auto& pp) { return 10*pp.first + pp.second; });
	return result;
}

int main(int argc, char* argv[]) {
	std::vector<std::string> input;
	if (argc > 1) {
		input = get_input(argv[1]);
	}
	else {
		input = get_input();
	}
	// std::vector<int> cal_values = combine_digits(first_last_digit_v1(input));
	std::vector<int> cal_values = combine_digits(first_last_digit_v2(input));
	// dump(first_last_digit_v2(input));
	// dump(cal_values);
	std::cout << std::reduce(cal_values.begin(), cal_values.end()) << std::endl;
	return 0;
}



/*

--- Day 1: Trebuchet?! ---
Something is wrong with global snow production, and you've been selected to take a look. The Elves have even given you a map; on it, they've used stars to mark the top fifty locations that are likely to be having problems.

You've been doing this long enough to know that to restore snow operations, you need to check all fifty stars by December 25th.

Collect stars by solving puzzles. Two puzzles will be made available on each day in the Advent calendar; the second puzzle is unlocked when you complete the first. Each puzzle grants one star. Good luck!

You try to ask why they can't just use a weather machine ("not powerful enough") and where they're even sending you ("the sky") and why your map looks mostly blank ("you sure ask a lot of questions") and hang on did you just say the sky ("of course, where do you think snow comes from") when you realize that the Elves are already loading you into a trebuchet ("please hold still, we need to strap you in").

As they're making the final adjustments, they discover that their calibration document (your puzzle input) has been amended by a very young Elf who was apparently just excited to show off her art skills. Consequently, the Elves are having trouble reading the values on the document.

The newly-improved calibration document consists of lines of text; each line originally contained a specific calibration value that the Elves now need to recover. On each line, the calibration value can be found by combining the first digit and the last digit (in that order) to form a single two-digit number.

For example:

1abc2
pqr3stu8vwx
a1b2c3d4e5f
treb7uchet
In this example, the calibration values of these four lines are 12, 38, 15, and 77. Adding these together produces 142.

Consider your entire calibration document. What is the sum of all of the calibration values?

Your puzzle answer was 55002.

--- Part Two ---
Your calculation isn't quite right. It looks like some of the digits are actually spelled out with letters: one, two, three, four, five, six, seven, eight, and nine also count as valid "digits".

Equipped with this new information, you now need to find the real first and last digit on each line. For example:

two1nine
eightwothree
abcone2threexyz
xtwone3four
4nineeightseven2
zoneight234
7pqrstsixteen
In this example, the calibration values are 29, 83, 13, 24, 42, 14, and 76. Adding these together produces 281.

What is the sum of all of the calibration values?

Your puzzle answer was 55093.

Both parts of this puzzle are complete! They provide two gold stars: **

*/
