// c++ -Wall -Wextra -pedantic -O3 -std=c++20 day04.cpp -o day04 && time ./day04 day04in.txt 2
// v1 works fine, but chews up too much memory, so rework to only count children, not actually copy them
#include <algorithm>
#include <compare>
#include <fstream>
#include <functional>
#include <iostream>
#include <list>
#include <map>
#include <numeric>
// #include <ranges> // my clang doesn't support :-(
#include <optional>
#include <sstream>
#include <tuple>
#include <string>
#include <vector>
#include <utility>


class LottoCard {
	public:
	std::vector<int> winners;
	std::vector<int> held;
	size_t child_count;
	std::string id;

	LottoCard() : child_count(0) { }
	explicit LottoCard(const std::string&, const std::string& parent_id="");
	LottoCard(const LottoCard&) = default;
	LottoCard(LottoCard&&) = default;

	std::vector<int> held_winners() const;
	uint64_t value() const;
	std::string str(const std::string& prefix="") const;
	size_t size() const { return 1U + child_count; }
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
template <>
void dump(const std::vector<int>& input) {
	for (size_t aa=0; aa<input.size(); aa++) {
		std::cout << aa << ": " << input[aa] << std::endl;
	}
}
template <>
void dump(const std::vector<uint64_t>& input) {
	for (size_t aa=0; aa<input.size(); aa++) {
		std::cout << aa << ": " << input[aa] << std::endl;
	}
}
template <>
void dump(const std::vector<std::tuple<LottoCard, std::vector<int>, uint64_t>>& input) {
	for (size_t aa=0; aa<input.size(); aa++) {
		std::stringstream vistr;
		for (const int cc : std::get<1>(input[aa])) {
			vistr << " " << cc;
		}
		std::cout << aa << ": " << std::get<0>(input[aa]).str()
			<< " ||" << vistr.str()
			<< " ==> " << std::get<2>(input[aa]) << std::endl;
	}
}

LottoCard::LottoCard(const std::string& input, const std::string& parent_id) : child_count(0) {
	if (! (input.starts_with("card") || input.starts_with("Card"))) {
		std::cerr << "Error: invalid card " << input << std::endl;
	}
	std::string token;
	size_t cidx = 4; // start after "card"
	auto premature_end_err = [&input]() {
		std::cerr << "Error: premature end of card " << input << std::endl;
	};
	auto eat_ws = [&input, &cidx]() -> bool {
		bool moved =false;
		while (cidx<input.size() && std::isspace(static_cast<unsigned char>(input[cidx]))) {
			cidx++;
			moved = true;
		}
		return moved;
	};
	auto get_number = [&input, &cidx](auto save_to) -> bool {
		size_t start_idx = cidx;
		if (input[cidx] == '-') {
			cidx++;
		}
		while (cidx<input.size() && std::isdigit(static_cast<unsigned char>(input[cidx]))) {
			cidx++;
		}
		try {
			*save_to = std::stoi(input.substr(start_idx, cidx-start_idx));
			return true;
		}
		catch (...) {
			return false;
		}
	};
	eat_ws();
	if (cidx >= input.size()) { premature_end_err(); return; }
	if (int idnum=0; get_number(&idnum)) {
		if (parent_id.empty()) {
			id = std::to_string(idnum);
		}
		else {
			id = parent_id + "." + std::to_string(idnum);
		}
	}
	else {
		std::cerr << "Error: invalid id of card " << input << std::endl;
		return;
	}
	cidx++; // :
	while (input[cidx] != '|') {
		eat_ws();
		if (cidx >= input.size()) { premature_end_err(); return; }
		get_number(std::back_inserter(winners));
		if (cidx >= input.size()) { premature_end_err(); return; }
		eat_ws();
		if (cidx >= input.size()) { premature_end_err(); return; }
	}
	cidx++; // |
	while (cidx < input.size()) {
		eat_ws();
		if (cidx >= input.size()) { premature_end_err(); return; }
		get_number(std::back_inserter(held));
	}
	// using std::cerr;
	// using std::endl;
	// if (held.size() != 25) { cerr << "Warning: not 25 numbers held for " << str() << endl; }
	// if (winners.size() != 10) { cerr << "Warning: not 10 winners for " << str() << endl; }
}

std::vector<int> LottoCard::held_winners() const {
	std::vector<int> winners_c(winners);
	std::vector<int> held_c(held);
	std::stable_sort(winners_c.begin(), winners_c.end());
	std::stable_sort(held_c.begin(), held_c.end());
	std::vector<int> common;
	std::set_intersection(winners_c.begin(), winners_c.end(),
		held_c.begin(), held_c.end(),
		std::back_inserter(common));
	return common;
}

uint64_t LottoCard::value() const {
	uint64_t value = 1;
	std::vector<int> hw = held_winners();
	if (hw.size() > 63) {
		std::cerr << "Error: value overflow for " << str() << std::endl;
		return -1;
	}
	if (hw.size() > 0) {
		value = value << (hw.size()-1);
		return value;
	}
	else {
		return 0U;
	}
}

std::string LottoCard::str(const std::string& prefix) const {
	std::stringstream out;
	out << prefix << "Card " << id << ":";
	for (const int ww : winners) {
		out << " " << ww;
	}
	out << " |";
	for (const int hh : held) {
		out << " " << hh;
	}
	out << " + " << child_count << " children";
	return out.str();
}

std::vector<LottoCard> parse_cards(const std::vector<std::string>& input, int part) {
	std::vector<LottoCard> results;
	for (const std::string& line : input) {
		if (line.empty()) { continue; }
		results.emplace_back(LottoCard(line));
	}
	if (part == 1) {
		return results;
	}
	const size_t csize = results.size();
	for (size_t rcidx=0; rcidx<csize; rcidx++) {
		// process in reverse
		size_t cidx = csize - rcidx - 1;
		LottoCard& me = results[cidx];
		const size_t num_children = me.held_winners().size();
		if (cidx + num_children >= results.size()) {
			std::cerr << "Error: missing some of " << num_children
				<< " child(ren) from input " << me.str() << std::endl;
			return results;
		}
		for (size_t cc=0; cc<num_children; cc++) {
			me.child_count += results[cidx+cc+1].size();
		}
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

int part1_answer(const std::vector<LottoCard>& cards) {
	/*
	std::vector<std::tuple<LottoCard, std::vector<int>, uint64_t>> card_values;
	std::transform(cards.begin(), cards.end(), std::back_inserter(card_values),
		[](const LottoCard& cc) -> std::tuple<LottoCard, std::vector<int>, uint64_t> {
			return {cc, cc.held_winners(), cc.value()};
		});
	dump(card_values);
	*/
	return std::transform_reduce(cards.begin(), cards.end(), 0U, std::plus<>(),
		[](const LottoCard& cc) { return cc.value(); });
}

int part2_answer(const std::vector<LottoCard>& cards) {
	// dump(cards);
	return std::transform_reduce(cards.begin(), cards.end(), 0U, std::plus<>(),
		[](const LottoCard& cc) { return cc.size(); });
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
	std::vector<LottoCard> cards = parse_cards(input, part);
	if (part == 2) {
		std::cout << part2_answer(cards) << std::endl;
	}
	else {
		std::cout << part1_answer(cards) << std::endl;
	}
	std::cout << "done" << std::endl;
	return 0;
}


/*
--- Day 4: Scratchcards ---
The gondola takes you up. Strangely, though, the ground doesn't seem to be coming with you; you're not climbing a mountain. As the circle of Snow Island recedes below you, an entire new landmass suddenly appears above you! The gondola carries you to the surface of the new island and lurches into the station.

As you exit the gondola, the first thing you notice is that the air here is much warmer than it was on Snow Island. It's also quite humid. Is this where the water source is?

The next thing you notice is an Elf sitting on the floor across the station in what seems to be a pile of colorful square cards.

"Oh! Hello!" The Elf excitedly runs over to you. "How may I be of service?" You ask about water sources.

"I'm not sure; I just operate the gondola lift. That does sound like something we'd have, though - this is Island Island, after all! I bet the gardener would know. He's on a different island, though - er, the small kind surrounded by water, not the floating kind. We really need to come up with a better naming scheme. Tell you what: if you can help me with something quick, I'll let you borrow my boat and you can go visit the gardener. I got all these scratchcards as a gift, but I can't figure out what I've won."

The Elf leads you over to the pile of colorful cards. There, you discover dozens of scratchcards, all with their opaque covering already scratched off. Picking one up, it looks like each card has two lists of numbers separated by a vertical bar (|): a list of winning numbers and then a list of numbers you have. You organize the information into a table (your puzzle input).

As far as the Elf has been able to figure out, you have to figure out which of the numbers you have appear in the list of winning numbers. The first match makes the card worth one point and each match after the first doubles the point value of that card.

For example:

Card 1: 41 48 83 86 17 | 83 86  6 31 17  9 48 53
Card 2: 13 32 20 16 61 | 61 30 68 82 17 32 24 19
Card 3:  1 21 53 59 44 | 69 82 63 72 16 21 14  1
Card 4: 41 92 73 84 69 | 59 84 76 51 58  5 54 83
Card 5: 87 83 26 28 32 | 88 30 70 12 93 22 82 36
Card 6: 31 18 13 56 72 | 74 77 10 23 35 67 36 11
In the above example, card 1 has five winning numbers (41, 48, 83, 86, and 17) and eight numbers you have (83, 86, 6, 31, 17, 9, 48, and 53). Of the numbers you have, four of them (48, 83, 17, and 86) are winning numbers! That means card 1 is worth 8 points (1 for the first match, then doubled three times for each of the three matches after the first).

Card 2 has two winning numbers (32 and 61), so it is worth 2 points.
Card 3 has two winning numbers (1 and 21), so it is worth 2 points.
Card 4 has one winning number (84), so it is worth 1 point.
Card 5 has no winning numbers, so it is worth no points.
Card 6 has no winning numbers, so it is worth no points.
So, in this example, the Elf's pile of scratchcards is worth 13 points.

Take a seat in the large pile of colorful cards. How many points are they worth in total?

Your puzzle answer was 24175.

--- Part Two ---
Just as you're about to report your findings to the Elf, one of you realizes that the rules have actually been printed on the back of every card this whole time.

There's no such thing as "points". Instead, scratchcards only cause you to win more scratchcards equal to the number of winning numbers you have.

Specifically, you win copies of the scratchcards below the winning card equal to the number of matches. So, if card 10 were to have 5 matching numbers, you would win one copy each of cards 11, 12, 13, 14, and 15.

Copies of scratchcards are scored like normal scratchcards and have the same card number as the card they copied. So, if you win a copy of card 10 and it has 5 matching numbers, it would then win a copy of the same cards that the original card 10 won: cards 11, 12, 13, 14, and 15. This process repeats until none of the copies cause you to win any more cards. (Cards will never make you copy a card past the end of the table.)

This time, the above example goes differently:

Card 1: 41 48 83 86 17 | 83 86  6 31 17  9 48 53
Card 2: 13 32 20 16 61 | 61 30 68 82 17 32 24 19
Card 3:  1 21 53 59 44 | 69 82 63 72 16 21 14  1
Card 4: 41 92 73 84 69 | 59 84 76 51 58  5 54 83
Card 5: 87 83 26 28 32 | 88 30 70 12 93 22 82 36
Card 6: 31 18 13 56 72 | 74 77 10 23 35 67 36 11
Card 1 has four matching numbers, so you win one copy each of the next four cards: cards 2, 3, 4, and 5.
Your original card 2 has two matching numbers, so you win one copy each of cards 3 and 4.
Your copy of card 2 also wins one copy each of cards 3 and 4.
Your four instances of card 3 (one original and three copies) have two matching numbers, so you win four copies each of cards 4 and 5.
Your eight instances of card 4 (one original and seven copies) have one matching number, so you win eight copies of card 5.
Your fourteen instances of card 5 (one original and thirteen copies) have no matching numbers and win no more cards.
Your one instance of card 6 (one original) has no matching numbers and wins no more cards.
Once all of the originals and copies have been processed, you end up with 1 instance of card 1, 2 instances of card 2, 4 instances of card 3, 8 instances of card 4, 14 instances of card 5, and 1 instance of card 6. In total, this example pile of scratchcards causes you to ultimately have 30 scratchcards!

Process all of the original and copied scratchcards until no more scratchcards are won. Including the original set of scratchcards, how many total scratchcards do you end up with?

Your puzzle answer was 18846301.

Both parts of this puzzle are complete! They provide two gold stars: **

*/
