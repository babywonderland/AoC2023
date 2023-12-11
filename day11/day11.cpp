// c++ -Wall -Wextra -pedantic -O3 -std=c++20 day11.cpp -o day11 && time ./day11 day11in.txt 1
// vi:set noet ts=4 sw=4:
#include <algorithm>
#include <compare>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <numeric>
#include <set>
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
	out << "(" << x << "," << y << ")";
	return out.str();
}
std::ostream& operator<<(std::ostream& out, const Position& pp) {
	out << pp.str();
	return out;
}

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

std::pair<uint64_t, uint64_t> universe_dimensions(std::vector<Position>& gaxmap) {
	uint64_t max_x = 0;
	uint64_t max_y = 0;
	for (const Position& pp : gaxmap) {
		max_x = std::max(max_x, pp.x);
		max_y = std::max(max_y, pp.y);
	}
	return {max_x, max_y};
}

void expand_universe(std::vector<Position>& gaxmap, int part) {
	std::sort(gaxmap.begin(), gaxmap.end());
	auto [xmax, ymax] = universe_dimensions(gaxmap);
	auto inc_x_above = [&gaxmap, part](uint64_t threshold) -> void {
		for (auto pi=gaxmap.rbegin(); pi!=gaxmap.rend(); ++pi) {
			if (pi->x > threshold) {
				if (part == 1) {
					pi->x++;
				}
				else {
					pi->x += 999999U;
				}
			}
			else {
				break;
			}
		}
	};
	uint64_t gap_counter = xmax;
	for (auto pi=gaxmap.rbegin(); pi!=gaxmap.rend(); /*no-op*/) {
		while ((gap_counter > 0) && (gap_counter > pi->x)) {
			inc_x_above(gap_counter);
			gap_counter--;
		}
		while ((pi!=gaxmap.rend()) && (pi->x == gap_counter)) {
			++pi;
		}
		if (gap_counter > 0) {
			gap_counter--;
		}
	}
	auto inc_y_above = [&gaxmap, part](uint64_t threshold) -> void {
		for (Position& pp : gaxmap) {
			if (pp.y > threshold) {
				if (part == 1) {
					pp.y++;
				}
				else {
					pp.y += 999999U;
				}
			}
		}
	};
	std::set<uint64_t> yvals;
	for (const Position& pp : gaxmap) {
		yvals.insert(pp.y);
	}
	for (uint64_t ry=0; ry<ymax; ry++) {
		uint64_t yy = ymax-ry;
		if (! yvals.contains(yy)) {
			inc_y_above(yy);
		}
	}
}

std::vector<Position> find_galaxies(const std::vector<std::string>& input) {
	std::vector<Position> galaxies;
	size_t cols = 0;
	for (size_t yy=0; yy<input.size(); yy++) {
		if (input[yy].empty()) { continue; }
		if (cols == 0) {
			cols = input[yy].size();
		}
		else if (cols != input[yy].size()) {
			std::cerr << "Warning: line " << yy+1 << " length mismatch" << std::endl;
		}
		for (size_t xx=0; xx<input[yy].size(); xx++) {
			if (input[yy][xx] == '#') {
				galaxies.push_back(Position(xx, yy));
			}
			else if (input[yy][xx] == '.') {
				; // no-op
			}
			else {
				std::cerr << "Warning: unknown char " << input[yy][xx]
					<< " on line " << yy+1 << std::endl;
			}
		}
	}
	return galaxies;
}

uint64_t get_answer(const std::vector<Position>& galaxies) {
	std::vector<std::pair<Position, Position>> gaxpairs;
	for (size_t aa=0; aa<galaxies.size(); aa++) {
		for (size_t bb=aa+1; bb<galaxies.size(); bb++) {
			gaxpairs.push_back({galaxies[aa], galaxies[bb]});
		}
	}
	return std::transform_reduce(gaxpairs.begin(), gaxpairs.end(), 0ULL, std::plus<>(),
		[](const auto& gp) {
			return (std::max(gp.first.x, gp.second.x) - std::min(gp.first.x, gp.second.x))
				+  (std::max(gp.first.y, gp.second.y) - std::min(gp.first.y, gp.second.y));
		});
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
	std::vector<Position> galaxies = find_galaxies(input);
	expand_universe(galaxies, part);
	// auto udim = universe_dimensions(galaxies);
	// std::cout << "galaxies: " << udim.first << "x" << udim.second << std::endl;
	// dump(galaxies);
	std::cout << get_answer(galaxies) << std::endl;
	return 0;
}

