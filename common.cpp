#include "common.hpp"
#include <chrono>
#include <map>
#include <random>
#include <regex>
#include <unordered_map>

static std::unordered_map<std::string, int> rankMap = {
    {"2",  2 },
    {"3",  3 },
    {"4",  4 },
    {"5",  5 },
    {"6",  6 },
    {"7",  7 },
    {"8",  8 },
    {"9",  9 },
    {"10", 10},
    {"J",  11},
    {"Q",  12},
    {"K",  13},
    {"A",  14}
};

static std::unordered_map<int, std::string> rankMapReverse = {
    {2,  "2" },
    {3,  "3" },
    {4,  "4" },
    {5,  "5" },
    {6,  "6" },
    {7,  "7" },
    {8,  "8" },
    {9,  "9" },
    {10, "10"},
    {11, "J" },
    {12, "Q" },
    {13, "K" },
    {14, "A" }
};

static std::unordered_map<std::string, int> suitMap = {
    {"S", 0},
    {"H", 1},
    {"D", 2},
    {"C", 3}
};

static std::unordered_map<int, std::string> suitMapReverse = {
    {0, "S"},
    {1, "H"},
    {2, "D"},
    {3, "C"}
};

bool isSubstring(const std::string& s1, const std::string& s2) {
    return s1.find(s2) != std::string::npos;
}

std::string createTimestamp() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm now_tm;
    localtime_r(&now_time_t, &now_tm);
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
                            now.time_since_epoch()) %
                        1000;
    std::ostringstream oss;
    oss << std::put_time(&now_tm, "%Y-%m-%dT%H:%M:%S.");
    oss << std::setw(3) << std::setfill('0') << milliseconds.count();
    return oss.str();
}

std::string createLog(const std::string& from,
                      const std::string& to,
                      const std::string& message) {
    return "[" + from + "," + to + "," + createTimestamp() + "] " + message;
}

std::string getKeys(const std::map<std::string, int>& map) {
    std::string keys;
    for (auto const& [key, val] : map) {
        keys += key;
    }
    return keys;
}

bool isRank(char c) {
    return std::isdigit(c) || c == 'J' || c == 'Q' || c == 'K' || c == 'A';
}

bool isSuit(char c) {
    return c == 'S' || c == 'H' || c == 'D' || c == 'C';
}

std::string getPrettyCards(const std::string& cardString, bool sort) {
    std::string result, rank, suit;
    std::vector<std::pair<int, int>> cards;

    for (size_t i = 0; i < cardString.size(); ++i) {
        if (isRank(cardString[i])) {
            rank += cardString[i];
        }
        else {
            suit = cardString[i];
            cards.emplace_back(suitMap[suit], rankMap[rank]);
            rank.clear();
            suit.clear();
        }
    }

    if (sort) {
        std::sort(cards.begin(), cards.end());
    }

    for (const auto& [s, r] : cards) {
        result += rankMapReverse[r] + suitMapReverse[s] + ", ";
    }

    if (!result.empty()) {
        result.pop_back();
        result.pop_back();
    }

    return result;
}

std::string findCardWithSuit(const std::string& cards, char suit) {
    if (!isSuit(suit)) return "";

    std::regex re("(?:10|[2-9JQKA])" + std::string(1, suit));
    std::smatch match;

    if (std::regex_search(cards, match, re)) {
        return match.str();
    }

    return "";
}

char findTrickSuit(const std::string& trick) {
    std::regex re("(?:10|[2-9JQKA])[SHDC]");
    std::smatch match;

    if (std::regex_search(trick, match, re)) {
        return match.str().back();
    }

    return '\0';
}

void deleteCard(std::string& cards, const std::string& card) {
    size_t pos = cards.find(card);
    if (pos != std::string::npos) {
        cards.erase(pos, card.size());
    }
}

std::string getRandomCard(const std::string& cards) {
    std::string card;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distr(0, cards.size() - 1);

    size_t index = distr(gen);

    if (isSuit(cards[index])) index--;
    if (cards[index] == '0') index--;

    if (cards[index] == '1') {
        card = cards.substr(index, 3);
    }
    else {
        card = cards.substr(index, 2);
    }

    return card;
}

bool compareRanks(const std::string& c1, const std::string& c2) {
    int r1 = rankMap[c1.substr(0, c1.size() - 1)];
    int r2 = rankMap[c2.substr(0, c2.size() - 1)];
    return r1 < r2;
}
