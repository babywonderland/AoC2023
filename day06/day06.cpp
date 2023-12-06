// c++ -Wall -Wextra -pedantic -O3 -std=c++20 day06.cpp -o day06 && time ./day06 day06in.txt 1
// vi:set noet ts=4 sw=4:
#include <algorithm>
#include <cmath>
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

class Race {
	public:
	uint64_t time;
	uint64_t record;
	std::string str() const {
		std::stringstream out;
		out << "{" << time << ", " << record << "}";
		return out.str();
	}
};

std::vector<Race> parse_races(const std::vector<std::string>& input, int part) {
	std::vector<uint64_t> times;
	if (str_tolower(input[0].substr(0,5)) == "time:") {
		std::vector<std::string> str_times = split(input[0].substr(5));
		if (part == 2) {
			try {
				times.push_back(std::stoull(join(str_times, "")));
			}
			catch (...) {
				std::cerr << "Warning: invalid time " << join(str_times, "") << std::endl;
			}
		}
		else {
			for (const std::string& tstr : str_times) {
				if (tstr.empty()) { continue; }
				try {
					times.push_back(std::stoull(tstr));
				}
				catch (...) {
					std::cerr << "Warning: invalid time " << tstr << std::endl;
				}
			}
		}
	}
	std::vector<uint64_t> records;
	if (str_tolower(input[1].substr(0,9)) == "distance:") {
		std::vector<std::string> str_records = split(input[1].substr(9));
		if (part == 2) {
			try {
				records.push_back(std::stoull(join(str_records, "")));
			}
			catch (...) {
				std::cerr << "Warning: invalid distance " << join(str_records, "") << std::endl;
			}
		}
		else {
			for (const std::string& srec : str_records) {
				if (srec.empty()) { continue; }
				try {
					records.push_back(std::stoull(srec));
				}
				catch (...) {
					std::cerr << "Warning: invalid distance " << srec << std::endl;
				}
			}
		}
	}
	std::vector<Race> results;
	if (times.size() != records.size()) {
		std::cerr << "Error: mismatching number of times (" << times.size()
			<< ") and distances (" << records.size() << ")" << std::endl;
		return results;
	}
	for (size_t aa=0; aa<times.size(); aa++) {
		results.push_back(Race{times[aa], records[aa]});
	}
	return results;
}

int part1_answer(std::vector<Race>& input) {
	std::vector<uint64_t> error_margins;
	std::transform(input.cbegin(), input.cend(), std::back_inserter(error_margins),
		[](const Race& rr) {
			double b = rr.time;
			double a = -1;
			double c = -double(rr.record);
			double intercept1 = (-b + std::sqrt(b*b - (4*a*c))) / (2*a);
			double intercept2 = (-b - std::sqrt(b*b - (4*a*c))) / (2*a);
			uint64_t hold_time_min = static_cast<uint64_t>(intercept1);
			// >= because we have to beat the record, not merely tie it
			if (intercept1 >= hold_time_min) {
				hold_time_min++;
			}
			uint64_t hold_time_max = static_cast<uint64_t>(intercept2);
			if (intercept2 <= hold_time_max) {
				hold_time_max--;
			}
			return hold_time_max - hold_time_min + 1;
		});
	// dump(error_margins);
	return std::reduce(error_margins.begin(), error_margins.end(), 1ULL, std::multiplies<>());
}

int part2_answer(std::vector<Race>& input) {
	if (input.size() != 1) {
		std::cerr << "Error: expected only one race" << std::endl;
		return 0;
	}
	double b = input[0].time;
	double a = -1;
	double c = -double(input[0].record);
	double intercept1 = (-b + std::sqrt(b*b - (4*a*c))) / (2*a);
	double intercept2 = (-b - std::sqrt(b*b - (4*a*c))) / (2*a);
	uint64_t hold_time_min = static_cast<uint64_t>(intercept1);
	// >= because we have to beat the record, not merely tie it
	if (intercept1 >= hold_time_min) {
		hold_time_min++;
	}
	uint64_t hold_time_max = static_cast<uint64_t>(intercept2);
	if (intercept2 <= hold_time_max) {
		hold_time_max--;
	}
	return hold_time_max - hold_time_min + 1;
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

	std::vector<Race> races = parse_races(input, part);
	// dump(races);
	if (part == 2) {
		std::cout << part2_answer(races) << std::endl;
	}
	else {
		std::cout << part1_answer(races) << std::endl;
	}
	std::cout << "done" << std::endl;
	return 0;
}

