// c++ -Wall -Wextra -pedantic -O3 -std=c++20 day03.cpp -o day03 && time ./day03 day03in.txt
#include <algorithm>
#include <compare>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <numeric>
// #include <ranges> // my clang doesn't support :-(
#include <regex>
#include <sstream>
#include <string>
#include <vector>
#include <utility>

// beware: our origin is top-left and our y-axis is inverted
// from the usual Cartesian (our positive is down)
class Position {
	public:
	unsigned int x;
	unsigned int y;
	Position() : x(0), y(0) { }
	Position(unsigned int xx, unsigned int yy) : x(xx), y(yy) { }
	auto operator<=>(const Position&) const = default;
	std::string str() const;
};

class PartNumber {
	public:
	Position start;
	Position end;
	int number;
	PartNumber() : number(0) { }
	explicit PartNumber(int nn, Position ss={}, Position ee={})
			: start(ss), end(ee), number(nn) { }
	std::string str() const;
	std::vector<Position> adjacent_locations(const size_t maxrows, const size_t maxcols) const;
};

class Gear {
	public:
	Position location;
	PartNumber parts[2];
	Gear(const Position& pp, const PartNumber& p1, const PartNumber& p2)
		: location(pp), parts{p1, p2} { }
	std::string str() const;
};

class EngineSchematic {
	public:
	EngineSchematic() : cols_(0) { raw_.reserve(100*1024); }
	void append_schematic(const std::string&);
	void set_cols(size_t cc) { cols_ = cc; }
	size_t get_cols() const { return cols_; }
	size_t get_rows() const;
	char get_position(unsigned int xx, unsigned int yy) const { return raw_[get_cols()*yy + xx]; }
	char get_position(const Position& pos) const { return get_position(pos.x, pos.y); }
	bool is_symbol(unsigned int xx, unsigned int yy) const;
	bool is_symbol(const Position& pos) const { return is_symbol(pos.x, pos.y); }
	bool is_splat(unsigned int xx, unsigned int yy) const { return get_position(xx,yy) == '*'; }
	bool is_splat(const Position& pos) const { return is_splat(pos.x, pos.y); }
	std::vector<PartNumber> get_part_numbers() const;
	std::vector<Gear> get_gears() const;
	protected:
	std::string raw_;
	size_t cols_;

	std::vector<PartNumber> candidate_parts() const;
};

