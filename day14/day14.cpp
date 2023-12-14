// c++ -Wall -Wextra -pedantic -O3 -std=c++20 day14.cpp -o day14 && time ./day14 day14in.txt 1
// vi:set noet ts=4 sw=4:
#include <algorithm>
#include <compare>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <numeric>
#include <optional>
#include <sstream>
#include <string>
#include <vector>
#include <utility>


enum class Direction {
	NORTH = 1,
	SOUTH,
	EAST,
	WEST
};

class RockPlatform {
	public:
	RockPlatform() : cols_(0U) { raw_.reserve(512); }
	RockPlatform(const RockPlatform& other) : raw_(other.raw_), cols_(other.cols_) { }
	// my clang ver apparently didn't implement <=> for std::string
	RockPlatform operator=(const RockPlatform& other) {
		raw_ = other.raw_;
		cols_ = other.cols_;
		return *this;
	}
	bool operator<(const RockPlatform& other) const {
		if (cols_ != other.cols_) { return cols_ < other.cols_; }
		else { return raw_ < other.raw_; }
	}
	bool operator==(const RockPlatform& other) const {
		return (cols_ == other.cols_) && (raw_ == other.raw_);
	}

	void clear() { cols_ = 0U; raw_.clear(); }
	bool empty() const { return raw_.empty(); }
	void append_map(const std::string&);
	void set_cols(size_t cc) { cols_ = cc; }
	size_t get_cols() const { return cols_; }
	size_t get_rows() const;
	std::string str() const;
	char get_pos(size_t xx, size_t yy) const;
	void set_pos(size_t xx, size_t yy, const char cc);
	void tilt(const Direction dir);
	uint64_t load(const Direction toward) const;

	protected:
	std::string raw_;
	size_t cols_;
};

