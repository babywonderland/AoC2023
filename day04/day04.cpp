// c++ -Wall -Wextra -pedantic -O3 -std=c++20 day04.cpp -o day04 && time ./day04 day04in.txt 2
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
	std::list<LottoCard> children;
	std::string id;

	LottoCard() { }
	explicit LottoCard(const std::string&, const std::string& parent_id="");
	LottoCard(const LottoCard&) = default;
	LottoCard(LottoCard&&) = default;

	std::vector<int> held_winners() const;
	uint64_t value() const;
	std::string str(const std::string& prefix="") const;
	size_t size() const;
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

LottoCard::LottoCard(const std::string& input, const std::string& parent_id) {
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
	for (const LottoCard& child : children) {
		std::string cpfx = prefix + "  ";
		out << "\n" << child.str(cpfx);
	}
	return out.str();
}

size_t LottoCard::size() const {
	size_t sz = 1;
	for (const LottoCard& child : children) {
		sz += child.size();
	}
	return sz;
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
		const size_t child_count = me.held_winners().size();
		if (cidx + child_count >= results.size()) {
			std::cerr << "Error: missing some of " << child_count
				<< " child(ren) from input " << me.str() << std::endl;
			return results;
		}
		for (size_t cc=0; cc<child_count; cc++) {
			me.children.emplace_front(results[cidx+cc+1]);
			auto update_children_ids = [](LottoCard& kid, const std::string& pid) -> void {
				auto uci_impl = [](auto& self, LottoCard& kid, const std::string& pid) -> void {
					kid.id = pid + "." + kid.id;
					for (LottoCard& gkid : kid.children) {
						self(self, gkid, pid);
					}
				};
				return uci_impl(uci_impl, kid, pid);
			};
			update_children_ids(me.children.front(), me.id);
		}
		me.children.reverse();
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
	#if 1
	std::vector<std::tuple<LottoCard, std::vector<int>, uint64_t>> card_values;
	std::transform(cards.begin(), cards.end(), std::back_inserter(card_values),
		[](const LottoCard& cc) -> std::tuple<LottoCard, std::vector<int>, uint64_t> {
			return {cc, cc.held_winners(), cc.value()};
		});
	dump(card_values);
	#endif
	return std::transform_reduce(cards.begin(), cards.end(), 0U, std::plus<>(),
		[](const LottoCard& cc) { return cc.value(); });
}

int part2_answer(const std::vector<LottoCard>& cards) {
	dump(cards);
	std::vector<size_t> sizes;
	std::transform(cards.begin(), cards.end(), std::back_inserter(sizes),
		[](const LottoCard& cc) -> size_t { return cc.size(); });
	dump(sizes);
	return std::reduce(sizes.begin(), sizes.end());
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


