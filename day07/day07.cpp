// c++ -Wall -Wextra -pedantic -O3 -std=c++20 day07.cpp -o day07 && time ./day07 day07in.txt 1
// vi:set noet ts=4 sw=4:
#include <algorithm>
#include <array>
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

enum class CardLabelv1 {
	// leave 0 as an invalid value
	C2=1, C3, C4, C5, C6, C7, C8, C9, CT, CJ, CQ, CK, CA
};
CardLabelv1 make_cardlabel1(const char cc) {
	switch (cc) {
		case '2': return CardLabelv1::C2;
		case '3': return CardLabelv1::C3;
		case '4': return CardLabelv1::C4;
		case '5': return CardLabelv1::C5;
		case '6': return CardLabelv1::C6;
		case '7': return CardLabelv1::C7;
		case '8': return CardLabelv1::C8;
		case '9': return CardLabelv1::C9;
		case 'T': return CardLabelv1::CT;
		case 'J': return CardLabelv1::CJ;
		case 'Q': return CardLabelv1::CQ;
		case 'K': return CardLabelv1::CK;
		case 'A': return CardLabelv1::CA;
		default: {
			std::cerr << "Error: invalid CardLabelv1 " << cc << std::endl;
			return static_cast<CardLabelv1>(0);
		}
	}
}
std::ostream& operator<<(std::ostream& out, const CardLabelv1& val) {
	switch (val) {
		case CardLabelv1::C2: out << "2"; break;
		case CardLabelv1::C3: out << "3"; break;
		case CardLabelv1::C4: out << "4"; break;
		case CardLabelv1::C5: out << "5"; break;
		case CardLabelv1::C6: out << "6"; break;
		case CardLabelv1::C7: out << "7"; break;
		case CardLabelv1::C8: out << "8"; break;
		case CardLabelv1::C9: out << "9"; break;
		case CardLabelv1::CT: out << "T"; break;
		case CardLabelv1::CJ: out << "J"; break;
		case CardLabelv1::CQ: out << "Q"; break;
		case CardLabelv1::CK: out << "K"; break;
		case CardLabelv1::CA: out << "A"; break;
		default: out << "??";
	}
	return out;
}

enum class CardLabelv2 {
	// leave 0 as an invalid value
	CJ=1, C2, C3, C4, C5, C6, C7, C8, C9, CT, CQ, CK, CA
};
CardLabelv2 make_cardlabel2(const char cc) {
	switch (cc) {
		case 'J': return CardLabelv2::CJ;
		case '2': return CardLabelv2::C2;
		case '3': return CardLabelv2::C3;
		case '4': return CardLabelv2::C4;
		case '5': return CardLabelv2::C5;
		case '6': return CardLabelv2::C6;
		case '7': return CardLabelv2::C7;
		case '8': return CardLabelv2::C8;
		case '9': return CardLabelv2::C9;
		case 'T': return CardLabelv2::CT;
		case 'Q': return CardLabelv2::CQ;
		case 'K': return CardLabelv2::CK;
		case 'A': return CardLabelv2::CA;
		default: {
			std::cerr << "Error: invalid CardLabelv2 " << cc << std::endl;
			return static_cast<CardLabelv2>(0);
		}
	}
}
std::ostream& operator<<(std::ostream& out, const CardLabelv2& val) {
	switch (val) {
		case CardLabelv2::CJ: out << "J"; break;
		case CardLabelv2::C2: out << "2"; break;
		case CardLabelv2::C3: out << "3"; break;
		case CardLabelv2::C4: out << "4"; break;
		case CardLabelv2::C5: out << "5"; break;
		case CardLabelv2::C6: out << "6"; break;
		case CardLabelv2::C7: out << "7"; break;
		case CardLabelv2::C8: out << "8"; break;
		case CardLabelv2::C9: out << "9"; break;
		case CardLabelv2::CT: out << "T"; break;
		case CardLabelv2::CQ: out << "Q"; break;
		case CardLabelv2::CK: out << "K"; break;
		case CardLabelv2::CA: out << "A"; break;
		default: out << "??";
	}
	return out;
}