void RockPlatform::append_map(const std::string& line) {
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

size_t RockPlatform::get_rows() const {
	if (cols_ == 0U) {
		return 0U;
	}
	size_t rows = raw_.size() / cols_;
	if (raw_.size() % cols_ != 0) {
		rows++;
	}
	return rows;
}

std::string RockPlatform::str() const {
	std::stringstream out;
	out << "map[" << get_cols() << "x" << get_rows() << "]:";
	for (size_t rs=0; rs<raw_.size(); rs+=cols_) {
		out << "\n  " << raw_.substr(rs, cols_);
	}
	return out.str();
}

char RockPlatform::get_pos(size_t xx, size_t yy) const {
	if ((xx < cols_) && (yy < get_rows())) {
		return raw_[yy*cols_ + xx];
	}
	else {
		std::cerr << "Error: invalid get-map position, " << xx << ", " << yy << std::endl;
		return '.';
	}
}

void RockPlatform::set_pos(const size_t xx, const size_t yy, const char cc) {
	if ((xx < cols_) && (yy < get_rows()))
	{
		raw_[yy*cols_ + xx] = cc;
	}
	else {
		std::cerr << "Error: invalid set-map position, " << xx << ", " << yy << std::endl;
	}
}

void RockPlatform::tilt(const Direction dir) {
	switch(dir) {
		case Direction::NORTH:
			for (size_t col=0; col<get_cols(); col++) {
				std::optional<size_t> first_empty_row;
				for (size_t row=0; row<get_rows(); row++) {
					const char here = get_pos(col, row);
					if (here == '#') {
						first_empty_row.reset();
					}
					else if ((! first_empty_row) && (here == '.')) {
						first_empty_row = row;
					}
					else if ((here == 'O') && first_empty_row) {
						set_pos(col, first_empty_row.value(), 'O');
						set_pos(col, row, '.');
						// rewind the scan to find the new, first-empty-row
						// alternatively, keep an empty-row queue and pop
						// off locations as they fill
						row = first_empty_row.value();
						first_empty_row.reset();
					}
				}
			}
			break;
		case Direction::SOUTH:
			for (size_t col=0; col<get_cols(); col++) {
				const size_t rsize = get_rows();
				std::optional<size_t> first_empty_row;
				for (size_t ridx=0; ridx<rsize; ridx++) {
					size_t row = rsize - ridx - 1;
					const char here = get_pos(col, row);
					if (here == '#') {
						first_empty_row.reset();
					}
					else if ((! first_empty_row) && (here == '.')) {
						first_empty_row = row;
					}
					else if ((here == 'O') && first_empty_row) {
						set_pos(col, first_empty_row.value(), 'O');
						set_pos(col, row, '.');
						ridx = rsize - first_empty_row.value() - 1;
						first_empty_row.reset();
					}
				}
			}
			break;
		case Direction::EAST:
			for (size_t row=0; row<get_rows(); row++) {
				std::optional<size_t> first_empty_col;
				for (size_t cidx=0; cidx<get_cols(); cidx++) {
					size_t col = get_cols() - cidx - 1;
					const char here = get_pos(col, row);
					if (here == '#') {
						first_empty_col.reset();
					}
					else if ((! first_empty_col) && (here == '.')) {
						first_empty_col = col;
					}
					else if ((here == 'O') && first_empty_col) {
						set_pos(first_empty_col.value(), row, 'O');
						set_pos(col, row, '.');
						cidx = get_cols() - first_empty_col.value() - 1;
						first_empty_col.reset();
					}
				}
			}
			break;
		case Direction::WEST:
			for (size_t row=0; row<get_rows(); row++) {
				std::optional<size_t> first_empty_col;
				for (size_t col=0; col<get_cols(); col++) {
					const char here = get_pos(col, row);
					if (here == '#') {
						first_empty_col.reset();
					}
					else if ((! first_empty_col) && (here == '.')) {
						first_empty_col = col;
					}
					else if ((here == 'O') && first_empty_col) {
						set_pos(first_empty_col.value(), row, 'O');
						set_pos(col, row, '.');
						col = first_empty_col.value();
						first_empty_col.reset();
					}
				}
			}
			break;
		default:
			std::cerr << "not implemented" << std::endl;
	}
}

uint64_t RockPlatform::load(const Direction toward) const {
	uint64_t load = 0;
	const size_t rsize = get_rows();
	switch(toward) {
		case Direction::NORTH:
			for (size_t row=0; row<rsize; row++) {
				for (size_t col=0; col<get_cols(); col++) {
					if (get_pos(col, row) == 'O') {
						load += rsize-row;
					}
				}
			}
			break;
		default:
			std::cerr << "not implemented" << std::endl;
	}
	return load;
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

int part2_answer(RockPlatform platform) {
	std::vector<RockPlatform> snapshots;
	snapshots.push_back(platform);
	size_t loop_size = 0;
	for (int ss=0;
			(ss<1000000000) && ((loop_size==0) || ((ss % loop_size) != (1000000000 % loop_size)));
			ss++)
	{
		platform.tilt(Direction::NORTH);
		platform.tilt(Direction::WEST);
		platform.tilt(Direction::SOUTH);
		platform.tilt(Direction::EAST);
		if (loop_size == 0) {
			snapshots.push_back(platform);
			for (size_t aa=0; aa<snapshots.size(); aa++) {
				for (size_t bb=aa+1; bb<snapshots.size(); bb++) {
					if (snapshots[aa] == snapshots[bb]) {
						loop_size = bb-aa;
					}
				}
			}
		}
	}
	return platform.load(Direction::NORTH);
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
	RockPlatform platform;
	for (const std::string& line : input) {
		if (line.empty()) { continue; }
		platform.append_map(line);
	}
	if (part == 2) {
		std::cout << "load=" << part2_answer(platform) << std::endl;
	}
	else {
		platform.tilt(Direction::NORTH);
		// std::cout << platform.str() << std::endl;
		std::cout << "load=" << platform.load(Direction::NORTH) << std::endl;
	}
	return 0;
}

/*
--- Day 14: Parabolic Reflector Dish ---
You reach the place where all of the mirrors were pointing: a massive parabolic reflector dish attached to the side of another large mountain.

The dish is made up of many small mirrors, but while the mirrors themselves are roughly in the shape of a parabolic reflector dish, each individual mirror seems to be pointing in slightly the wrong direction. If the dish is meant to focus light, all it's doing right now is sending it in a vague direction.

This system must be what provides the energy for the lava! If you focus the reflector dish, maybe you can go where it's pointing and use the light to fix the lava production.

Upon closer inspection, the individual mirrors each appear to be connected via an elaborate system of ropes and pulleys to a large metal platform below the dish. The platform is covered in large rocks of various shapes. Depending on their position, the weight of the rocks deforms the platform, and the shape of the platform controls which ropes move and ultimately the focus of the dish.

In short: if you move the rocks, you can focus the dish. The platform even has a control panel on the side that lets you tilt it in one of four directions! The rounded rocks (O) will roll when the platform is tilted, while the cube-shaped rocks (#) will stay in place. You note the positions of all of the empty spaces (.) and rocks (your puzzle input). For example:

O....#....
O.OO#....#
.....##...
OO.#O....O
.O.....O#.
O.#..O.#.#
..O..#O..O
.......O..
#....###..
#OO..#....
Start by tilting the lever so all of the rocks will slide north as far as they will go:

OOOO.#.O..
OO..#....#
OO..O##..O
O..#.OO...
........#.
..#....#.#
..O..#.O.O
..O.......
#....###..
#....#....
You notice that the support beams along the north side of the platform are damaged; to ensure the platform doesn't collapse, you should calculate the total load on the north support beams.

The amount of load caused by a single rounded rock (O) is equal to the number of rows from the rock to the south edge of the platform, including the row the rock is on. (Cube-shaped rocks (#) don't contribute to load.) So, the amount of load caused by each rock in each row is as follows:

OOOO.#.O.. 10
OO..#....#  9
OO..O##..O  8
O..#.OO...  7
........#.  6
..#....#.#  5
..O..#.O.O  4
..O.......  3
#....###..  2
#....#....  1
The total load is the sum of the load caused by all of the rounded rocks. In this example, the total load is 136.

Tilt the platform so that the rounded rocks all roll north. Afterward, what is the total load on the north support beams?

Your puzzle answer was 108792.

--- Part Two ---
The parabolic reflector dish deforms, but not in a way that focuses the beam. To do that, you'll need to move the rocks to the edges of the platform. Fortunately, a button on the side of the control panel labeled "spin cycle" attempts to do just that!

Each cycle tilts the platform four times so that the rounded rocks roll north, then west, then south, then east. After each tilt, the rounded rocks roll as far as they can before the platform tilts in the next direction. After one cycle, the platform will have finished rolling the rounded rocks in those four directions in that order.

Here's what happens in the example above after each of the first few cycles:

After 1 cycle:
.....#....
....#...O#
...OO##...
.OO#......
.....OOO#.
.O#...O#.#
....O#....
......OOOO
#...O###..
#..OO#....

After 2 cycles:
.....#....
....#...O#
.....##...
..O#......
.....OOO#.
.O#...O#.#
....O#...O
.......OOO
#..OO###..
#.OOO#...O

After 3 cycles:
.....#....
....#...O#
.....##...
..O#......
.....OOO#.
.O#...O#.#
....O#...O
.......OOO
#...O###.O
#.OOO#...O
This process should work if you leave it running long enough, but you're still worried about the north support beams. To make sure they'll survive for a while, you need to calculate the total load on the north support beams after 1000000000 cycles.

In the above example, after 1000000000 cycles, the total load on the north support beams is 64.

Run the spin cycle for 1000000000 cycles. Afterward, what is the total load on the north support beams?

Your puzzle answer was 99118.

Both parts of this puzzle are complete! They provide two gold stars: **
*/

