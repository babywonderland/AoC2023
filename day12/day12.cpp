// c++ -Wall -Wextra -pedantic -O3 -std=c++20 day12.cpp -o day12 && time ./day12 day12in.txt 1
// vi:set noet ts=4 sw=4:
#include <algorithm>
#include <compare>
#include <deque>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>
#include <utility>

class SpringRecord {
	public:
	std::string statuses;
	std::deque<unsigned int> cdblocks;

	static std::string badblock(size_t len);

	SpringRecord(const std::string&, int);
	std::string str() const;
	std::vector<std::string> resolve_statuses() const;
	uint64_t resolve_statuses_count() const;
};

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
template <typename TT>
std::string join (const std::deque<TT>& input, const std::string& delim=" ") {
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

std::string SpringRecord::badblock(size_t len) {
	std::string bbstr;
	bbstr.reserve(len+1);
	for (size_t aa=0; aa<len; aa++) { bbstr += '#'; }
	return bbstr;
}

SpringRecord::SpringRecord(const std::string& line, int part) {
	auto lwords = split(line);
	if (lwords.size() != 2) {
		std::cerr << "Error: invalid input line, " << line << std::endl;
		return;
	}
	if (part == 2) {
		statuses = join(std::vector<std::string>(5, lwords[0]), "?");
	}
	else {
		statuses = lwords[0];
	}
	for (const std::string& num : split(lwords[1], ",")) {
		try {
			cdblocks.push_back(std::stoul(num));
		}
		catch (...) {
			std::cerr << "Error: invalid input number " << num << std::endl;
		}
		if (cdblocks.back() < 0) {
			std::cerr << "Error: invalid negative input number " << num << std::endl;
			cdblocks.pop_back();
		}
	}
	if (part == 2) {
		std::deque<unsigned int> copy(cdblocks);
		for (int ii=0; ii<4; ii++) {
			cdblocks.insert(cdblocks.end(), copy.begin(), copy.end());
		}
	}
}

std::string SpringRecord::str() const {
	return std::string("SpringRecord{") + statuses + ", [" + join(cdblocks, ",") + "]}";
}

// ref_str is status, may contain ? and be longer than test_str
// candidate must not contain ? and is being tested for match against ref_str
bool matches(const std::string_view& ref_str, const std::string_view& candidate) {
	for (size_t aa=0; aa<candidate.size(); aa++) {
		if (aa >= ref_str.size()) {
			return false;
		}
		switch (candidate[aa]) {
			case '#':
				if ((ref_str[aa] != '#') && (ref_str[aa] != '?')) { return false; }
				break;
			case '.':
				if ((ref_str[aa] != '.') && (ref_str[aa] != '?')) { return false; }
				break;
			default:
				std::cerr << "Error: invalid status char in " << candidate << std::endl;
				return false;
		}
	}
	return true;
}

std::vector<std::string> SpringRecord::resolve_statuses() const {
	auto fit_next_bad = [this](std::string partial, std::deque<unsigned int> badlist) {
		auto impl = [this](auto& self, std::string partial, std::deque<unsigned int> badlist) {
			if (badlist.empty()) {
				for (size_t matchpos=partial.size(); matchpos<statuses.size(); matchpos++) {
					switch (statuses[matchpos]) {
						case '#':
							// not a match
							return std::vector<std::string>();
						case '?':
						case '.':
							partial += ".";
							break;
						default:
							std::cerr << "Invalid status char in " << statuses << std::endl;
							return std::vector<std::string>();
					}
				}
				return std::vector<std::string>(1U, partial);
			}
			std::vector<std::string> solutions;
			std::string dstr = badblock(badlist.front());
			badlist.pop_front();
			for (size_t matchpos=partial.size();
					(matchpos<=statuses.size()-dstr.size());
					matchpos++)
			{
				if (statuses[matchpos] == '.') {
					partial += '.';
					continue;
				}
				bool dmatch = matches(statuses.substr(matchpos), dstr);
				std::vector<std::string> rcsol;
				if (dmatch && statuses[matchpos+dstr.size()] != '#') {
					rcsol = self(self, partial+dstr+".", badlist);
					solutions.insert(solutions.end(), rcsol.begin(), rcsol.end());
				}
				else if (dmatch && matchpos+dstr.size() == statuses.size()) {
					rcsol = self(self, partial+dstr, badlist);
					solutions.insert(solutions.end(), rcsol.begin(), rcsol.end());
				}
				if (statuses[matchpos] == '#') {
					// cannot proceed further without matching a dstr
					break;
				}
				else if (statuses[matchpos] == '?') {
					// can continue matching
					partial += ".";
				}
			}
			return solutions;
		};
		return impl(impl, partial, badlist);
	};
	std::vector<std::string> solutions = fit_next_bad("", cdblocks);
	// std::cout << "solutions(" << solutions.size() << ") for " << str()
	// 	<< ":\n  " << join(solutions, "\n  ") << std::endl;
	return solutions;
}

uint64_t SpringRecord::resolve_statuses_count() const {
	// this cache is the difference between being able to run in ~1s
	// or taking hours, if not years
	std::map<std::string, uint64_t> comp_cache;
	auto fit_next_bad = [this, &comp_cache](std::string partial, std::deque<unsigned int> badlist) {
		auto impl = [this, &comp_cache](
				auto& self, std::string partial, std::deque<unsigned int> badlist) -> uint64_t
		{
			if (badlist.empty()) {
				for (size_t matchpos=partial.size(); matchpos<statuses.size(); matchpos++) {
					switch (statuses[matchpos]) {
						case '#':
							// not a match
							return 0ULL;
						case '?':
						case '.':
							partial += ".";
							break;
						default:
							std::cerr << "Invalid status char in " << statuses << std::endl;
							return 0ULL;
					}
				}
				return 1ULL;
			}
			auto cache_iter = comp_cache.find(std::to_string(partial.size()) + join(badlist, " "));
			if (cache_iter != comp_cache.end()) {
				return cache_iter->second;
			}
			uint64_t count = 0;
			std::string dstr = badblock(badlist.front());
			badlist.pop_front();
			for (size_t matchpos=partial.size();
					(matchpos<=statuses.size()-dstr.size());
					matchpos++)
			{
				if (statuses[matchpos] == '.') {
					partial += '.';
					continue;
				}
				bool dmatch = matches(statuses.substr(matchpos), dstr);
				if (dmatch && statuses[matchpos+dstr.size()] != '#') {
					std::string next_partial = partial+dstr+".";
					uint64_t ans_recurse =  self(self, next_partial, badlist);
					std::string ckey = std::to_string(next_partial.size())+join(badlist, " ");
					comp_cache[ckey] = ans_recurse;
					count += ans_recurse;
				}
				else if (dmatch && matchpos+dstr.size() == statuses.size()) {
					std::string next_partial = partial+dstr;
					uint64_t ans_recurse =  self(self, next_partial, badlist);
					std::string ckey = std::to_string(next_partial.size())+join(badlist, " ");
					comp_cache[ckey] = ans_recurse;
					count += ans_recurse;
				}
				if (statuses[matchpos] == '#') {
					// cannot proceed further without matching a dstr
					break;
				}
				else if (statuses[matchpos] == '?') {
					// can continue matching
					partial += ".";
				}
			}
			return count;
		};
		return impl(impl, partial, badlist);
	};
	return fit_next_bad("", cdblocks);
}

std::vector<SpringRecord> parse_records(const std::vector<std::string>& input, int part) {
	std::vector<SpringRecord> results;
	for (const std::string& line : input) {
		if (line.empty()) { continue; }
		results.push_back(SpringRecord(line, part));
	}
	return results;
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

unsigned int part1_answer(const std::vector<SpringRecord>& springs) {
	return std::transform_reduce(springs.begin(), springs.end(), 0U, std::plus<>(),
		[](const auto& ss) { return ss.resolve_statuses().size(); });
}

uint64_t part2_answer(const std::vector<SpringRecord>& springs) {
	return std::transform_reduce(springs.begin(), springs.end(), 0ULL, std::plus<>(),
		[](const auto& ss) { return ss.resolve_statuses_count(); });
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
	std::vector<SpringRecord> springs = parse_records(input, part);
	// dump(springs);
	if (part == 2) {
		std::cout << part2_answer(springs) << std::endl;
	}
	else {
		std::cout << part1_answer(springs) << std::endl;
	}
	return 0;
}

/*
--- Day 12: Hot Springs ---
You finally reach the hot springs! You can see steam rising from secluded areas attached to the primary, ornate building.

As you turn to enter, the researcher stops you. "Wait - I thought you were looking for the hot springs, weren't you?" You indicate that this definitely looks like hot springs to you.

"Oh, sorry, common mistake! This is actually the onsen! The hot springs are next door."

You look in the direction the researcher is pointing and suddenly notice the massive metal helixes towering overhead. "This way!"

It only takes you a few more steps to reach the main gate of the massive fenced-off area containing the springs. You go through the gate and into a small administrative building.

"Hello! What brings you to the hot springs today? Sorry they're not very hot right now; we're having a lava shortage at the moment." You ask about the missing machine parts for Desert Island.

"Oh, all of Gear Island is currently offline! Nothing is being manufactured at the moment, not until we get more lava to heat our forges. And our springs. The springs aren't very springy unless they're hot!"

"Say, could you go up and see why the lava stopped flowing? The springs are too cold for normal operation, but we should be able to find one springy enough to launch you up there!"

There's just one problem - many of the springs have fallen into disrepair, so they're not actually sure which springs would even be safe to use! Worse yet, their condition records of which springs are damaged (your puzzle input) are also damaged! You'll need to help them repair the damaged records.

In the giant field just outside, the springs are arranged into rows. For each row, the condition records show every spring and whether it is operational (.) or damaged (#). This is the part of the condition records that is itself damaged; for some springs, it is simply unknown (?) whether the spring is operational or damaged.

However, the engineer that produced the condition records also duplicated some of this information in a different format! After the list of springs for a given row, the size of each contiguous group of damaged springs is listed in the order those groups appear in the row. This list always accounts for every damaged spring, and each number is the entire size of its contiguous group (that is, groups are always separated by at least one operational spring: #### would always be 4, never 2,2).

So, condition records with no unknown spring conditions might look like this:

#.#.### 1,1,3
.#...#....###. 1,1,3
.#.###.#.###### 1,3,1,6
####.#...#... 4,1,1
#....######..#####. 1,6,5
.###.##....# 3,2,1
However, the condition records are partially damaged; some of the springs' conditions are actually unknown (?). For example:

???.### 1,1,3
.??..??...?##. 1,1,3
?#?#?#?#?#?#?#? 1,3,1,6
????.#...#... 4,1,1
????.######..#####. 1,6,5
?###???????? 3,2,1
Equipped with this information, it is your job to figure out how many different arrangements of operational and broken springs fit the given criteria in each row.

In the first line (???.### 1,1,3), there is exactly one way separate groups of one, one, and three broken springs (in that order) can appear in that row: the first three unknown springs must be broken, then operational, then broken (#.#), making the whole row #.#.###.

The second line is more interesting: .??..??...?##. 1,1,3 could be a total of four different arrangements. The last ? must always be broken (to satisfy the final contiguous group of three broken springs), and each ?? must hide exactly one of the two broken springs. (Neither ?? could be both broken springs or they would form a single contiguous group of two; if that were true, the numbers afterward would have been 2,3 instead.) Since each ?? can either be #. or .#, there are four possible arrangements of springs.

The last line is actually consistent with ten different arrangements! Because the first number is 3, the first and second ? must both be . (if either were #, the first number would have to be 4 or higher). However, the remaining run of unknown spring conditions have many different ways they could hold groups of two and one broken springs:

?###???????? 3,2,1
.###.##.#...
.###.##..#..
.###.##...#.
.###.##....#
.###..##.#..
.###..##..#.
.###..##...#
.###...##.#.
.###...##..#
.###....##.#
In this example, the number of possible arrangements for each row is:

???.### 1,1,3 - 1 arrangement
.??..??...?##. 1,1,3 - 4 arrangements
?#?#?#?#?#?#?#? 1,3,1,6 - 1 arrangement
????.#...#... 4,1,1 - 1 arrangement
????.######..#####. 1,6,5 - 4 arrangements
?###???????? 3,2,1 - 10 arrangements
Adding all of the possible arrangement counts together produces a total of 21 arrangements.

For each row, count all of the different arrangements of operational and broken springs that meet the given criteria. What is the sum of those counts?

Your puzzle answer was 7195.

--- Part Two ---
As you look out at the field of springs, you feel like there are way more springs than the condition records list. When you examine the records, you discover that they were actually folded up this whole time!

To unfold the records, on each row, replace the list of spring conditions with five copies of itself (separated by ?) and replace the list of contiguous groups of damaged springs with five copies of itself (separated by ,).

So, this row:

.# 1
Would become:

.#?.#?.#?.#?.# 1,1,1,1,1
The first line of the above example would become:

???.###????.###????.###????.###????.### 1,1,3,1,1,3,1,1,3,1,1,3,1,1,3
In the above example, after unfolding, the number of possible arrangements for some rows is now much larger:

???.### 1,1,3 - 1 arrangement
.??..??...?##. 1,1,3 - 16384 arrangements
?#?#?#?#?#?#?#? 1,3,1,6 - 1 arrangement
????.#...#... 4,1,1 - 16 arrangements
????.######..#####. 1,6,5 - 2500 arrangements
?###???????? 3,2,1 - 506250 arrangements
After unfolding, adding all of the possible arrangement counts together produces 525152.

Unfold your condition records; what is the new sum of possible arrangement counts?

Your puzzle answer was 33992866292225.

Both parts of this puzzle are complete! They provide two gold stars: **
*/

