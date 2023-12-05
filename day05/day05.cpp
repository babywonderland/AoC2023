// c++ -Wall -Wextra -pedantic -O3 -std=c++20 day05.cpp -o day05 && time ./day05 day05in.txt 2
// vi:set noet ts=4 sw=4:
#include <algorithm>
#include <compare>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <numeric>
#include <optional>
#include <regex>
#include <sstream>
#include <string>
#include <vector>
#include <utility>

// In retrospect, the intermediate steps aren't needed, so a better (and much,
// much faster) approach would be to compose a cummulative map, then search
// in reverse from smallest location to largest location, looking for the first
// hit among seeds. Ranges would work well for composing the cummulative map.

class ClosedInterval {
	public:
	int64_t lower;
	int64_t upper;

	ClosedInterval() : lower(0), upper(0) { }
	ClosedInterval(int64_t ll, int64_t uu) : lower(ll), upper(uu) { normalize(); }
	// Clang thinks these aren't user-declared, so it deletes the copy-constructor
	// of things that contain them if uncommented
	// ClosedInterval(const ClosedInterval&) = default;
	// ClosedInterval(ClosedInterval&&) = default;
	auto operator<=>(const ClosedInterval&) const = default;

	std::string str() const {
		std::stringstream out;
		out << "ClosedInterval(" << lower << ", " << upper << ")";
		return out.str();
	}
	void normalize() { if (lower > upper) { std::swap(lower, upper); } }
	std::vector<int64_t> expand() const;
	bool contains(const int64_t val) const { return (val >= lower) && (val <= upper); }
};

class BadClangNoBiscuit {
	public:
	ClosedInterval src;
	int64_t dst;

	auto operator<=>(const BadClangNoBiscuit&) const = default;
	std::string str() const { return src.str() + " -> " + std::to_string(dst); }
};
bool operator<(const BadClangNoBiscuit& lhs, const int64_t rhs) {
	return lhs.src.lower < rhs;
}
bool operator<(const int64_t lhs, const BadClangNoBiscuit& rhs) {
	return lhs < rhs.src.lower;
}

// In a "real" situation, seed, soil, fertilizer, water, light, temp, humidity, and location
// would all want to be distinct types derived from int
class Almanac {
	public:
	typedef std::vector<BadClangNoBiscuit> amap_t;
	std::vector<int64_t> seeds;
	amap_t seed2soil;
	amap_t soil2fertilizer;
	amap_t fertilizer2water;
	amap_t water2light;
	amap_t light2temp;
	amap_t temp2humidity;
	amap_t humidity2location;
	bool seeds_are_ranged;

	Almanac() : seeds_are_ranged(false) { }
	// part 1: seeds are numbers; part 2: seeds are ranges
	Almanac(const std::vector<std::string>&, const int part);
	Almanac(const Almanac&) = default;

	std::string str() const;
	int64_t get_soil(const int64_t seed) const { return amap_lookup(seed2soil, seed); }
	int64_t get_fertilizer(const int64_t soil) const { return amap_lookup(soil2fertilizer, soil); }
	int64_t get_water(const int64_t fertilizer) const {
	   	return amap_lookup(fertilizer2water, fertilizer);
	}
	int64_t get_light(const int64_t water) const { return amap_lookup(water2light, water); }
	int64_t get_temp(const int64_t light) const { return amap_lookup(light2temp, light); }
	int64_t get_humidity(const int64_t temp) const { return amap_lookup(temp2humidity, temp); }
	int64_t get_location(const int64_t humidity) const {
		return amap_lookup(humidity2location, humidity);
	}
	int64_t get_location_for_seed(const int64_t seed) const {
		/*
		int64_t steps[8] = { };
		std::cout << "g4ls seed  " << std::endl; steps[0] = seed;
		std::cout << "g4ls soil  " << std::endl; steps[1] = get_soil(steps[0]);
		std::cout << "g4ls fert  " << std::endl; steps[2] = get_fertilizer(steps[1]);
		std::cout << "g4ls water " << std::endl; steps[3] = get_water(steps[2]);
		std::cout << "g4ls light " << std::endl; steps[4] = get_light(steps[3]);
		std::cout << "g4ls temp  " << std::endl; steps[5] = get_temp(steps[4]);
		std::cout << "g4ls hum   " << std::endl; steps[6] = get_humidity(steps[5]);
		std::cout << "g4ls loc   " << std::endl; steps[7] = get_location(steps[6]);
		std::cout << "g4ls res   " << steps[7] << std::endl;
		*/
		return get_location(get_humidity(get_temp(get_light(get_water(
					get_fertilizer(get_soil(seed)))))));
	}

