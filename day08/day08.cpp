// c++ -Wall -Wextra -pedantic -O3 -std=c++20 day08.cpp -o day08 && time ./day08 day08in.txt 1
// vi:set noet ts=4 sw=4:
#include <algorithm>
#include <compare>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <numeric>
#include <regex>
#include <set>
#include <sstream>
#include <string>
#include <vector>
#include <utility>

class MapNode {
	public:
	// std::string id;
	std::string left;
	std::string right;

	MapNode() { }
	// MapNode(const std::string& ii, const std::string& ll, const std::string& rr)
	// 	: id(ii), left(ll), right(rr) { }
	MapNode(const std::string& ll, const std::string& rr) : left(ll), right(rr) { }

	std::string str() const{
		std::stringstream out;
		out << "{" << left << ", " << right << "}";
		return out.str();
	}
};
typedef std::map<std::string, MapNode> desertmap_t;

void dump(const desertmap_t& input) {
	for (const auto& [kk, vv] : input) {
		std::cout << kk << " -> " << vv.str() << std::endl;
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

desertmap_t parse_mapnodes(const std::vector<std::string>& input)
{
	desertmap_t result;
	for (const std::string& line : input) {
		if (line.empty()) { continue; }
		MapNode thisnode;
		std::smatch matches;
		if (std::regex_match(line.begin(), line.end(), matches,
				std::regex(R"((\w+)\s*=\s*\((\w+),\s*(\w+)\))")))
		{
			std::string id = matches[1];
			thisnode.left = matches[2];
			thisnode.right = matches[3];
			if (result.contains(id)) {
				std::cerr << "Warning: replacing map node " << id << std::endl;
			}
			result[id] = thisnode;
		}
		else {
			std::cerr << "Error: non-conformant line " << line << std::endl;
		}
	}
	return result;
}

size_t count_steps(const desertmap_t& map, const std::string& path,
		const std::string& from, const std::string& to)
{
	size_t steps = 0;
	bool found = false;
	desertmap_t::const_iterator where = map.find(from);
	while (where != map.end()) {
		// std::cout << "step " << steps << " at " << where->first
		// 	<< " going " << path[steps % path.size()]
		// 	<< " to " << where->second.str() << std::endl;
		if (where->first == to) {
			found = true;
			break;
		}
		if (path[steps % path.size()] == 'R') {
			where = map.find(where->second.right);
		}
		else if (path[steps % path.size()] == 'L') {
			where = map.find(where->second.left);
		}
		else {
			std::cerr << "Error: invalid path movement " << path[steps % path.size()] << std::endl;
			return steps;
		}
		steps++;
	}
	if (! found) {
		std::cerr << "Error: node missing from map" << std::endl;
	}
	return steps;
}

desertmap_t::const_iterator advance(const desertmap_t& map,
		const std::string& path, const size_t step, const desertmap_t::const_iterator& from)
{
	const char which_way = path[step % path.size()];
	return map.find(which_way == 'R' ? from->second.right : from->second.left);
}

size_t part2_answer(const desertmap_t& map, const std::string& path) {
	std::vector<desertmap_t::const_iterator> whence;
	std::vector<uint64_t> common_factors;
	for (desertmap_t::const_iterator mm=map.begin(); mm!=map.end(); ++mm) {
		if (mm->first.ends_with("A")) {
			whence.push_back(mm);
		}
		else {
			continue;
		}
		desertmap_t::const_iterator camel = whence.back();
		size_t ss = 0;
		for (/*no-op*/; (camel!=map.end()) && !camel->first.ends_with("Z"); ss++) {
			camel = advance(map, path, ss, camel);
		}
		// Had to discover this through experimentation
		// std::cout << whence.back()->first << " ends at " << camel->first
		// 	<< " after " << ss << " steps (" << (ss/path.size()) << " and "
		// 	<< (ss % path.size()) << "/" << path.size() << " paths)" << std::endl;
		if (ss % path.size() != 0) {
			std::cerr << "Warning: path " << whence.back()->first
				<< " does not end on a path-length multiple" << std::endl;
		}
		common_factors.push_back(ss/path.size());
	}
	// dump(common_factors);
	return std::reduce(common_factors.begin(), common_factors.end(), path.size(),
		[](const uint64_t prior_lcm, const uint64_t fac) { return std::lcm(prior_lcm, fac); });
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
	if (input.empty()) {
		std::cerr << "Error: no input" << std::endl;
		return 1;
	}
	std::string path = input[0];
	input.erase(input.begin());
	desertmap_t map = parse_mapnodes(input);
	// dump(map);
	if (part == 2) {
		std::cout << part2_answer(map, path) << std::endl;
	}
	else {
		std::cout << count_steps(map, path, "AAA", "ZZZ") << std::endl;
	}
	return 0;
}


/*
--- Day 8: Haunted Wasteland ---
You're still riding a camel across Desert Island when you spot a sandstorm quickly approaching. When you turn to warn the Elf, she disappears before your eyes! To be fair, she had just finished warning you about ghosts a few minutes ago.

One of the camel's pouches is labeled "maps" - sure enough, it's full of documents (your puzzle input) about how to navigate the desert. At least, you're pretty sure that's what they are; one of the documents contains a list of left/right instructions, and the rest of the documents seem to describe some kind of network of labeled nodes.

It seems like you're meant to use the left/right instructions to navigate the network. Perhaps if you have the camel follow the same instructions, you can escape the haunted wasteland!

After examining the maps for a bit, two nodes stick out: AAA and ZZZ. You feel like AAA is where you are now, and you have to follow the left/right instructions until you reach ZZZ.

This format defines each node of the network individually. For example:

RL

AAA = (BBB, CCC)
BBB = (DDD, EEE)
CCC = (ZZZ, GGG)
DDD = (DDD, DDD)
EEE = (EEE, EEE)
GGG = (GGG, GGG)
ZZZ = (ZZZ, ZZZ)
Starting with AAA, you need to look up the next element based on the next left/right instruction in your input. In this example, start with AAA and go right (R) by choosing the right element of AAA, CCC. Then, L means to choose the left element of CCC, ZZZ. By following the left/right instructions, you reach ZZZ in 2 steps.

Of course, you might not find ZZZ right away. If you run out of left/right instructions, repeat the whole sequence of instructions as necessary: RL really means RLRLRLRLRLRLRLRL... and so on. For example, here is a situation that takes 6 steps to reach ZZZ:

LLR

AAA = (BBB, BBB)
BBB = (AAA, ZZZ)
ZZZ = (ZZZ, ZZZ)
Starting at AAA, follow the left/right instructions. How many steps are required to reach ZZZ?

Your puzzle answer was 11911.

--- Part Two ---
The sandstorm is upon you and you aren't any closer to escaping the wasteland. You had the camel follow the instructions, but you've barely left your starting position. It's going to take significantly more steps to escape!

What if the map isn't for people - what if the map is for ghosts? Are ghosts even bound by the laws of spacetime? Only one way to find out.

After examining the maps a bit longer, your attention is drawn to a curious fact: the number of nodes with names ending in A is equal to the number ending in Z! If you were a ghost, you'd probably just start at every node that ends with A and follow all of the paths at the same time until they all simultaneously end up at nodes that end with Z.

For example:

LR

11A = (11B, XXX)
11B = (XXX, 11Z)
11Z = (11B, XXX)
22A = (22B, XXX)
22B = (22C, 22C)
22C = (22Z, 22Z)
22Z = (22B, 22B)
XXX = (XXX, XXX)
Here, there are two starting nodes, 11A and 22A (because they both end with A). As you follow each left/right instruction, use that instruction to simultaneously navigate away from both nodes you're currently on. Repeat this process until all of the nodes you're currently on end with Z. (If only some of the nodes you're on end with Z, they act like any other node and you continue as normal.) In this example, you would proceed as follows:

Step 0: You are at 11A and 22A.
Step 1: You choose all of the left paths, leading you to 11B and 22B.
Step 2: You choose all of the right paths, leading you to 11Z and 22C.
Step 3: You choose all of the left paths, leading you to 11B and 22Z.
Step 4: You choose all of the right paths, leading you to 11Z and 22B.
Step 5: You choose all of the left paths, leading you to 11B and 22C.
Step 6: You choose all of the right paths, leading you to 11Z and 22Z.
So, in this example, you end up entirely on nodes that end in Z after 6 steps.

Simultaneously start on every node that ends with A. How many steps does it take before you're only on nodes that end with Z?

Your puzzle answer was 10151663816849.

Both parts of this puzzle are complete! They provide two gold stars: **
*/

