// c++ -Wall -Wextra -pedantic -O3 -std=c++20 day02.cpp -o day02 && time ./day02 day02in.txt
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

void dump(const std::vector<std::string>& input) {
	for (size_t aa=0; aa<input.size(); aa++) {
		std::cout << aa << ": >>" << input[aa] << "<<" << std::endl;
	}
}

class RGBCount {
	public:
	unsigned int red;
	unsigned int green;
	unsigned int blue;
	RGBCount() : red(0), green(0), blue(0) { }
	RGBCount(unsigned int rr, unsigned int gg, unsigned int bb) : red(rr), green(gg), blue(bb) { }
	RGBCount(const RGBCount&) = default;
	auto operator<=>(const RGBCount&) const = default;
};
std::ostream& operator<<(std::ostream& out, const RGBCount& val) {
	out << "{R:" << val.red << " G:" << val.green << " B:" << val.blue << "}";
	return out;
}

class DiceGame {
	public:
	int id_;
	std::vector<RGBCount> draws_;

	explicit DiceGame(int id) : id_(id) { }
	DiceGame(const DiceGame&) = default;

	std::string str() const;
};
std::ostream& operator<<(std::ostream& out, const DiceGame& val) {
	out << val.str();
	return out;
}

std::string DiceGame::str() const {
	std::stringstream out;
	out << "DiceGame " << id_ << ":";
	for (const RGBCount& dd : draws_) {
		out << " " << dd;
	}
	return out.str();
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

std::vector<DiceGame> ParseGames(const std::vector<std::string>& input) {
	std::vector<DiceGame> result;
	for (const std::string& line : input) {
		// What I learned: C++ regex sucks
		// There is no equivalent to a /g flag, so a nested subgroup that
		// repeats can only be captured in multiple steps by doing a match
		// capturing the outermost group, then a new match on that to capture
		// subgroups within. This despite that the engine had to identify
		// those groups already in order to get the final result. Ugh.
		DiceGame game(0);
		std::smatch matches1;
		std::string::const_iterator re_start = line.begin();
		if (std::regex_search(re_start, line.end(), matches1,
				std::regex(R"(^\s*game\s+(\d+):\s*)", std::regex::icase)))
		{
			game.id_ = std::stoi(matches1[1].str());
			std::smatch matches2;
			for (re_start = matches1.suffix().first;
					std::regex_search(re_start, line.end(), matches2,
						std::regex(R"(^;?\s*([^;]+))"));
					re_start = matches2.suffix().first)
			{
				RGBCount draw;
				std::smatch matches3;
				for (re_start = matches2[1].first;
						std::regex_search(re_start, matches2[1].second, matches3,
							std::regex(R"(^,?\s*(\d+)\s+(\w+)\s*)"));
						re_start = matches3.suffix().first)
				{
					if (matches3[2].str() == "red") {
						draw.red = std::stoi(matches3[1]);
					}
					else if (matches3[2].str() == "green") {
						draw.green = std::stoi(matches3[1]);
					}
					else if (matches3[2].str() == "blue") {
						draw.blue = std::stoi(matches3[1]);
					}
					else {
						std::cerr << "Skipping unknown dice color: " << matches3.str() << std::endl;
					}
				}
				if ((draw.red > 0) || (draw.green > 0) || (draw.blue > 0)) {
					game.draws_.push_back(std::move(draw));
				}
			}
			// std::cout << "Parse complete for " << game << std::endl;
			result.push_back(std::move(game));
		}
	}
	return result;
}



template <typename TT>
std::vector<TT> filter_and_transform(const std::vector<DiceGame>& input,
		bool (*filter_pred)(const DiceGame&), TT (*xform_pred)(const DiceGame&))
{
	std::vector<DiceGame> matches;
	std::copy_if(input.begin(), input.end(), std::back_inserter(matches), filter_pred);
	std::vector<TT> output;
	std::transform(matches.begin(), matches.end(), std::back_inserter(output), xform_pred);
	return output;
}

int sum_ids_12r13g14b(const std::vector<DiceGame>& input) {
	std::vector<int> matching_ids = filter_and_transform<int>(input,
		[](const DiceGame& gg) -> bool {
			auto max_pred = [](const RGBCount& lhs, const RGBCount& rhs) {
				return RGBCount(std::max(lhs.red, rhs.red),
					std::max(lhs.green, rhs.green),
					std::max(lhs.blue, rhs.blue));
			};
			RGBCount max_counts
				= std::reduce(gg.draws_.cbegin(), gg.draws_.cend(), RGBCount(), max_pred);
			return (max_counts.red <= 12) && (max_counts.green <= 13) && (max_counts.blue <= 14);
		},
		[](const DiceGame& gg) -> int { return gg.id_; });
	return std::reduce(matching_ids.cbegin(), matching_ids.cend());
}

int sum_min_power(const std::vector<DiceGame>& input) {
	std::vector<int> min_power;
	std::transform(input.cbegin(), input.cend(), std::back_inserter(min_power),
		[](const DiceGame& gg) -> int {
			auto max_pred = [](const RGBCount& lhs, const RGBCount& rhs) {
				return RGBCount(std::max(lhs.red, rhs.red),
					std::max(lhs.green, rhs.green),
					std::max(lhs.blue, rhs.blue));
			};
			RGBCount max_counts
				= std::reduce(gg.draws_.cbegin(), gg.draws_.cend(), RGBCount(), max_pred);
			return max_counts.red * max_counts.green * max_counts.blue;
		});
	return std::reduce(min_power.cbegin(), min_power.cend());
}

int main(int argc, char* argv[]) {
	std::vector<std::string> input;
	if (argc > 1) {
		input = get_input(argv[1]);
	}
	else {
		input = get_input();
	}
	std::vector<DiceGame> games_played = ParseGames(input);
	// part 1
	// std::cout << sum_ids_12r13g14b(games_played) << std::endl;
	// part 2
	std::cout << sum_min_power(games_played) << std::endl;
	return 0;
}




/*
--- Day 2: Cube Conundrum ---
You're launched high into the atmosphere! The apex of your trajectory just barely reaches the surface of a large island floating in the sky. You gently land in a fluffy pile of leaves. It's quite cold, but you don't see much snow. An Elf runs over to greet you.

The Elf explains that you've arrived at Snow Island and apologizes for the lack of snow. He'll be happy to explain the situation, but it's a bit of a walk, so you have some time. They don't get many visitors up here; would you like to play a game in the meantime?

As you walk, the Elf shows you a small bag and some cubes which are either red, green, or blue. Each time you play this game, he will hide a secret number of cubes of each color in the bag, and your goal is to figure out information about the number of cubes.

To get information, once a bag has been loaded with cubes, the Elf will reach into the bag, grab a handful of random cubes, show them to you, and then put them back in the bag. He'll do this a few times per game.

You play several games and record the information from each game (your puzzle input). Each game is listed with its ID number (like the 11 in Game 11: ...) followed by a semicolon-separated list of subsets of cubes that were revealed from the bag (like 3 red, 5 green, 4 blue).

For example, the record of a few games might look like this:

Game 1: 3 blue, 4 red; 1 red, 2 green, 6 blue; 2 green
Game 2: 1 blue, 2 green; 3 green, 4 blue, 1 red; 1 green, 1 blue
Game 3: 8 green, 6 blue, 20 red; 5 blue, 4 red, 13 green; 5 green, 1 red
Game 4: 1 green, 3 red, 6 blue; 3 green, 6 red; 3 green, 15 blue, 14 red
Game 5: 6 red, 1 blue, 3 green; 2 blue, 1 red, 2 green
In game 1, three sets of cubes are revealed from the bag (and then put back again). The first set is 3 blue cubes and 4 red cubes; the second set is 1 red cube, 2 green cubes, and 6 blue cubes; the third set is only 2 green cubes.

The Elf would first like to know which games would have been possible if the bag contained only 12 red cubes, 13 green cubes, and 14 blue cubes?

In the example above, games 1, 2, and 5 would have been possible if the bag had been loaded with that configuration. However, game 3 would have been impossible because at one point the Elf showed you 20 red cubes at once; similarly, game 4 would also have been impossible because the Elf showed you 15 blue cubes at once. If you add up the IDs of the games that would have been possible, you get 8.

Determine which games would have been possible if the bag had been loaded with only 12 red cubes, 13 green cubes, and 14 blue cubes. What is the sum of the IDs of those games?

Your puzzle answer was 2541.

--- Part Two ---
The Elf says they've stopped producing snow because they aren't getting any water! He isn't sure why the water stopped; however, he can show you how to get to the water source to check it out for yourself. It's just up ahead!

As you continue your walk, the Elf poses a second question: in each game you played, what is the fewest number of cubes of each color that could have been in the bag to make the game possible?

Again consider the example games from earlier:

Game 1: 3 blue, 4 red; 1 red, 2 green, 6 blue; 2 green
Game 2: 1 blue, 2 green; 3 green, 4 blue, 1 red; 1 green, 1 blue
Game 3: 8 green, 6 blue, 20 red; 5 blue, 4 red, 13 green; 5 green, 1 red
Game 4: 1 green, 3 red, 6 blue; 3 green, 6 red; 3 green, 15 blue, 14 red
Game 5: 6 red, 1 blue, 3 green; 2 blue, 1 red, 2 green
In game 1, the game could have been played with as few as 4 red, 2 green, and 6 blue cubes. If any color had even one fewer cube, the game would have been impossible.
Game 2 could have been played with a minimum of 1 red, 3 green, and 4 blue cubes.
Game 3 must have been played with at least 20 red, 13 green, and 6 blue cubes.
Game 4 required at least 14 red, 3 green, and 15 blue cubes.
Game 5 needed no fewer than 6 red, 3 green, and 2 blue cubes in the bag.
The power of a set of cubes is equal to the numbers of red, green, and blue cubes multiplied together. The power of the minimum set of cubes in game 1 is 48. In games 2-5 it was 12, 1560, 630, and 36, respectively. Adding up these five powers produces the sum 2286.

For each game, find the minimum set of cubes that must have been present. What is the sum of the power of these sets?

Your puzzle answer was 66016.

Both parts of this puzzle are complete! They provide two gold stars: **
*/
