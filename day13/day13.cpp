// c++ -Wall -Wextra -pedantic -O3 -std=c++20 day13.cpp -o day13 && time ./day13 day13in.txt 1
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

class Coordinate {
	public:
	size_t x;
	size_t y;
	Coordinate() : x(0), y(0) { }
	Coordinate(size_t _x, size_t _y) : x(_x), y(_y) { }
	auto operator<=>(const Coordinate&) const = default;
	std::string str() const {
		std::stringstream out;
		out << "(" << x << ", " << y << ")";
		return out.str();
	}
};

class MirroredTerrain {
	public:
	MirroredTerrain() : cols_(0U) { raw_.reserve(512); }

	void clear() { cols_ = 0U; raw_.clear(); }
	bool empty() const { return raw_.empty(); }
	void append_map(const std::string&);
	void set_cols(size_t cc) { cols_ = cc; }
	size_t get_cols() const { return cols_; }
	size_t get_rows() const;
	std::string str() const;
	char get_pos(size_t xx, size_t yy) const;
	void set_pos(size_t xx, size_t yy, const char cc);
	void toggle(size_t xx, size_t yy);
	// It would be nice to implement the scanline functions as ranges iterators
	std::string h_scanline(size_t yy) const;
	std::string v_scanline(size_t xx) const;
	// For these, see Manacher's Algorithm if O(n) is needed
	std::vector<size_t> search_h_axis() const;
	std::vector<size_t> search_v_axis() const;
	std::vector<Coordinate> h_smudge_candidates() const;
	std::vector<Coordinate> v_smudge_candidates() const;

	protected:
	std::string raw_;
	size_t cols_;
};

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

