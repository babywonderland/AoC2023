// c++ -Wall -Wextra -pedantic -O3 -std=c++20 day10.cpp -o day10 && time ./day10 day10in.txt 1
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

// beware: our origin is top-left and our y-axis is inverted
// from the usual Cartesian (our positive is down)
class Position {
	public:
	int x;
	int y;
	Position() : x(0), y(0) { }
	Position(int xx, int yy) : x(xx), y(yy) { }
	auto operator<=>(const Position&) const = default;
	std::string str() const;
};

class Connections {
	public:
	enum Directions {
		N = 1U<<0,
		E = 1U<<1,
		S = 1U<<2,
		W = 1U<<3
	};
	unsigned char value;

	Connections() : value(0) { }
	explicit Connections(const char mapchar);
	char mapchar() const;
	bool connects_n() const { return (value & N) != 0; }
	bool connects_e() const { return (value & E) != 0; }
	bool connects_s() const { return (value & S) != 0; }
	bool connects_w() const { return (value & W) != 0; }
};

class PipeMap {
	public:
	PipeMap() : cols_(0U) { raw_.reserve(100*1024); }

	void append_map(const std::string&);
	void set_cols(size_t cc) { cols_ = cc; }
	size_t get_cols() const { return cols_; }
	size_t get_rows() const;
	std::string str() const;
	char get_pos(size_t xx, size_t yy) const;
	char get_pos(const Position& pos) const { return get_pos(pos.x, pos.y); }
	void set_pos(size_t xx, size_t yy, const char cc);
	void set_pos(const Position& pos, const char cc) { return set_pos(pos.x, pos.y, cc); }
	Position move(const Position& currently, const Position& whence) const;