/*
--- Day 11: Cosmic Expansion ---
You continue following signs for "Hot Springs" and eventually come across an observatory. The Elf within turns out to be a researcher studying cosmic expansion using the giant telescope here.

He doesn't know anything about the missing machine parts; he's only visiting for this research project. However, he confirms that the hot springs are the next-closest area likely to have people; he'll even take you straight there once he's done with today's observation analysis.

Maybe you can help him with the analysis to speed things up?

The researcher has collected a bunch of data and compiled the data into a single giant image (your puzzle input). The image includes empty space (.) and galaxies (#). For example:

...#......
.......#..
#.........
..........
......#...
.#........
.........#
..........
.......#..
#...#.....
The researcher is trying to figure out the sum of the lengths of the shortest path between every pair of galaxies. However, there's a catch: the universe expanded in the time it took the light from those galaxies to reach the observatory.

Due to something involving gravitational effects, only some space expands. In fact, the result is that any rows or columns that contain no galaxies should all actually be twice as big.

In the above example, three columns and two rows contain no galaxies:

   v  v  v
 ...#......
 .......#..
 #.........
>..........<
 ......#...
 .#........
 .........#
>..........<
 .......#..
 #...#.....
   ^  ^  ^
These rows and columns need to be twice as big; the result of cosmic expansion therefore looks like this:

....#........
.........#...
#............
.............
.............
........#....
.#...........
............#
.............
.............
.........#...
#....#.......
Equipped with this expanded universe, the shortest path between every pair of galaxies can be found. It can help to assign every galaxy a unique number:

....1........
.........2...
3............
.............
.............
........4....
.5...........
............6
.............
.............
.........7...
8....9.......
In these 9 galaxies, there are 36 pairs. Only count each pair once; order within the pair doesn't matter. For each pair, find any shortest path between the two galaxies using only steps that move up, down, left, or right exactly one . or # at a time. (The shortest path between two galaxies is allowed to pass through another galaxy.)

For example, here is one of the shortest paths between galaxies 5 and 9:

....1........
.........2...
3............
.............
.............
........4....
.5...........
.##.........6
..##.........
...##........
....##...7...
8....9.......
This path has length 9 because it takes a minimum of nine steps to get from galaxy 5 to galaxy 9 (the eight locations marked # plus the step onto galaxy 9 itself). Here are some other example shortest path lengths:

Between galaxy 1 and galaxy 7: 15
Between galaxy 3 and galaxy 6: 17
Between galaxy 8 and galaxy 9: 5
In this example, after expanding the universe, the sum of the shortest path between all 36 pairs of galaxies is 374.

Expand the universe, then find the length of the shortest path between every pair of galaxies. What is the sum of these lengths?

Your puzzle answer was 9684228.

--- Part Two ---
The galaxies are much older (and thus much farther apart) than the researcher initially estimated.

Now, instead of the expansion you did before, make each empty row or column one million times larger. That is, each empty row should be replaced with 1000000 empty rows, and each empty column should be replaced with 1000000 empty columns.

(In the example above, if each empty row or column were merely 10 times larger, the sum of the shortest paths between every pair of galaxies would be 1030. If each empty row or column were merely 100 times larger, the sum of the shortest paths between every pair of galaxies would be 8410. However, your universe will need to expand far beyond these values.)

Starting with the same initial image, expand the universe according to these new rules, then find the length of the shortest path between every pair of galaxies. What is the sum of these lengths?

Your puzzle answer was 483844716556.

Both parts of this puzzle are complete! They provide two gold stars: **
*/