void MirroredTerrain::append_map(const std::string& line) {
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

size_t MirroredTerrain::get_rows() const {
	if (cols_ == 0U) {
		return 0U;
	}
	size_t rows = raw_.size() / cols_;
	if (raw_.size() % cols_ != 0) {
		rows++;
	}
	return rows;
}

std::string MirroredTerrain::str() const {
	std::stringstream out;
	out << "map[" << get_cols() << "x" << get_rows() << "]:";
	for (size_t rs=0; rs<raw_.size(); rs+=cols_) {
		out << "\n  " << raw_.substr(rs, cols_);
	}
	return out.str();
}

char MirroredTerrain::get_pos(size_t xx, size_t yy) const {
	if ((xx < cols_) && (yy < get_rows())) {
		return raw_[yy*cols_ + xx];
	}
	else {
		std::cerr << "Error: invalid get-map position, " << xx << ", " << yy << std::endl;
		return '.';
	}
}

void MirroredTerrain::set_pos(const size_t xx, const size_t yy, const char cc) {
	if ((xx < cols_) && (yy < get_rows()))
	{
		raw_[yy*cols_ + xx] = cc;
	}
	else {
		std::cerr << "Error: invalid set-map position, " << xx << ", " << yy << std::endl;
	}
}

void MirroredTerrain::toggle(size_t xx, size_t yy) {
	if ((xx < cols_) && (yy < get_rows()))
	{
		if (raw_[yy*cols_ + xx] == '.') {
			raw_[yy*cols_ + xx] = '#';
		}
		else {
			raw_[yy*cols_ + xx] = '.';
		}
	}
	else {
		std::cerr << "Error: invalid set-map position, " << xx << ", " << yy << std::endl;
	}
}

std::string MirroredTerrain::h_scanline(size_t yy) const {
	if (yy < get_rows()) {
		return raw_.substr(yy*cols_, cols_);
	}
	else {
		return "";
	}
}

std::string MirroredTerrain::v_scanline(size_t xx) const {
	if (xx >= cols_) {
		return "";
	}
	const size_t rsize = get_rows();
	std::string result;
	result.reserve(rsize);
	for (size_t aa=0; aa<rsize; aa++) {
		result += raw_[aa*cols_ + xx];
	}
	return result;
}

std::vector<size_t> MirroredTerrain::search_h_axis() const {
	// axis is considered to be before index, i.e. at answer-0.5
	const size_t rsize = get_rows();
	std::vector<size_t> answers;
	for (size_t center=1; center<rsize; center++) {
		bool reflects = true;
		for (size_t dd=0; (center > dd) && (center + dd < rsize); dd++) {
			if (h_scanline(center-dd-1) != h_scanline(center+dd)) {
				reflects = false;
				break;
			}
		}
		if (reflects) {
			answers.push_back(center);
		}
	}
	return answers;
}

std::vector<size_t> MirroredTerrain::search_v_axis() const {
	// axis is considered to be before index, i.e. at answer-0.5
	std::vector<size_t> answers;
	std::vector<std::string> v_scanlines;
	for (size_t col=0; col<get_cols(); col++) {
		v_scanlines.push_back(v_scanline(col));
	}
	for (size_t center=1; center<get_cols(); center++) {
		bool reflects = true;
		for (size_t dd=0; (center > dd) && (center+dd < get_cols()); dd++) {
			if (v_scanlines[center-dd-1] != v_scanlines[center+dd]) {
				reflects = false;
				break;
			}
		}
		if (reflects) {
			answers.push_back(center);
		}
	}
	return answers;
}

std::vector<Coordinate> MirroredTerrain::h_smudge_candidates() const {
	const size_t rsize = get_rows();
	std::vector<Coordinate> answers;
	for (size_t center=1; center<rsize; center++) {
		std::vector<Coordinate> diffs;
		for (size_t dd=0; (center > dd) && (center + dd < rsize); dd++) {
			// I should probably cache the diff count calculations
			const std::string h_cdu = h_scanline(center-dd-1);
			const std::string h_cdl = h_scanline(center+dd);
			for (size_t xx=0; xx<get_cols(); xx++) {
				if (h_cdu[xx] != h_cdl[xx]) {
					diffs.push_back(Coordinate{xx, center-dd-1});
					diffs.push_back(Coordinate{xx, center+dd});
					if (diffs.size() > 2) {
						break;
					}
				}
			}
			if (diffs.size() > 2) {
				break;
			}
		}
		if (diffs.size() == 2) {
			answers.insert(answers.end(), diffs.begin(), diffs.end());
		}
	}
	return answers;
}

std::vector<Coordinate> MirroredTerrain::v_smudge_candidates() const {
	const size_t rsize = get_rows();
	std::vector<Coordinate> answers;
	std::vector<std::string> v_scanlines;
	for (size_t col=0; col<get_cols(); col++) {
		v_scanlines.push_back(v_scanline(col));
	}
	for (size_t center=1; center<get_cols(); center++) {
		std::vector<Coordinate> diffs;
		for (size_t dd=0; (center > dd) && (center+dd < get_cols()); dd++) {
			// I should probably cache the diff count calculations
			for (size_t yy=0; yy<rsize; yy++) {
				if (v_scanlines[center-dd-1][yy] != v_scanlines[center+dd][yy]) {
					diffs.push_back(Coordinate{center-dd-1, yy});
					diffs.push_back(Coordinate{center+dd, yy});
					if (diffs.size() > 2) {
						break;
					}
				}
			}
			if (diffs.size() > 2) {
				break;
			}
		}
		if (diffs.size() == 2) {
			answers.insert(answers.end(), diffs.begin(), diffs.end());
		}
	}
	return answers;
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

std::vector<MirroredTerrain> parse_terrains(const std::vector<std::string>& input) {
	std::vector<MirroredTerrain> results;
	MirroredTerrain current;
	for (const std::string& line : input) {
		if (line.empty()) {
			if (! current.empty()) {
				results.push_back(std::move(current));
				current.clear();
				continue;
			}
		}
		current.append_map(line);
	}
	if (! current.empty()) {
		results.push_back(std::move(current));
	}
	return results;
}

size_t part1_answer(const std::vector<MirroredTerrain>& map) {
	bool log = false;
	return std::transform_reduce(map.begin(), map.end(), 0ULL, std::plus<>(),
		[log](const MirroredTerrain& mm) {
			const std::vector<size_t> v_mirrors = mm.search_v_axis();
			size_t vidx_sum = std::reduce(v_mirrors.begin(), v_mirrors.end());
			const std::vector<size_t> h_mirrors = mm.search_h_axis();
			size_t hidx_sum = std::reduce(h_mirrors.begin(), h_mirrors.end());
			if (log) {
				std::cout << "for " << mm.str() << std::endl
					<< "v: " << join(v_mirrors) << std::endl
					<< "h: " << join(h_mirrors) << std::endl;
			}
			return vidx_sum + 100*hidx_sum;
		});
}

size_t part2_answer(const std::vector<MirroredTerrain>& map) {
	// I admit, this is ugly
	bool log = false;
	return std::transform_reduce(map.begin(), map.end(), 0ULL, std::plus<>(),
		[log](const MirroredTerrain& mm) {
			std::vector<size_t> orig_v = mm.search_v_axis();
			std::vector<size_t> orig_h = mm.search_h_axis();
			std::sort(orig_v.begin(), orig_v.end());
			std::sort(orig_h.begin(), orig_h.end());
			std::vector<Coordinate> h_candidates = mm.h_smudge_candidates();
			std::vector<Coordinate> v_candidates = mm.v_smudge_candidates();
			std::sort(h_candidates.begin(), h_candidates.end());
			std::sort(v_candidates.begin(), v_candidates.end());
			if (log) {
				std::cout << "for " << mm.str() << std::endl
					<< "h_s: " << join(h_candidates) << std::endl
					<< "v_s: " << join(v_candidates) << std::endl;
			}
			std::vector<Coordinate> combined_candidates;
			std::set_union(h_candidates.begin(), h_candidates.end(),
				v_candidates.begin(), v_candidates.end(),
				std::back_inserter(combined_candidates));
			std::vector<size_t> modified_v;
			std::vector<size_t> modified_h;
			for (const Coordinate& cc : combined_candidates) {
				MirroredTerrain modified(mm);
				modified.toggle(cc.x, cc.y);
				std::vector<size_t> candidate_v = modified.search_v_axis();
				std::vector<size_t> candidate_h = modified.search_h_axis();
				if (((! candidate_v.empty()) || (! candidate_h.empty()))
						&& ((candidate_v != orig_v) || (candidate_h != orig_h)))
				{
					modified_v = std::move(candidate_v);
					modified_h = std::move(candidate_h);
					if (log) {
						std::cout << "for " << modified.str() << std::endl
							<< "v: " << join(modified_v) << std::endl
							<< "h: " << join(modified_h) << std::endl;
					}
					break;
				}
			}
			std::sort(modified_v.begin(), modified_v.end());
			std::sort(modified_h.begin(), modified_h.end());
			std::vector<size_t> modified_unique_v;
			std::vector<size_t> modified_unique_h;
			std::set_difference(modified_v.begin(), modified_v.end(),
					orig_v.begin(), orig_v.end(), std::back_inserter(modified_unique_v));
			std::set_difference(modified_h.begin(), modified_h.end(),
					orig_h.begin(), orig_h.end(), std::back_inserter(modified_unique_h));
			size_t vidx_sum = std::reduce(modified_unique_v.begin(), modified_unique_v.end());
			size_t hidx_sum = std::reduce(modified_unique_h.begin(), modified_unique_h.end());
			return vidx_sum + 100*hidx_sum;
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
	std::vector<MirroredTerrain> lava_island = parse_terrains(input);
	if (part == 2) {
		std::cout << part2_answer(lava_island) << std::endl;
	}
	else {
		std::cout << part1_answer(lava_island) << std::endl;
	}
	std::cout << "done" << std::endl;
	return 0;
}

// test input for multiple v-axes + h-axes
// #########
// #########
// .##.##.##
// ....##...
// ####..###
// ####..###
// ....##...
// .##.##.##

/*
--- Day 13: Point of Incidence ---
With your help, the hot springs team locates an appropriate spring which launches you neatly and precisely up to the edge of Lava Island.

There's just one problem: you don't see any lava.

You do see a lot of ash and igneous rock; there are even what look like gray mountains scattered around. After a while, you make your way to a nearby cluster of mountains only to discover that the valley between them is completely full of large mirrors. Most of the mirrors seem to be aligned in a consistent way; perhaps you should head in that direction?

As you move through the valley of mirrors, you find that several of them have fallen from the large metal frames keeping them in place. The mirrors are extremely flat and shiny, and many of the fallen mirrors have lodged into the ash at strange angles. Because the terrain is all one color, it's hard to tell where it's safe to walk or where you're about to run into a mirror.

You note down the patterns of ash (.) and rocks (#) that you see as you walk (your puzzle input); perhaps by carefully analyzing these patterns, you can figure out where the mirrors are!

For example:

#.##..##.
..#.##.#.
##......#
##......#
..#.##.#.
..##..##.
#.#.##.#.

#...##..#
#....#..#
..##..###
#####.##.
#####.##.
..##..###
#....#..#
To find the reflection in each pattern, you need to find a perfect reflection across either a horizontal line between two rows or across a vertical line between two columns.

In the first pattern, the reflection is across a vertical line between two columns; arrows on each of the two columns point at the line between the columns:

123456789
    ><   
#.##..##.
..#.##.#.
##......#
##......#
..#.##.#.
..##..##.
#.#.##.#.
    ><   
123456789
In this pattern, the line of reflection is the vertical line between columns 5 and 6. Because the vertical line is not perfectly in the middle of the pattern, part of the pattern (column 1) has nowhere to reflect onto and can be ignored; every other column has a reflected column within the pattern and must match exactly: column 2 matches column 9, column 3 matches 8, 4 matches 7, and 5 matches 6.

The second pattern reflects across a horizontal line instead:

1 #...##..# 1
2 #....#..# 2
3 ..##..### 3
4v#####.##.v4
5^#####.##.^5
6 ..##..### 6
7 #....#..# 7
This pattern reflects across the horizontal line between rows 4 and 5. Row 1 would reflect with a hypothetical row 8, but since that's not in the pattern, row 1 doesn't need to match anything. The remaining rows match: row 2 matches row 7, row 3 matches row 6, and row 4 matches row 5.

To summarize your pattern notes, add up the number of columns to the left of each vertical line of reflection; to that, also add 100 multiplied by the number of rows above each horizontal line of reflection. In the above example, the first pattern's vertical line has 5 columns to its left and the second pattern's horizontal line has 4 rows above it, a total of 405.

Find the line of reflection in each of the patterns in your notes. What number do you get after summarizing all of your notes?

Your puzzle answer was 32723.

--- Part Two ---
You resume walking through the valley of mirrors and - SMACK! - run directly into one. Hopefully nobody was watching, because that must have been pretty embarrassing.

Upon closer inspection, you discover that every mirror has exactly one smudge: exactly one . or # should be the opposite type.

In each pattern, you'll need to locate and fix the smudge that causes a different reflection line to be valid. (The old reflection line won't necessarily continue being valid after the smudge is fixed.)

Here's the above example again:

#.##..##.
..#.##.#.
##......#
##......#
..#.##.#.
..##..##.
#.#.##.#.

#...##..#
#....#..#
..##..###
#####.##.
#####.##.
..##..###
#....#..#
The first pattern's smudge is in the top-left corner. If the top-left # were instead ., it would have a different, horizontal line of reflection:

1 ..##..##. 1
2 ..#.##.#. 2
3v##......#v3
4^##......#^4
5 ..#.##.#. 5
6 ..##..##. 6
7 #.#.##.#. 7
With the smudge in the top-left corner repaired, a new horizontal line of reflection between rows 3 and 4 now exists. Row 7 has no corresponding reflected row and can be ignored, but every other row matches exactly: row 1 matches row 6, row 2 matches row 5, and row 3 matches row 4.

In the second pattern, the smudge can be fixed by changing the fifth symbol on row 2 from . to #:

1v#...##..#v1
2^#...##..#^2
3 ..##..### 3
4 #####.##. 4
5 #####.##. 5
6 ..##..### 6
7 #....#..# 7
Now, the pattern has a different horizontal line of reflection between rows 1 and 2.

Summarize your notes as before, but instead use the new different reflection lines. In this example, the first pattern's new horizontal line has 3 rows above it and the second pattern's new horizontal line has 1 row above it, summarizing to the value 400.

In each pattern, fix the smudge and find the different line of reflection. What number do you get after summarizing the new reflection line in each pattern in your notes?

Your puzzle answer was 34536.

Both parts of this puzzle are complete! They provide two gold stars: **
*/
