// c++ -Wall -Wextra -pedantic -O3 -std=c++20 day16.cpp -o day16 && time ./day16 day16in.txt 1
// vi:set noet ts=4 sw=4:
#include <algorithm>
#include <compare>
#include <concepts>
#include <fstream>
#include <functional>
#include <iostream>
#include <numeric>
#include <set>
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
std::ostream& operator<<(std::ostream& out, const Direction& dir) {
	switch (dir) {
		case Direction::NORTH: out << "N"; break;
		case Direction::SOUTH: out << "S"; break;
		case Direction::EAST:  out << "E"; break;
		case Direction::WEST:  out << "W"; break;
		default: out << "?"; break;
	}
	return out;
}

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
	bool move(const Direction& dir,
		uint64_t xmax=0, uint64_t ymax=0,
		uint64_t xmin=0, uint64_t ymin=0);
	Position north(uint64_t limit=0) const;
	Position south(uint64_t limit=0) const;
	Position east(uint64_t limit=0) const;
	Position west(uint64_t limit=0) const;
};

class Beam {
	public:
	Position where;
	Direction toward;

	auto operator<=>(const Beam&) const = default;
	std::string str() const;
};

class MirrorLayout {
	public:
	MirrorLayout() : cols_(0U) { raw_.reserve(512); }

	void clear() { cols_ = 0U; raw_.clear(); energized_.clear(); }
	bool empty() const { return raw_.empty(); }
	void append_map(const std::string&);
	void set_cols(size_t cc) { cols_ = cc; }
	size_t get_cols() const { return cols_; }
	size_t get_rows() const;
	std::string str() const;
	char get_pos(const size_t xx, const size_t yy) const;
	void set_pos(const size_t xx, const size_t yy, const char cc);
	size_t energized_count() const;
	bool energized(const size_t xx, const size_t yy) const;
	void set_energized(const size_t xx, const size_t yy, const bool energized);
	size_t add_beam(const Beam& start);

	protected:
	std::string raw_;
	std::string energized_;
	size_t cols_;

	char get_pos_impl(const std::string& map, size_t xx, size_t yy) const;
	void set_pos_impl(std::string& map, size_t xx, size_t yy, const char cc);
};

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

bool Position::move(const Direction& dir,
	uint64_t xmax, uint64_t ymax, uint64_t xmin, uint64_t ymin)
{
	Position before=*this;
	switch (dir) {
		case Direction::NORTH: *this = north(ymin); break;
		case Direction::SOUTH: *this = south(ymax); break;
		case Direction::EAST:  *this = east(xmax); break;
		case Direction::WEST:  *this = west(xmin); break;
		default:
			std::cerr << "Error: invalid move " << dir << std::endl;
	}
	return *this != before;
}

std::string Position::str() const {
	std::stringstream out;
	out << "(" << x << ", " << y << ")";
	return out.str();
}

Position Position::north(uint64_t limit) const {
	if (y > limit) {
		return Position(x, y-1);
	}
	return *this;
}

Position Position::south(uint64_t limit) const {
	if ((limit == 0) || (y < limit)) {
		return Position(x, y+1);
	}
	return *this;
}

Position Position::east(uint64_t limit) const {
	if ((limit == 0) || (x < limit)) {
		return Position(x+1, y);
	}
	return *this;
}

Position Position::west(uint64_t limit) const {
	if (x > limit) {
		return Position(x-1, y);
	}
	return *this;
}

void MirrorLayout::append_map(const std::string& line) {
	if (raw_.empty()) {
		raw_ += line;
		set_cols(line.size());
		energized_ += std::string(cols_, '.');
	}
	else if ((! raw_.empty()) && (line.size() != cols_)) {
		std::cerr << "Warning: expected " << cols_  << "-long lines, but this line is "
			<< line.size() << ":\n  " << line << std::endl;
		std::string line2(line);
		line2.resize(cols_, '.');
		raw_ += line2;
		energized_ += std::string(cols_, '.');
	}
	else {
		raw_ += line;
		energized_ += std::string(cols_, '.');
	}
}