	protected:
	static std::optional<amap_t::value_type> parse_amap(const std::string&);
	static int64_t amap_lookup(const amap_t&, const int64_t);
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

std::vector<int64_t> ClosedInterval::expand() const {
	std::vector<int64_t> result;
	if (lower == upper) {
		result = std::vector<int64_t>(1, lower);
	}
	else if (lower < upper) {
		result = std::vector<int64_t>(upper-lower+1);
		std::iota(result.begin(), result.end(), lower);
	}
	else {
		for (int64_t ii=lower; ii>=upper; ii--) {
			result.push_back(ii);
		}
	}
	return result;
}

Almanac::Almanac(const std::vector<std::string>& input, const int part) {
	int64_t load_state = 0;
	for (size_t aa=0; aa<input.size(); ++aa) {
		const std::string& line = input[aa];
		if (line.empty()) { continue; }
		switch (load_state) {
			case 0: { // seeds
				if (! str_tolower(line).starts_with("seeds:")) {
					std::cerr << "Error: expected 'seeds:' on line " << aa << std::endl;
					return;
				}
				for (const std::string& num : split(line.substr(6))) {
					if (num.empty()) { continue; }
					try {
						seeds.push_back(std::stoll(num));
					}
					catch(...) {
						std::cerr << "Error: expected number instead of "
							<< num << " on line " << aa << std::endl;
						return;
					}
				}
				if (part == 2) {
					if ((seeds.size() % 2) != 0) {
						std::cerr << "Error: even number of seeds required, got "
							<< seeds.size() << " on line " << aa << std::endl;
						return;
					}
					seeds_are_ranged = true;
				}
				load_state = 1;
				break;
			}
			case 1: { // seed2soil
				if (! str_tolower(line).starts_with("seed-to-soil map:")) {
					std::cerr << "Error: expected 'seed-to-soil map:' on line " << aa << std::endl;
					return;
				}
				load_state = 2;
				break;
			}
			case 2: { // seed2soil
				auto vals = parse_amap(line);
				if (vals) {
					seed2soil.push_back(vals.value());
					break;
				}
				else {
					load_state = 3;
					// and fall through
				}
			}
			case 3: { // soil2fertilizer
				if (! str_tolower(line).starts_with("soil-to-fertilizer map:")) {
					std::cerr << "Error: expected 'soil-to-fertilizer map:' on line "
						<< aa << std::endl;
					return;
				}
				load_state = 4;
				break;
			}
			case 4: { // soil2fertilizer
				auto vals = parse_amap(line);
				if (vals) {
					soil2fertilizer.push_back(vals.value());
					break;
				}
				else {
					load_state = 5;
					// and fall through
				}
			}
			case 5: { // fertilizer2water
				if (! str_tolower(line).starts_with("fertilizer-to-water map:")) {
					std::cerr << "Error: expected 'fertilizer-to-water map:' on line "
						<< aa << std::endl;
					return;
				}
				load_state = 6;
				break;
			}
			case 6: { // fertilizer2water
				auto vals = parse_amap(line);
				if (vals) {
					fertilizer2water.push_back(vals.value());
					break;
				}
				else {
					load_state = 7;
					// and fall through
				}
			}
			case 7: { // water2light
				if (! str_tolower(line).starts_with("water-to-light map:")) {
					std::cerr << "Error: expected 'water-to-light map:' on line "
						<< aa << std::endl;
					return;
				}
				load_state = 8;
				break;
			}
			case 8: { // water2light
				auto vals = parse_amap(line);
				if (vals) {
					water2light.push_back(vals.value());
					break;
				}
				else {
					load_state = 9;
					// and fall through
				}
			}
			case 9: { // light2temp
				if (! str_tolower(line).starts_with("light-to-temperature map:")) {
					std::cerr << "Error: expected 'light-to-temperature map:' on line "
						<< aa << std::endl;
					return;
				}
				load_state = 10;
				break;
			}
			case 10: { // light2temp
				auto vals = parse_amap(line);
				if (vals) {
					light2temp.push_back(vals.value());
					break;
				}
				else {
					load_state = 11;
					// and fall through
				}
			}
			case 11: { // temp2humidity
				if (! str_tolower(line).starts_with("temperature-to-humidity map:")) {
					std::cerr << "Error: expected 'temperature-to-humidity map:' on line "
						<< aa << std::endl;
					return;
				}
				load_state = 12;
				break;
			}
			case 12: { // temp2humidity
				auto vals = parse_amap(line);
				if (vals) {
					temp2humidity.push_back(vals.value());
					break;
				}
				else {
					load_state = 13;
					// and fall through
				}
			}
			case 13: { // humidity2location
				if (! str_tolower(line).starts_with("humidity-to-location map:")) {
					std::cerr << "Error: expected 'humidity-to-location map:' on line "
						<< aa << std::endl;
					return;
				}
				load_state = 14;
				break;
			}
			case 14: { // humidity2location
				auto vals = parse_amap(line);
				if (vals) {
					humidity2location.push_back(vals.value());
					break;
				}
				else {
					std::cerr << "Warning: unknown junk at end of file, '"
						<< line << "' on line " << aa << std::endl;
				}
				break;
			}
		}
	}
	std::sort(seed2soil.begin(), seed2soil.end());
	std::sort(soil2fertilizer.begin(), soil2fertilizer.end());
	std::sort(fertilizer2water.begin(), fertilizer2water.end());
	std::sort(water2light.begin(), water2light.end());
	std::sort(light2temp.begin(), light2temp.end());
	std::sort(temp2humidity.begin(), temp2humidity.end());
	std::sort(humidity2location.begin(), humidity2location.end());
}

std::string Almanac::str() const {
	std::stringstream out;
	auto dump_map = [&out](const Almanac::amap_t& amap) -> void {
		for (const auto& aa : amap) {
			const auto& [key, val] = aa;
			out << "  " << key.lower << "..." << key.upper
				<< " -> " << val << "..." << (val + (key.upper-key.lower)) << "\n";
		}
	};
	out << "seeds: " << join(seeds) << "\n";
	out << "seed-to-soil:\n";
	dump_map(seed2soil);
	out << "soil-to-fertilizer:\n";
	dump_map(soil2fertilizer);
	out << "fertilizer-to-water:\n";
	dump_map(fertilizer2water);
	out << "water-to-light:\n";
	dump_map(water2light);
	out << "light-to-temp:\n";
	dump_map(light2temp);
	out << "temp-to-humidity:\n";
	dump_map(temp2humidity);
	out << "humidity-to-location:\n";
	dump_map(humidity2location);
	return out.str();
}

std::optional<Almanac::amap_t::value_type> Almanac::parse_amap(const std::string& input) {
	std::optional<Almanac::amap_t::value_type> result;
	std::vector<std::string> nums = split(input);
	try {
		std::optional<int64_t> dst_start;
		std::optional<int64_t> src_start;
		std::optional<int64_t> len;
		for (const std::string& nn : nums) {
			if (nn.empty()) { continue; }
			else if (! dst_start) { dst_start = std::stoll(nn); }
			else if (! src_start) { src_start = std::stoll(nn); }
			else if (! len) { len = std::stoll(nn); }
			else { break; }
		}
		if (! len) {
			return result;
		}
		ClosedInterval srci(src_start.value(), src_start.value()+len.value()-1);
		srci.normalize();
		result = Almanac::amap_t::value_type{srci, dst_start.value()};
	}
	catch (...) {
		result = std::nullopt;
	}
	return result;
}

int64_t Almanac::amap_lookup(const amap_t& amap, const int64_t inputval) {
	if (amap.empty()) { return inputval; }
	bool found = false;
	int64_t outputval = 0;
	const size_t greater_idx
		= std::distance(amap.begin(), std::upper_bound(amap.begin(), amap.end(), inputval));
	size_t idx = greater_idx;
	if (idx == amap.size()) {
		idx--;
	}
	while (true) {
		if (amap[idx].src.contains(inputval)) {
			outputval = amap[idx].dst + (inputval - amap[idx].src.lower);
			found = true;
			break;
		}
		if (idx == 0) { break; }
		idx--;
	}
	if (! found) {
		outputval = inputval;
	}
	return outputval;
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

int64_t part1_answer(const Almanac& ref) {
	std::vector<int64_t> locations;
	std::transform(ref.seeds.cbegin(), ref.seeds.cend(), std::back_inserter(locations),
		[&ref](const int64_t seed) { return ref.get_location_for_seed(seed); });
	// dump(locations);
	if (locations.empty()) { return 0; }
	return *std::min_element(locations.cbegin(), locations.cend());
}

int64_t part2_answer(const Almanac& ref) {
	int64_t min_location = LLONG_MAX; // 9223372036854775807LL
	for (size_t ss=0; ss<ref.seeds.size(); ss+=2) {
		for (int64_t seed=ref.seeds[ss]; seed<ref.seeds[ss]+ref.seeds[ss+1]; seed++) {
			int64_t location = ref.get_location_for_seed(seed);
			if (location < min_location) {
				min_location = location;
			}
		}
	}
	return min_location;
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
	Almanac islandisland(input, part);
	// std::cout << islandisland.str();
	if (part == 2) {
		std::cout << part2_answer(islandisland) << std::endl;
	}
	else {
		std::cout << part1_answer(islandisland) << std::endl;
	}
	std::cout << "done" << std::endl;
	return 0;
}


/*
--- Day 5: If You Give A Seed A Fertilizer ---
You take the boat and find the gardener right where you were told he would be: managing a giant "garden" that looks more to you like a farm.

"A water source? Island Island is the water source!" You point out that Snow Island isn't receiving any water.

"Oh, we had to stop the water because we ran out of sand to filter it with! Can't make snow with dirty water. Don't worry, I'm sure we'll get more sand soon; we only turned off the water a few days... weeks... oh no." His face sinks into a look of horrified realization.

"I've been so busy making sure everyone here has food that I completely forgot to check why we stopped getting more sand! There's a ferry leaving soon that is headed over in that direction - it's much faster than your boat. Could you please go check it out?"

You barely have time to agree to this request when he brings up another. "While you wait for the ferry, maybe you can help us with our food production problem. The latest Island Island Almanac just arrived and we're having trouble making sense of it."

The almanac (your puzzle input) lists all of the seeds that need to be planted. It also lists what type of soil to use with each kind of seed, what type of fertilizer to use with each kind of soil, what type of water to use with each kind of fertilizer, and so on. Every type of seed, soil, fertilizer and so on is identified with a number, but numbers are reused by each category - that is, soil 123 and fertilizer 123 aren't necessarily related to each other.

For example:

seeds: 79 14 55 13

seed-to-soil map:
50 98 2
52 50 48

soil-to-fertilizer map:
0 15 37
37 52 2
39 0 15

fertilizer-to-water map:
49 53 8
0 11 42
42 0 7
57 7 4

water-to-light map:
88 18 7
18 25 70

light-to-temperature map:
45 77 23
81 45 19
68 64 13

temperature-to-humidity map:
0 69 1
1 0 69

humidity-to-location map:
60 56 37
56 93 4
The almanac starts by listing which seeds need to be planted: seeds 79, 14, 55, and 13.

The rest of the almanac contains a list of maps which describe how to convert numbers from a source category into numbers in a destination category. That is, the section that starts with seed-to-soil map: describes how to convert a seed number (the source) to a soil number (the destination). This lets the gardener and his team know which soil to use with which seeds, which water to use with which fertilizer, and so on.

Rather than list every source number and its corresponding destination number one by one, the maps describe entire ranges of numbers that can be converted. Each line within a map contains three numbers: the destination range start, the source range start, and the range length.

Consider again the example seed-to-soil map:

50 98 2
52 50 48
The first line has a destination range start of 50, a source range start of 98, and a range length of 2. This line means that the source range starts at 98 and contains two values: 98 and 99. The destination range is the same length, but it starts at 50, so its two values are 50 and 51. With this information, you know that seed number 98 corresponds to soil number 50 and that seed number 99 corresponds to soil number 51.

The second line means that the source range starts at 50 and contains 48 values: 50, 51, ..., 96, 97. This corresponds to a destination range starting at 52 and also containing 48 values: 52, 53, ..., 98, 99. So, seed number 53 corresponds to soil number 55.

Any source numbers that aren't mapped correspond to the same destination number. So, seed number 10 corresponds to soil number 10.

So, the entire list of seed numbers and their corresponding soil numbers looks like this:

seed  soil
0     0
1     1
...   ...
48    48
49    49
50    52
51    53
...   ...
96    98
97    99
98    50
99    51
With this map, you can look up the soil number required for each initial seed number:

Seed number 79 corresponds to soil number 81.
Seed number 14 corresponds to soil number 14.
Seed number 55 corresponds to soil number 57.
Seed number 13 corresponds to soil number 13.
The gardener and his team want to get started as soon as possible, so they'd like to know the closest location that needs a seed. Using these maps, find the lowest location number that corresponds to any of the initial seeds. To do this, you'll need to convert each seed number through other categories until you can find its corresponding location number. In this example, the corresponding types are:

Seed 79, soil 81, fertilizer 81, water 81, light 74, temperature 78, humidity 78, location 82.
Seed 14, soil 14, fertilizer 53, water 49, light 42, temperature 42, humidity 43, location 43.
Seed 55, soil 57, fertilizer 57, water 53, light 46, temperature 82, humidity 82, location 86.
Seed 13, soil 13, fertilizer 52, water 41, light 34, temperature 34, humidity 35, location 35.
So, the lowest location number in this example is 35.

What is the lowest location number that corresponds to any of the initial seed numbers?

Your puzzle answer was 265018614.

--- Part Two ---
Everyone will starve if you only plant such a small number of seeds. Re-reading the almanac, it looks like the seeds: line actually describes ranges of seed numbers.

The values on the initial seeds: line come in pairs. Within each pair, the first value is the start of the range and the second value is the length of the range. So, in the first line of the example above:

seeds: 79 14 55 13
This line describes two ranges of seed numbers to be planted in the garden. The first range starts with seed number 79 and contains 14 values: 79, 80, ..., 91, 92. The second range starts with seed number 55 and contains 13 values: 55, 56, ..., 66, 67.

Now, rather than considering four seed numbers, you need to consider a total of 27 seed numbers.

In the above example, the lowest location number can be obtained from seed number 82, which corresponds to soil 84, fertilizer 84, water 84, light 77, temperature 45, humidity 46, and location 46. So, the lowest location number is 46.

Consider all of the initial seed numbers listed in the ranges on the first line of the almanac. What is the lowest location number that corresponds to any of the initial seed numbers?

Your puzzle answer was 63179500.

Both parts of this puzzle are complete! They provide two gold stars: **
*/