	protected:
	std::string raw_;
	size_t cols_;
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

std::string Position::str() const {
	std::stringstream out;
	out << "(" << x << "," << y << ")";
	return out.str();
}
std::ostream& operator<<(std::ostream& out, const Position& pp) {
	out << pp.str();
	return out;
}

Connections::Connections(const char mapchar) : value(0) {
	if ((mapchar == '|') || (mapchar == 'L') || (mapchar == 'J')) {
		value |= N;
	}
	if ((mapchar == '-') || (mapchar == 'L') || (mapchar == 'F')) {
		value |= E;
	}
	if ((mapchar == '|') || (mapchar == '7') || (mapchar == 'F')) {
		value |= S;
	}
	if ((mapchar == '-') || (mapchar == 'J') || (mapchar == '7')) {
		value |= W;
	}
	if ((! value) && (mapchar != '.')) {
		std::cerr << "Warning: constructing empty connections for " << mapchar << std::endl;
	}
};

char Connections::mapchar() const {
	if (value == 0) { return '.'; }
	else if (value == Connections('|').value) { return '|'; }
	else if (value == Connections('-').value) { return '-'; }
	else if (value == Connections('L').value) { return 'L'; }
	else if (value == Connections('J').value) { return 'J'; }
	else if (value == Connections('7').value) { return '7'; }
	else if (value == Connections('F').value) { return 'F'; }
	else { return '?'; }
}

void PipeMap::append_map(const std::string& line) {
	if (raw_.empty()) {
		raw_ += line;
		set_cols(line.size());
	}
	else if ((! raw_.empty()) && (line.size() != cols_)) {
		std::cerr << "Warning: expected " << cols_  << "-long lines, but this line is "
			<< line.size() << ":\n  " << line << std::endl;
		std::string line2(line);
		line2.resize(cols_, '.');
		raw_ += line2;
	}
	else {
		raw_ += line;
	}
}

size_t PipeMap::get_rows() const {
	if (cols_ == 0U) {
		return 0U;
	}
	size_t rows = raw_.size() / cols_;
	if (raw_.size() % cols_ != 0) {
		rows++;
	}
	return rows;
}

std::string PipeMap::str() const {
	std::stringstream out;
	out << "map[" << get_cols() << "x" << get_rows() << "]:";
	for (size_t rs=0; rs<raw_.size(); rs+=cols_) {
		out << "\n  " << raw_.substr(rs, cols_);
	}
	return out.str();
}

char PipeMap::get_pos(size_t xx, size_t yy) const {
	if ((xx < cols_) && (yy < get_rows())) {
		return raw_[yy*cols_ + xx];
	}
	else {
		std::cerr << "Error: invalid get-map position, " << xx << ", " << yy << std::endl;
		return '.';
	}
}

void PipeMap::set_pos(const size_t xx, const size_t yy, const char cc) {
	if ((xx < cols_) && (yy < get_rows()))
	{
		raw_[yy*cols_ + xx] = cc;
	}
	else {
		std::cerr << "Error: invalid set-map position, " << xx << ", " << yy << std::endl;
	}
}

Position PipeMap::move(const Position& currently, const Position& whence) const {
	Connections choices(get_pos(currently));
	Position offset(0, 0);
	if (choices.connects_n() && (currently.y > 0) && (whence.y >= currently.y)) {
		offset.y = -1;
	}
	else if (choices.connects_e()
			&& (static_cast<unsigned int>(currently.x) < get_cols()-1)
			&& (whence.x <= currently.x))
	{
		offset.x = 1;
	}
	else if (choices.connects_s()
			&& (static_cast<unsigned int>(currently.y) < get_rows()-1)
			&& (whence.y <= currently.y))
	{
		offset.y = 1;
	}
	else if (choices.connects_w() && (currently.x > 0) && (whence.x >= currently.x)) {
		offset.x = -1;
	}
	else {
		std::cerr << "Error: cannot move " << get_pos(whence)
			<< "->" << get_pos(currently) << "->?? from " << currently << std::endl;
	}
	return Position(currently.x+offset.x, currently.y+offset.y);
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

std::pair<PipeMap, Position> build_map(const std::vector<std::string>& input) {
	PipeMap map;
	Position map_start;
	bool found_start = false;
	for (size_t aa=0; aa<input.size(); aa++) {
		if (input[aa].empty()) { continue; }
		if (const size_t spos=input[aa].find('S'); spos != input[aa].npos) {
			if (found_start) {
				std::cerr << "Warning: replacing prior start position "
					<< map_start << " with "
					<< Position(spos, aa) << std::endl;
			}
			map_start = Position(spos, aa);
			found_start = true;
		}
		map.append_map(input[aa]);
	}
	if (found_start) {
		Connections start_N;
		if (map_start.y > 0) {
			start_N = Connections(map.get_pos(map_start.x, map_start.y-1));
		}
		Connections start_E;
		if (static_cast<unsigned int>(map_start.x) < map.get_cols()-1) {
			start_E = Connections(map.get_pos(map_start.x+1, map_start.y));
		}
		Connections start_S;
		if (static_cast<unsigned int>(map_start.y) < map.get_rows()-1) {
			start_S = Connections(map.get_pos(map_start.x, map_start.y+1));
		}
		Connections start_W;
		if (map_start.x > 0) {
			start_W = Connections(map.get_pos(map_start.x-1, map_start.y));
		}
		char SS = '.';
		if (start_N.connects_s()) {
			if (start_E.connects_w()) { SS = 'L'; }
			else if (start_S.connects_n()) { SS = '|'; }
			else if (start_W.connects_e()) { SS = 'J'; }
		}
		else if (start_E.connects_w()) {
			if (start_S.connects_n()) { SS = 'F'; }
			else if (start_W.connects_e()) { SS = '-'; }
		}
		else if (start_S.connects_n() && start_W.connects_e()) { SS = '7'; }
		if (SS == '.') {
			std::cerr << "Warning: could not determine start connectivity" << std::endl;
		}
		else {
			map.set_pos(map_start, SS);
		}
	}
	return std::pair<PipeMap, Position>(std::move(map), std::move(map_start));
}

size_t part1_answer(const PipeMap& map, const Position& start) {
	size_t steps = 1;
	Position oneway[3]; // prev, curr, next
	Position otherway[3];
	oneway[0] = start;
	otherway[0] = start;
	oneway[1] = map.move(start, start);
	otherway[1] = map.move(start, oneway[1]);
	while ((oneway[1] != otherway[1]) && (oneway[0] != otherway[1])) {
		oneway[2] = map.move(oneway[1], oneway[0]);
		otherway[2] = map.move(otherway[1], otherway[0]);
		steps++;
		if (0) {
			std::cout << "step " << steps
				<< " 1w:" << oneway[0] << "(" << map.get_pos(oneway[0]) << ")->"
				<< oneway[1] << "(" << map.get_pos(oneway[1]) << ")->"
				<< oneway[2] << "(" << map.get_pos(oneway[2]) << ")  ow:"
				<< otherway[0] << "(" << map.get_pos(otherway[0]) << ")->"
				<< otherway[1] << "(" << map.get_pos(otherway[1]) << ")->"
				<< otherway[2] << "(" << map.get_pos(otherway[2]) << ")" << std::endl;
		}
		oneway[0] = oneway[1];
		otherway[0] = otherway[1];
		oneway[1] = oneway[2];
		otherway[1] = otherway[2];
	}
	if (oneway[0] == otherway[1]) {
		// one and other passed each other in between steps
		steps--;
	}
	return steps;
}

void clear_non_path(PipeMap& map, const Position& start) {
	std::string world(map.get_rows() * map.get_cols(), '.');
	auto world_set = [&world, &map](int xx, int yy, char cc) {
		world[yy*map.get_cols() + xx] = cc;
	};
	auto world_get = [&world, &map](int xx, int yy) {
		return world[yy*map.get_cols() + xx];
	};
	Position walk[3];
	walk[0] = start;
	world_set(start.x, start.y, 'P');
	walk[1] = map.move(start, start);
	world_set(walk[1].x, walk[1].y, 'P');
	while (walk[1] != start) {
		walk[2] = map.move(walk[1], walk[0]);
		world_set(walk[2].x, walk[2].y, 'P');
		walk[0] = walk[1];
		walk[1] = walk[2];
	}
	for (size_t xx=0; xx<map.get_cols(); xx++) {
		for (size_t yy=0; yy<map.get_rows(); yy++) {
			if (world_get(xx, yy) != 'P') {
				map.set_pos(xx, yy, '.');
			}
		}
	}
}

int part2_answer(PipeMap map, const Position& start) {
	// I cannot get perimiter-summed area to work, and I don't know why not
	clear_non_path(map, start);
	// std::cout << map.str() << std::endl;
	size_t enclosed_area = 0;
	for (size_t xx=0; xx<map.get_cols(); xx++) {
		bool inside = false;
		for (size_t yy=0; yy<map.get_rows(); yy++) {
			if (Connections(map.get_pos(xx, yy)).connects_e()) {
				inside = ! inside;
			}
			if (map.get_pos(xx, yy) == '.') {
				map.set_pos(xx, yy, inside ? 'I' : 'O');
				if (inside) {
					enclosed_area++;
				}
			}
		}
	}
	// std::cout << map.str() << std::endl;
	return enclosed_area;
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
	auto [map, start] = build_map(input);
	// std::cout << map.str() << std::endl;
	// std::cout << "start: " << start << " (" << map.get_pos(start) << ")" << std::endl;
	if (part == 2) {
		std::cout << part2_answer(map, start) << std::endl;
	}
	else {
		std::cout << part1_answer(map, start) << std::endl;
	}
	std::cout << "done" << std::endl;
	return 0;
}


/*
--- Day 10: Pipe Maze ---
You use the hang glider to ride the hot air from Desert Island all the way up to the floating metal island. This island is surprisingly cold and there definitely aren't any thermals to glide on, so you leave your hang glider behind.

You wander around for a while, but you don't find any people or animals. However, you do occasionally find signposts labeled "Hot Springs" pointing in a seemingly consistent direction; maybe you can find someone at the hot springs and ask them where the desert-machine parts are made.

The landscape here is alien; even the flowers and trees are made of metal. As you stop to admire some metal grass, you notice something metallic scurry away in your peripheral vision and jump into a big pipe! It didn't look like any animal you've ever seen; if you want a better look, you'll need to get ahead of it.

Scanning the area, you discover that the entire field you're standing on is densely packed with pipes; it was hard to tell at first because they're the same metallic silver color as the "ground". You make a quick sketch of all of the surface pipes you can see (your puzzle input).

The pipes are arranged in a two-dimensional grid of tiles:

| is a vertical pipe connecting north and south.
- is a horizontal pipe connecting east and west.
L is a 90-degree bend connecting north and east.
J is a 90-degree bend connecting north and west.
7 is a 90-degree bend connecting south and west.
F is a 90-degree bend connecting south and east.
. is ground; there is no pipe in this tile.
S is the starting position of the animal; there is a pipe on this tile, but your sketch doesn't show what shape the pipe has.
Based on the acoustics of the animal's scurrying, you're confident the pipe that contains the animal is one large, continuous loop.

For example, here is a square loop of pipe:

.....
.F-7.
.|.|.
.L-J.
.....
If the animal had entered this loop in the northwest corner, the sketch would instead look like this:

.....
.S-7.
.|.|.
.L-J.
.....
In the above diagram, the S tile is still a 90-degree F bend: you can tell because of how the adjacent pipes connect to it.

Unfortunately, there are also many pipes that aren't connected to the loop! This sketch shows the same loop as above:

-L|F7
7S-7|
L|7||
-L-J|
L|-JF
In the above diagram, you can still figure out which pipes form the main loop: they're the ones connected to S, pipes those pipes connect to, pipes those pipes connect to, and so on. Every pipe in the main loop connects to its two neighbors (including S, which will have exactly two pipes connecting to it, and which is assumed to connect back to those two pipes).

Here is a sketch that contains a slightly more complex main loop:

..F7.
.FJ|.
SJ.L7
|F--J
LJ...
Here's the same example sketch with the extra, non-main-loop pipe tiles also shown:

7-F7-
.FJ|7
SJLL7
|F--J
LJ.LJ
If you want to get out ahead of the animal, you should find the tile in the loop that is farthest from the starting position. Because the animal is in the pipe, it doesn't make sense to measure this by direct distance. Instead, you need to find the tile that would take the longest number of steps along the loop to reach from the starting point - regardless of which way around the loop the animal went.

In the first example with the square loop:

.....
.S-7.
.|.|.
.L-J.
.....
You can count the distance each tile in the loop is from the starting point like this:

.....
.012.
.1.3.
.234.
.....
In this example, the farthest point from the start is 4 steps away.

Here's the more complex loop again:

..F7.
.FJ|.
SJ.L7
|F--J
LJ...
Here are the distances for each tile on that loop:

..45.
.236.
01.78
14567
23...
Find the single giant loop starting at S. How many steps along the loop does it take to get from the starting position to the point farthest from the starting position?

Your puzzle answer was 7107.

--- Part Two ---
You quickly reach the farthest point of the loop, but the animal never emerges. Maybe its nest is within the area enclosed by the loop?

To determine whether it's even worth taking the time to search for such a nest, you should calculate how many tiles are contained within the loop. For example:

...........
.S-------7.
.|F-----7|.
.||.....||.
.||.....||.
.|L-7.F-J|.
.|..|.|..|.
.L--J.L--J.
...........
The above loop encloses merely four tiles - the two pairs of . in the southwest and southeast (marked I below). The middle . tiles (marked O below) are not in the loop. Here is the same loop again with those regions marked:

...........
.S-------7.
.|F-----7|.
.||OOOOO||.
.||OOOOO||.
.|L-7OF-J|.
.|II|O|II|.
.L--JOL--J.
.....O.....
In fact, there doesn't even need to be a full tile path to the outside for tiles to count as outside the loop - squeezing between pipes is also allowed! Here, I is still within the loop and O is still outside the loop:

..........
.S------7.
.|F----7|.
.||OOOO||.
.||OOOO||.
.|L-7F-J|.
.|II||II|.
.L--JL--J.
..........
In both of the above examples, 4 tiles are enclosed by the loop.

Here's a larger example:

.F----7F7F7F7F-7....
.|F--7||||||||FJ....
.||.FJ||||||||L7....
FJL7L7LJLJ||LJ.L-7..
L--J.L7...LJS7F-7L7.
....F-J..F7FJ|L7L7L7
....L7.F7||L7|.L7L7|
.....|FJLJ|FJ|F7|.LJ
....FJL-7.||.||||...
....L---J.LJ.LJLJ...
The above sketch has many random bits of ground, some of which are in the loop (I) and some of which are outside it (O):

OF----7F7F7F7F-7OOOO
O|F--7||||||||FJOOOO
O||OFJ||||||||L7OOOO
FJL7L7LJLJ||LJIL-7OO
L--JOL7IIILJS7F-7L7O
OOOOF-JIIF7FJ|L7L7L7
OOOOL7IF7||L7|IL7L7|
OOOOO|FJLJ|FJ|F7|OLJ
OOOOFJL-7O||O||||OOO
OOOOL---JOLJOLJLJOOO
In this larger example, 8 tiles are enclosed by the loop.

Any tile that isn't part of the main loop can count as being enclosed by the loop. Here's another example with many bits of junk pipe lying around that aren't connected to the main loop at all:

FF7FSF7F7F7F7F7F---7
L|LJ||||||||||||F--J
FL-7LJLJ||||||LJL-77
F--JF--7||LJLJ7F7FJ-
L---JF-JLJ.||-FJLJJ7
|F|F-JF---7F7-L7L|7|
|FFJF7L7F-JF7|JL---7
7-L-JL7||F7|L7F-7F7|
L.L7LFJ|||||FJL7||LJ
L7JLJL-JLJLJL--JLJ.L
Here are just the tiles that are enclosed by the loop marked with I:

FF7FSF7F7F7F7F7F---7
L|LJ||||||||||||F--J
FL-7LJLJ||||||LJL-77
F--JF--7||LJLJIF7FJ-
L---JF-JLJIIIIFJLJJ7
|F|F-JF---7IIIL7L|7|
|FFJF7L7F-JF7IIL---7
7-L-JL7||F7|L7F-7F7|
L.L7LFJ|||||FJL7||LJ
L7JLJL-JLJLJL--JLJ.L
In this last example, 10 tiles are enclosed by the loop.

Figure out whether you have time to search for the nest by calculating the area within the loop. How many tiles are enclosed by the loop?

Your puzzle answer was 281.

Both parts of this puzzle are complete! They provide two gold stars: **
*/