size_t MirrorLayout::get_rows() const {
	if (cols_ == 0U) {
		return 0U;
	}
	size_t rows = raw_.size() / cols_;
	if (raw_.size() % cols_ != 0) {
		rows++;
	}
	return rows;
}

std::string MirrorLayout::str() const {
	std::stringstream out;
	out << "map[" << get_cols() << "x" << get_rows() << "]:";
	for (size_t rs=0; rs<raw_.size(); rs+=cols_) {
		out << "\n  " << raw_.substr(rs, cols_);
	}
	out << "\nenergized-state (" << energized_count() << "):";
	for (size_t rs=0; rs<energized_.size(); rs+=cols_) {
		out << "\n  " << energized_.substr(rs, cols_);
	}
	return out.str();
}

char MirrorLayout::get_pos(size_t xx, size_t yy) const {
	return get_pos_impl(raw_, xx, yy);
}

void MirrorLayout::set_pos(const size_t xx, const size_t yy, const char cc) {
	return set_pos_impl(raw_, xx, yy, cc);
}

size_t MirrorLayout::energized_count() const {
	size_t count = 0;
	for (const char cc : energized_) {
		if (cc == '#') {
			count++;
		}
	}
	return count;
}

bool MirrorLayout::energized(size_t xx, size_t yy) const {
	return get_pos_impl(energized_, xx, yy) == '#';
}

void MirrorLayout::set_energized(const size_t xx, const size_t yy, const bool ee) {
	return set_pos_impl(energized_, xx, yy, (ee ? '#' : '.'));
}

size_t MirrorLayout::add_beam(const Beam& start) {
	bool verbose = false;
	std::set<Beam> processed;
	std::vector<Beam> paths;
	paths.push_back(start);
	for (size_t bidx=0; bidx<paths.size(); bidx++) {
		if (verbose) {
			std::cout << "Begin path " << bidx << ", " << paths[bidx].str() << std::endl;
		}
		bool still_on_map = true;
		while (still_on_map) {
			if (! processed.insert(paths[bidx]).second) {
				if (verbose) {
					std::cout << "Aborting path " << bidx
						<< " after already-processed point " << paths[bidx].str() << std::endl;
				}
				break;
			}
			set_energized(paths[bidx].where.x, paths[bidx].where.y, true);
			switch (get_pos(paths[bidx].where.x, paths[bidx].where.y)) {
				case '.':
					; // just keep going
					break;
				case '/':
					switch (paths[bidx].toward) {
						case Direction::NORTH: paths[bidx].toward = Direction::EAST; break;
						case Direction::SOUTH: paths[bidx].toward = Direction::WEST; break;
						case Direction::EAST:  paths[bidx].toward = Direction::NORTH; break;
						case Direction::WEST:  paths[bidx].toward = Direction::SOUTH; break;
						default:
							std::cerr << "Error: invalid direction for / "
								<< paths[bidx].toward << std::endl;
					}
					break;
				case '\\':
					switch (paths[bidx].toward) {
						case Direction::NORTH: paths[bidx].toward = Direction::WEST; break;
						case Direction::SOUTH: paths[bidx].toward = Direction::EAST; break;
						case Direction::EAST:  paths[bidx].toward = Direction::SOUTH; break;
						case Direction::WEST:  paths[bidx].toward = Direction::NORTH; break;
						default:
							std::cerr << "Error: invalid direction for \\ "
								<< paths[bidx].toward << std::endl;
					}
					break;
				case '|':
					switch (paths[bidx].toward) {
						case Direction::NORTH:
						case Direction::SOUTH:
							; // just keep going
							break;
						case Direction::EAST:
						case Direction::WEST: {
							paths[bidx].toward = Direction::NORTH;
							Beam split{paths[bidx].where, Direction::SOUTH};
							if (split.where.move(Direction::SOUTH, get_cols()-1, get_rows()-1)) {
								paths.push_back(split);
							}
							break;
						}
						default:
							std::cerr << "Error: invalid direction for \\ "
								<< paths[bidx].toward << std::endl;
					}
					break;
				case '-':
					switch (paths[bidx].toward) {
						case Direction::NORTH:
						case Direction::SOUTH: {
							paths[bidx].toward = Direction::EAST;
							Beam split{paths[bidx].where, Direction::WEST};
							if (split.where.move(Direction::WEST, get_cols()-1, get_rows()-1)) {
								paths.push_back(split);
							}
							break;
						}
						case Direction::EAST:
						case Direction::WEST:
							; // just keep going
							break;
						default:
							std::cerr << "Error: invalid direction for \\ "
								<< paths[bidx].toward << std::endl;
					}
					break;
				default:
					std::cerr << "Error: invalid map notation " << 
						get_pos(paths[bidx].where.x, paths[bidx].where.y) << std::endl;
			}
			still_on_map = paths[bidx].where.move(paths[bidx].toward, get_cols()-1, get_rows()-1);
		}
	}
	return energized_count();
}

