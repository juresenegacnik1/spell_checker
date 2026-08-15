#include "spell_checker.h"

#include <iostream>
#include <sstream>
#include <string>
#include <string_view>

bool isLetter(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

void removeTrailingCarriageReturn(std::string &line) {
    if (!line.empty() && line.back() == '\r') {
        line.pop_back();
    }
}

void loadDictionary(SpellChecker &checker, std::istream &input) {
    std::string line;
    while (std::getline(input, line)) {
        removeTrailingCarriageReturn(line);
        if (line == "===") {
            break;
        }

        std::istringstream words(line);
        std::string word;
        while (words >> word) {
            checker.addDictionaryWord(word);
        }
    }

    checker.finalizeDictionary();
}

std::string processTextLine(const std::string &line,
                            const SpellChecker &checker) {
    std::string result;
    result.reserve(line.size());

    std::size_t position = 0;
    while (position < line.size()) {
        if (!isLetter(line[position])) {
            // just copy paste letter characters
            result.push_back(line[position]);
            ++position;
            continue;
        }

        const std::size_t begin = position;
        while (position < line.size() && isLetter(line[position])) {
            ++position;
        }

        result += checker.correct(
            std::string_view(line).substr(begin, position - begin));
    }

    return result;
}

int main() {
    SpellChecker checker;
    loadDictionary(checker, std::cin);

    std::string line;
    while (std::getline(std::cin, line)) {
        removeTrailingCarriageReturn(line);
        if (line == "===") { // stop processing text after the second "===" line
            break;
        }

        // process the line and output the corrected version
        std::cout << processTextLine(line, checker) << '\n';
    }

    return 0;
}