void dump(const std::vector<std::string>& input) {
	for (size_t aa=0; aa<input.size(); aa++) {
		std::cout << aa << ": >>" << input[aa] << "<<" << std::endl;
	}
}
template <typename TT>
void dump(const std::vector<TT>& input) {
	for (size_t aa=0; aa<input.size(); aa++) {
		std::cout << aa << ": " << input[aa].str() << std::endl;
	}
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

std::string PartNumber::str() const {
	std::stringstream out;
	out << "PN{" << number << ", " << start << " -> " << end << "}";
	return out.str();
}
std::ostream& operator<<(std::ostream& out, const PartNumber& pp) {
	out << pp.str();
	return out;
}

std::vector<Position> PartNumber::adjacent_locations(
		const size_t maxrows,
		const size_t maxcols) const
{
	std::vector<Position> result;
	if (((start.y == end.y) && (start.x > end.x)) || (start.y > end.y)
			|| (end.x == 0) || (start.x == maxcols-1))
	{
		std::cerr << "Error: end " << end << " before start " << start
			<< " or invalid start/end" << std::endl;
		return result;
	}
	Position trace(start);
	if (end.y > start.y) {
		if (start.x > 0) {
			for (trace.x=0; trace.x<start.x; trace.x++) {
				result.push_back(trace);
			}
		}
		if (start.y > 0) {
			trace = start;
			trace.y--;
			if (trace.x > 0) {
				trace.x--;
			}
			for (/*no-op*/; trace.x<=maxcols-1; trace.x++) {
				result.push_back(trace);
			}
		}
		trace = end;
		if (trace.y < maxrows-1) {
			trace.y++;
			unsigned int endx = end.x;
			if (endx < maxcols-1) {
				endx++;
			}
			for (trace.x=0; trace.x<=endx; trace.x++) {
				result.push_back(trace);
			}
		}
		if (end.x < maxcols-1) {
			trace = end;
			for (trace.x++; trace.x<maxcols-1; trace.x++) {
				result.push_back(trace);
			}
		}
	}
	else {
		if (trace.x > 0) {
			trace.x--;
			result.push_back(trace); // W of start
			if (trace.y > 0) {
				trace.y--;
				result.push_back(trace); // NW corner
			}
		}
		trace = start;
		if (trace.y > 0) {
			trace.y--;
			for (/*no-op*/; trace.x<=end.x; trace.x++) {
				result.push_back(trace); // N edge
			}
			if (trace.x < maxcols-1) {
				result.push_back(trace); // NE corner
			}
		}
		trace = end;
		if (trace.x < maxcols-1) {
			trace.x++;
			result.push_back(trace); // E of end
			if (trace.y < maxrows-1) {
				trace.y++;
				result.push_back(trace); // SE corner
			}
		}
		trace = end;
		if (trace.y < maxrows-1) {
			trace.y++;
			unsigned int startx = start.x;
			if (startx > 0) {
				startx--; // SW corner
			}
			for (trace.x=startx; trace.x<=end.x; trace.x++) {
				result.push_back(trace); // S edge
			}
		}
	}
	return result;
}

std::string Gear::str() const {
	std::stringstream out;
	out << "Gear{" << location << ", " << parts[0] << ", " << parts[1] << "}";
	return out.str();
}
std::ostream& operator<<(std::ostream& out, const Gear& gg) {
	out << gg.str();
	return out;
}

void EngineSchematic::append_schematic(const std::string& line) {
	if (line.size() == cols_) {
		raw_ += line;
	}
	else {
		std::cerr << "Warning: expected " << cols_  << "-long lines, but this line is "
			<< line.size() << ":\n  " << line << std::endl;
		std::string line2(line);
		line2.resize(cols_, '.');
		raw_ += line2;
	}
}

size_t EngineSchematic::get_rows() const {
	if (cols_ == 0U) {
		return 0U;
	}
	size_t rows = raw_.size() / cols_;
	if (raw_.size() % cols_ != 0) {
		rows++;
	}
	return rows;
}

bool EngineSchematic::is_symbol(unsigned int xx, unsigned int yy) const {
	const char schematic_c = get_position(xx, yy);
	return ! ((schematic_c == '.') || std::isdigit(static_cast<unsigned char>(schematic_c)));
}

std::vector<PartNumber> EngineSchematic::get_part_numbers() const {
	std::vector<PartNumber> parts = candidate_parts();
	std::erase_if(parts, [this](const PartNumber& pp) -> bool {
			bool has_adj_symbol = false;
			for (const Position& adjacency : pp.adjacent_locations(get_rows(), get_cols())) {
				if (is_symbol(adjacency)) {
					has_adj_symbol = true;
					break;
				}
			}
			return ! has_adj_symbol;
		});
	return parts;
}

std::vector<Gear> EngineSchematic::get_gears() const {
	std::vector<Gear> gears;
	struct PNTriple {
		const PartNumber* pn1;
		const PartNumber* pn2;
		const PartNumber* pn3;
		PNTriple() : pn1(nullptr), pn2(nullptr), pn3(nullptr) { }
	};
	std::map<Position, PNTriple> candidate_gears;
	std::vector<PartNumber> parts = candidate_parts();
	for (const auto& pn : parts) {
		for (const Position& adjacency : pn.adjacent_locations(get_rows(), get_cols())) {
			if (is_splat(adjacency)) {
				if (candidate_gears[adjacency].pn3) {
					; // ignore this PN
				}
				else if (candidate_gears[adjacency].pn2) {
					candidate_gears[adjacency].pn3 = &pn;
				}
				else if (candidate_gears[adjacency].pn1) {
					candidate_gears[adjacency].pn2 = &pn;
				}
				else {
					candidate_gears[adjacency].pn1 = &pn;
				}
			}
		}
	}
	for (const auto& [gearpos, pnt] : candidate_gears) {
		if (pnt.pn2 && !pnt.pn3) {
			gears.emplace_back(gearpos, *pnt.pn1, *pnt.pn2);
		}
	}
	return gears;
}

std::vector<PartNumber> EngineSchematic::candidate_parts() const {
	std::vector<PartNumber> parts;
	Position here;
	std::string token;
	PartNumber current_part;
	auto start_part = [&current_part, &here]() {
		current_part.start = here;
	};
	auto end_part = [&token, &current_part, &here, &parts]() {
		current_part.end = here;
		current_part.number = std::stoi(token);
		parts.push_back(std::move(current_part));
		current_part = PartNumber();
		token.clear();
	};
	for (size_t cc=0; cc<raw_.size(); cc++) {
		if (std::isdigit(static_cast<unsigned char>(raw_[cc]))) {
			if (token.empty()) {
				here.x = cc % cols_;
				here.y = cc / cols_;
				start_part();
			}
			token += raw_[cc];
		}
		else if (! token.empty()) {
			end_part();
		}
		here.x = cc % cols_;
		here.y = cc / cols_;
	}
	if (! token.empty()) {
		end_part();
	}
	return parts;
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

EngineSchematic build_schematic(const std::vector<std::string>& input) {
	EngineSchematic result;
	for (const std::string& line : input) {
		if (line.empty()) { continue; }
		if (result.get_cols() == 0) {
			result.set_cols(line.size());
		}
		result.append_schematic(line);
	}
	return result;
}

int part1_answer(const EngineSchematic& schematic) {
	std::vector<PartNumber> parts = schematic.get_part_numbers();
	// dump(parts);
	return std::transform_reduce(parts.begin(), parts.end(), 0, std::plus<>(),
			[](const PartNumber& pn) -> int { return pn.number; });
}

int part2_answer(const EngineSchematic& schematic) {
	std::vector<Gear> gears = schematic.get_gears();
	// dump(gears);
	return std::transform_reduce(gears.begin(), gears.end(), 0, std::plus<>(),
			[](const Gear& gg) -> int { return gg.parts[0].number * gg.parts[1].number; });
}

int main(int argc, char* argv[]) {
	std::vector<std::string> input;
	if (argc > 1) {
		input = get_input(argv[1]);
	}
	else {
		input = get_input();
	}
	EngineSchematic schematic = build_schematic(input);
	// std::cout << part1_answer(schematic) << std::endl;
	std::cout << part2_answer(schematic) << std::endl;
	return 0;
}



/*
--- Day 3: Gear Ratios ---
You and the Elf eventually reach a gondola lift station; he says the gondola lift will take you up to the water source, but this is as far as he can bring you. You go inside.

It doesn't take long to find the gondolas, but there seems to be a problem: they're not moving.

"Aaah!"

You turn around to see a slightly-greasy Elf with a wrench and a look of surprise. "Sorry, I wasn't expecting anyone! The gondola lift isn't working right now; it'll still be a while before I can fix it." You offer to help.

The engineer explains that an engine part seems to be missing from the engine, but nobody can figure out which one. If you can add up all the part numbers in the engine schematic, it should be easy to work out which part is missing.

The engine schematic (your puzzle input) consists of a visual representation of the engine. There are lots of numbers and symbols you don't really understand, but apparently any number adjacent to a symbol, even diagonally, is a "part number" and should be included in your sum. (Periods (.) do not count as a symbol.)

Here is an example engine schematic:

467..114..
...*......
..35..633.
......#...
617*......
.....+.58.
..592.....
......755.
...$.*....
.664.598..
In this schematic, two numbers are not part numbers because they are not adjacent to a symbol: 114 (top right) and 58 (middle right). Every other number is adjacent to a symbol and so is a part number; their sum is 4361.

Of course, the actual engine schematic is much larger. What is the sum of all of the part numbers in the engine schematic?

Your puzzle answer was 525119.

--- Part Two ---
The engineer finds the missing part and installs it in the engine! As the engine springs to life, you jump in the closest gondola, finally ready to ascend to the water source.

You don't seem to be going very fast, though. Maybe something is still wrong? Fortunately, the gondola has a phone labeled "help", so you pick it up and the engineer answers.

Before you can explain the situation, she suggests that you look out the window. There stands the engineer, holding a phone in one hand and waving with the other. You're going so slowly that you haven't even left the station. You exit the gondola.

The missing part wasn't the only issue - one of the gears in the engine is wrong. A gear is any * symbol that is adjacent to exactly two part numbers. Its gear ratio is the result of multiplying those two numbers together.

This time, you need to find the gear ratio of every gear and add them all up so that the engineer can figure out which gear needs to be replaced.

Consider the same engine schematic again:

467..114..
...*......
..35..633.
......#...
617*......
.....+.58.
..592.....
......755.
...$.*....
.664.598..
In this schematic, there are two gears. The first is in the top left; it has part numbers 467 and 35, so its gear ratio is 16345. The second gear is in the lower right; its gear ratio is 451490. (The * adjacent to 617 is not a gear because it is only adjacent to one part number.) Adding up all of the gear ratios produces 467835.

What is the sum of all of the gear ratios in your engine schematic?

Your puzzle answer was 76504829.

Both parts of this puzzle are complete! They provide two gold stars: **
*/