char MirrorLayout::get_pos_impl(const std::string& map, const size_t xx, const size_t yy) const {
	if ((xx < cols_) && (yy < get_rows())) {
		return map[yy*cols_ + xx];
	}
	else {
		std::cerr << "Error: invalid get-map position, " << xx << ", " << yy << std::endl;
		return '.';
	}
}

void MirrorLayout::set_pos_impl(std::string& map, const size_t xx, const size_t yy, const char cc) {
	if ((xx < cols_) && (yy < get_rows()))
	{
		map[yy*cols_ + xx] = cc;
	}
	else {
		std::cerr << "Error: invalid set-map position, " << xx << ", " << yy << std::endl;
	}
}

std::string Beam::str() const {
	std::stringstream out;
	out << "Beam{" << where.str() << ", " << toward << "}";
	return out.str();
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

MirrorLayout document_mirrors(const std::vector<std::string>& input) {
	MirrorLayout map;
	for (const std::string& line : input) {
		if (line.empty()) { continue; }
		map.append_map(line);
	}
	return map;
}

int part1_answer(MirrorLayout map) {
	map.add_beam(Beam{Position(), Direction::EAST});
	// std::cout << map.str() << std::endl;
	return map.energized_count();
}

int part2_answer(const MirrorLayout& initial) {
	const size_t rsize = initial.get_rows();
	size_t max_energized = 0;
	for (unsigned int xx=0; xx<initial.get_cols(); xx++) {
		MirrorLayout trialS(initial);
		trialS.add_beam(Beam{Position(xx, 0), Direction::SOUTH});
		max_energized = std::max(max_energized, trialS.energized_count());
		MirrorLayout trialN(initial);
		trialN.add_beam(Beam{Position(xx, trialN.get_rows()-1), Direction::NORTH});
		max_energized = std::max(max_energized, trialN.energized_count());
	}
	for (unsigned int yy=0; yy<rsize; yy++) {
		MirrorLayout trialE(initial);
		trialE.add_beam(Beam{Position(0, yy), Direction::EAST});
		max_energized = std::max(max_energized, trialE.energized_count());
		MirrorLayout trialW(initial);
		trialW.add_beam(Beam{Position(trialW.get_cols()-1, yy), Direction::WEST});
		max_energized = std::max(max_energized, trialW.energized_count());
	}
	return max_energized;
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
	MirrorLayout lava_room = document_mirrors(input);
	if (part == 2) {
		std::cout << part2_answer(lava_room) << std::endl;
	}
	else {
		std::cout << part1_answer(lava_room) << std::endl;
	}
	return 0;
}

/*
--- Day 16: The Floor Will Be Lava ---
With the beam of light completely focused somewhere, the reindeer leads you deeper still into the Lava Production Facility. At some point, you realize that the steel facility walls have been replaced with cave, and the doorways are just cave, and the floor is cave, and you're pretty sure this is actually just a giant cave.

Finally, as you approach what must be the heart of the mountain, you see a bright light in a cavern up ahead. There, you discover that the beam of light you so carefully focused is emerging from the cavern wall closest to the facility and pouring all of its energy into a contraption on the opposite side.

Upon closer inspection, the contraption appears to be a flat, two-dimensional square grid containing empty space (.), mirrors (/ and \), and splitters (| and -).

The contraption is aligned so that most of the beam bounces around the grid, but each tile on the grid converts some of the beam's light into heat to melt the rock in the cavern.

You note the layout of the contraption (your puzzle input). For example:

.|...\....
|.-.\.....
.....|-...
........|.
..........
.........\
..../.\\..
.-.-/..|..
.|....-|.\
..//.|....
The beam enters in the top-left corner from the left and heading to the right. Then, its behavior depends on what it encounters as it moves:

If the beam encounters empty space (.), it continues in the same direction.
If the beam encounters a mirror (/ or \), the beam is reflected 90 degrees depending on the angle of the mirror. For instance, a rightward-moving beam that encounters a / mirror would continue upward in the mirror's column, while a rightward-moving beam that encounters a \ mirror would continue downward from the mirror's column.
If the beam encounters the pointy end of a splitter (| or -), the beam passes through the splitter as if the splitter were empty space. For instance, a rightward-moving beam that encounters a - splitter would continue in the same direction.
If the beam encounters the flat side of a splitter (| or -), the beam is split into two beams going in each of the two directions the splitter's pointy ends are pointing. For instance, a rightward-moving beam that encounters a | splitter would split into two beams: one that continues upward from the splitter's column and one that continues downward from the splitter's column.
Beams do not interact with other beams; a tile can have many beams passing through it at the same time. A tile is energized if that tile has at least one beam pass through it, reflect in it, or split in it.

In the above example, here is how the beam of light bounces around the contraption:

>|<<<\....
|v-.\^....
.v...|->>>
.v...v^.|.
.v...v^...
.v...v^..\
.v../2\\..
<->-/vv|..
.|<<<2-|.\
.v//.|.v..
Beams are only shown on empty tiles; arrows indicate the direction of the beams. If a tile contains beams moving in multiple directions, the number of distinct directions is shown instead. Here is the same diagram but instead only showing whether a tile is energized (#) or not (.):

######....
.#...#....
.#...#####
.#...##...
.#...##...
.#...##...
.#..####..
########..
.#######..
.#...#.#..
Ultimately, in this example, 46 tiles become energized.

The light isn't energizing enough tiles to produce lava; to debug the contraption, you need to start by analyzing the current situation. With the beam starting in the top-left heading right, how many tiles end up being energized?

Your puzzle answer was 6514.

--- Part Two ---
As you try to work out what might be wrong, the reindeer tugs on your shirt and leads you to a nearby control panel. There, a collection of buttons lets you align the contraption so that the beam enters from any edge tile and heading away from that edge. (You can choose either of two directions for the beam if it starts on a corner; for instance, if the beam starts in the bottom-right corner, it can start heading either left or upward.)

So, the beam could start on any tile in the top row (heading downward), any tile in the bottom row (heading upward), any tile in the leftmost column (heading right), or any tile in the rightmost column (heading left). To produce lava, you need to find the configuration that energizes as many tiles as possible.

In the above example, this can be achieved by starting the beam in the fourth tile from the left in the top row:

.|<2<\....
|v-v\^....
.v.v.|->>>
.v.v.v^.|.
.v.v.v^...
.v.v.v^..\
.v.v/2\\..
<-2-/vv|..
.|<<<2-|.\
.v//.|.v..
Using this configuration, 51 tiles are energized:

.#####....
.#.#.#....
.#.#.#####
.#.#.##...
.#.#.##...
.#.#.##...
.#.#####..
########..
.#######..
.#...#.#..
Find the initial beam configuration that energizes the largest number of tiles; how many tiles are energized in that configuration?

Your puzzle answer was 8089.

Both parts of this puzzle are complete! They provide two gold stars: **
*/