/*
--- Day 6: Wait For It ---
The ferry quickly brings you across Island Island. After asking around, you discover that there is indeed normally a large pile of sand somewhere near here, but you don't see anything besides lots of water and the small island where the ferry has docked.

As you try to figure out what to do next, you notice a poster on a wall near the ferry dock. "Boat races! Open to the public! Grand prize is an all-expenses-paid trip to Desert Island!" That must be where the sand comes from! Best of all, the boat races are starting in just a few minutes.

You manage to sign up as a competitor in the boat races just in time. The organizer explains that it's not really a traditional race - instead, you will get a fixed amount of time during which your boat has to travel as far as it can, and you win if your boat goes the farthest.

As part of signing up, you get a sheet of paper (your puzzle input) that lists the time allowed for each race and also the best distance ever recorded in that race. To guarantee you win the grand prize, you need to make sure you go farther in each race than the current record holder.

The organizer brings you over to the area where the boat races are held. The boats are much smaller than you expected - they're actually toy boats, each with a big button on top. Holding down the button charges the boat, and releasing the button allows the boat to move. Boats move faster if their button was held longer, but time spent holding the button counts against the total race time. You can only hold the button at the start of the race, and boats don't move until the button is released.

For example:

Time:      7  15   30
Distance:  9  40  200
This document describes three races:

The first race lasts 7 milliseconds. The record distance in this race is 9 millimeters.
The second race lasts 15 milliseconds. The record distance in this race is 40 millimeters.
The third race lasts 30 milliseconds. The record distance in this race is 200 millimeters.
Your toy boat has a starting speed of zero millimeters per millisecond. For each whole millisecond you spend at the beginning of the race holding down the button, the boat's speed increases by one millimeter per millisecond.

So, because the first race lasts 7 milliseconds, you only have a few options:

Don't hold the button at all (that is, hold it for 0 milliseconds) at the start of the race. The boat won't move; it will have traveled 0 millimeters by the end of the race.
Hold the button for 1 millisecond at the start of the race. Then, the boat will travel at a speed of 1 millimeter per millisecond for 6 milliseconds, reaching a total distance traveled of 6 millimeters.
Hold the button for 2 milliseconds, giving the boat a speed of 2 millimeters per millisecond. It will then get 5 milliseconds to move, reaching a total distance of 10 millimeters.
Hold the button for 3 milliseconds. After its remaining 4 milliseconds of travel time, the boat will have gone 12 millimeters.
Hold the button for 4 milliseconds. After its remaining 3 milliseconds of travel time, the boat will have gone 12 millimeters.
Hold the button for 5 milliseconds, causing the boat to travel a total of 10 millimeters.
Hold the button for 6 milliseconds, causing the boat to travel a total of 6 millimeters.
Hold the button for 7 milliseconds. That's the entire duration of the race. You never let go of the button. The boat can't move until you let go of the button. Please make sure you let go of the button so the boat gets to move. 0 millimeters.
Since the current record for this race is 9 millimeters, there are actually 4 different ways you could win: you could hold the button for 2, 3, 4, or 5 milliseconds at the start of the race.

In the second race, you could hold the button for at least 4 milliseconds and at most 11 milliseconds and beat the record, a total of 8 different ways to win.

In the third race, you could hold the button for at least 11 milliseconds and no more than 19 milliseconds and still beat the record, a total of 9 ways you could win.

To see how much margin of error you have, determine the number of ways you can beat the record in each race; in this example, if you multiply these values together, you get 288 (4 * 8 * 9).

Determine the number of ways you could beat the record in each race. What do you get if you multiply these numbers together?

Your puzzle answer was 1710720.

--- Part Two ---
As the race is about to start, you realize the piece of paper with race times and record distances you got earlier actually just has very bad kerning. There's really only one race - ignore the spaces between the numbers on each line.

So, the example from before:

Time:      7  15   30
Distance:  9  40  200
...now instead means this:

Time:      71530
Distance:  940200
Now, you have to figure out how many ways there are to win this single race. In this example, the race lasts for 71530 milliseconds and the record distance you need to beat is 940200 millimeters. You could hold the button anywhere from 14 to 71516 milliseconds and beat the record, a total of 71503 ways!

How many ways can you beat the record in this one much longer race?

Your puzzle answer was 35349468.

Both parts of this puzzle are complete! They provide two gold stars: **
*/