enum class HandType { // (ranked)
	// leave 0 as an invalid value
	M0=1, M2, M2M2, M3, M3M2, M4, M5
};
std::ostream& operator<<(std::ostream& out, const HandType& val) {
	switch (val) {
		case HandType::M5:   out << "five-of-a-kind"; break;
		case HandType::M4:   out << "four-of-a-kind"; break;
		case HandType::M3M2: out << "full-house"; break;
		case HandType::M3:   out << "three-of-a-kind"; break;
		case HandType::M2M2: out << "two-pair"; break;
		case HandType::M2:   out << "one-pair"; break;
		case HandType::M0:   out << "high-card"; break;
		default: out << "???";
	}
	return out;
}

class CamelHandv1 {
	public:
	std::array<CardLabelv1, 5> cards;
	int bid;

	CamelHandv1(const std::string& input);
	std::strong_ordering operator<=>(const CamelHandv1&) const;

	std::string str() const;
	HandType handtype() const; // highest matching hand type
};

class CamelHandv2 {
	public:
	std::array<CardLabelv2, 5> cards;
	int bid;

	CamelHandv2(const std::string& input);
	std::strong_ordering operator<=>(const CamelHandv2&) const;

	std::string str() const;
	// return highest matching hand type, and joker-value used (or CJ if unused)
	std::tuple<HandType,CardLabelv2> handtype() const;
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

CamelHandv1::CamelHandv1(const std::string& input) {
	size_t idx = 0;
	for (/*no-op*/; idx<5; idx++) {
		cards[idx] = make_cardlabel1(input[idx]);
	}
	try {
		bid = std::stoi(input.substr(idx));
	}
	catch (...) {
		std::cerr << "Error: invalid bid in input, " << input << std::endl;
	}
}

std::strong_ordering CamelHandv1::operator<=>(const CamelHandv1& other) const {
	if (auto cmp = handtype()<=>other.handtype(); cmp!=0) {
		return cmp;
	}
	for (size_t aa=0; aa<5; aa++) {
		if (auto cmp = static_cast<int>(cards[aa])<=>static_cast<int>(other.cards[aa]); cmp!=0) {
			return cmp;
		}
	}
	return bid<=>other.bid;
}

std::string CamelHandv1::str() const {
	std::stringstream out;
	out << "hand{";
	for (const CardLabelv1& cc : cards) {
		out << cc;
	}
	out << " (" << handtype() << "), ";
	out << bid << "}";
	return out.str();
}

HandType CamelHandv1::handtype() const {
	std::array<size_t, 14> counts;
	counts.fill(0);
	for (const CardLabelv1& cc : cards) {
		counts[static_cast<size_t>(cc)]++;
	}
	size_t doubles = 0;
	size_t triples = 0;
	for (size_t aa=1; aa<counts.size(); aa++) {
		if (counts[aa] >= 5) {
			return HandType::M5;
		}
		else if (counts[aa] >= 4) {
			return HandType::M4;
		}
		else if (counts[aa] >= 3) {
			triples++;
		}
		else if (counts[aa] >= 2) {
			doubles++;
		}
	}
	if ((triples > 0) && (doubles > 0)) {
		return HandType::M3M2;
	}
	else if (triples > 0) {
		return HandType::M3;
	}
	else if (doubles > 1) {
		return HandType::M2M2;
	}
	else if (doubles > 0) {
		return HandType::M2;
	}
	return HandType::M0;
}

CamelHandv2::CamelHandv2(const std::string& input) {
	size_t idx = 0;
	for (/*no-op*/; idx<5; idx++) {
		cards[idx] = make_cardlabel2(input[idx]);
	}
	try {
		bid = std::stoi(input.substr(idx));
	}
	catch (...) {
		std::cerr << "Error: invalid bid in input, " << input << std::endl;
	}
}

std::strong_ordering CamelHandv2::operator<=>(const CamelHandv2& other) const {
	if (auto cmp = std::get<0>(handtype())<=>std::get<0>(other.handtype()); cmp!=0) {
		return cmp;
	}
	for (size_t aa=0; aa<5; aa++) {
		if (auto cmp = static_cast<int>(cards[aa])<=>static_cast<int>(other.cards[aa]); cmp!=0) {
			return cmp;
		}
	}
	return bid<=>other.bid;
}

std::string CamelHandv2::str() const {
	std::stringstream out;
	out << "hand{";
	for (const CardLabelv2& cc : cards) {
		out << cc;
	}
	auto [my_type, my_jval] = handtype();
	out << " (" << my_type;
	if (std::find(cards.begin(), cards.end(), CardLabelv2::CJ) != cards.end()) {
		out << ";j=" << my_jval;
	}
	out << "), " << bid << "}";
	return out.str();
}

std::tuple<HandType,CardLabelv2> CamelHandv2::handtype() const {
	std::tuple<HandType,CardLabelv2> result;
	std::array<size_t, 14> counts;
	counts.fill(0);
	for (const CardLabelv2& cc : cards) {
		counts[static_cast<size_t>(cc)]++;
	}
	const size_t cj_idx = static_cast<size_t>(CardLabelv2::CJ);
	if (counts[cj_idx] > 0) {
		size_t j_count = counts[cj_idx];
		counts[cj_idx] = 0;
		if (j_count == 5) {
			counts[static_cast<size_t>(CardLabelv2::CA)] = 5;
		}
		else {
			auto most_iter = std::max_element(counts.begin(), counts.end());
			(*most_iter) += j_count;
			std::get<1>(result) = static_cast<CardLabelv2>(std::distance(counts.begin(), most_iter));
		}
	}
	size_t doubles = 0;
	size_t triples = 0;
	for (size_t aa=1; aa<counts.size(); aa++) {
		if (counts[aa] >= 5) {
			std::get<0>(result) = HandType::M5;
			return result;
		}
		else if (counts[aa] >= 4) {
			std::get<0>(result) = HandType::M4;
			return result;
		}
		else if (counts[aa] >= 3) {
			triples++;
		}
		else if (counts[aa] >= 2) {
			doubles++;
		}
	}
	if ((triples > 0) && (doubles > 0)) {
		std::get<0>(result) = HandType::M3M2;
	}
	else if (triples > 0) {
		std::get<0>(result) = HandType::M3;
	}
	else if (doubles > 1) {
		std::get<0>(result) = HandType::M2M2;
	}
	else if (doubles > 0) {
		std::get<0>(result) = HandType::M2;
	}
	else {
		std::get<0>(result) = HandType::M0;
	}
	return result;
}

std::vector<CamelHandv1> parse_handsv1(const std::vector<std::string>& input) {
	std::vector<CamelHandv1> results;
	for (const std::string& line : input) {
		if (line.empty()) { continue; }
		results.emplace_back(line);
	}
	return results;
}

std::vector<CamelHandv2> parse_handsv2(const std::vector<std::string>& input) {
	std::vector<CamelHandv2> results;
	for (const std::string& line : input) {
		if (line.empty()) { continue; }
		results.emplace_back(line);
	}
	return results;
}

template<typename TT>
uint64_t answer(const std::vector<TT>& hands) {
	std::vector<TT> hands_sorted(hands);
	std::sort(hands_sorted.begin(), hands_sorted.end());
	uint64_t total_winnings = 0;
	for (size_t rank=0; rank<hands_sorted.size(); rank++) {
		total_winnings += (rank+1) * hands_sorted[rank].bid;
	}
	return total_winnings;
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
	if (part == 2) {
		std::vector<CamelHandv2> hands = parse_handsv2(input);
		// std::sort(hands.begin(), hands.end());
		// dump(hands);
		std::cout << answer(hands) << std::endl;
	}
	else {
		std::vector<CamelHandv1> hands = parse_handsv1(input);
		// std::sort(hands.begin(), hands.end());
		// dump(hands);
		std::cout << answer(hands) << std::endl;
	}
	return 0;
}

/*
--- Day 7: Camel Cards ---
Your all-expenses-paid trip turns out to be a one-way, five-minute ride in an airship. (At least it's a cool airship!) It drops you off at the edge of a vast desert and descends back to Island Island.

"Did you bring the parts?"

You turn around to see an Elf completely covered in white clothing, wearing goggles, and riding a large camel.

"Did you bring the parts?" she asks again, louder this time. You aren't sure what parts she's looking for; you're here to figure out why the sand stopped.

"The parts! For the sand, yes! Come with me; I will show you." She beckons you onto the camel.

After riding a bit across the sands of Desert Island, you can see what look like very large rocks covering half of the horizon. The Elf explains that the rocks are all along the part of Desert Island that is directly above Island Island, making it hard to even get there. Normally, they use big machines to move the rocks and filter the sand, but the machines have broken down because Desert Island recently stopped receiving the parts they need to fix the machines.

You've already assumed it'll be your job to figure out why the parts stopped when she asks if you can help. You agree automatically.

Because the journey will take a few days, she offers to teach you the game of Camel Cards. Camel Cards is sort of similar to poker except it's designed to be easier to play while riding a camel.

In Camel Cards, you get a list of hands, and your goal is to order them based on the strength of each hand. A hand consists of five cards labeled one of A, K, Q, J, T, 9, 8, 7, 6, 5, 4, 3, or 2. The relative strength of each card follows this order, where A is the highest and 2 is the lowest.

Every hand is exactly one type. From strongest to weakest, they are:

Five of a kind, where all five cards have the same label: AAAAA
Four of a kind, where four cards have the same label and one card has a different label: AA8AA
Full house, where three cards have the same label, and the remaining two cards share a different label: 23332
Three of a kind, where three cards have the same label, and the remaining two cards are each different from any other card in the hand: TTT98
Two pair, where two cards share one label, two other cards share a second label, and the remaining card has a third label: 23432
One pair, where two cards share one label, and the other three cards have a different label from the pair and each other: A23A4
High card, where all cards' labels are distinct: 23456
Hands are primarily ordered based on type; for example, every full house is stronger than any three of a kind.

If two hands have the same type, a second ordering rule takes effect. Start by comparing the first card in each hand. If these cards are different, the hand with the stronger first card is considered stronger. If the first card in each hand have the same label, however, then move on to considering the second card in each hand. If they differ, the hand with the higher second card wins; otherwise, continue with the third card in each hand, then the fourth, then the fifth.

So, 33332 and 2AAAA are both four of a kind hands, but 33332 is stronger because its first card is stronger. Similarly, 77888 and 77788 are both a full house, but 77888 is stronger because its third card is stronger (and both hands have the same first and second card).

To play Camel Cards, you are given a list of hands and their corresponding bid (your puzzle input). For example:

32T3K 765
T55J5 684
KK677 28
KTJJT 220
QQQJA 483
This example shows five hands; each hand is followed by its bid amount. Each hand wins an amount equal to its bid multiplied by its rank, where the weakest hand gets rank 1, the second-weakest hand gets rank 2, and so on up to the strongest hand. Because there are five hands in this example, the strongest hand will have rank 5 and its bid will be multiplied by 5.

So, the first step is to put the hands in order of strength:

32T3K is the only one pair and the other hands are all a stronger type, so it gets rank 1.
KK677 and KTJJT are both two pair. Their first cards both have the same label, but the second card of KK677 is stronger (K vs T), so KTJJT gets rank 2 and KK677 gets rank 3.
T55J5 and QQQJA are both three of a kind. QQQJA has a stronger first card, so it gets rank 5 and T55J5 gets rank 4.
Now, you can determine the total winnings of this set of hands by adding up the result of multiplying each hand's bid with its rank (765 * 1 + 220 * 2 + 28 * 3 + 684 * 4 + 483 * 5). So the total winnings in this example are 6440.

Find the rank of every hand in your set. What are the total winnings?

Your puzzle answer was 249748283.

--- Part Two ---
To make things a little more interesting, the Elf introduces one additional rule. Now, J cards are jokers - wildcards that can act like whatever card would make the hand the strongest type possible.

To balance this, J cards are now the weakest individual cards, weaker even than 2. The other cards stay in the same order: A, K, Q, T, 9, 8, 7, 6, 5, 4, 3, 2, J.

J cards can pretend to be whatever card is best for the purpose of determining hand type; for example, QJJQ2 is now considered four of a kind. However, for the purpose of breaking ties between two hands of the same type, J is always treated as J, not the card it's pretending to be: JKKK2 is weaker than QQQQ2 because J is weaker than Q.

Now, the above example goes very differently:

32T3K 765
T55J5 684
KK677 28
KTJJT 220
QQQJA 483
32T3K is still the only one pair; it doesn't contain any jokers, so its strength doesn't increase.
KK677 is now the only two pair, making it the second-weakest hand.
T55J5, KTJJT, and QQQJA are now all four of a kind! T55J5 gets rank 3, QQQJA gets rank 4, and KTJJT gets rank 5.
With the new joker rule, the total winnings in this example are 5905.

Using the new joker rule, find the rank of every hand in your set. What are the new total winnings?

Your puzzle answer was 248029057.

Both parts of this puzzle are complete! They provide two gold stars: **
*/

