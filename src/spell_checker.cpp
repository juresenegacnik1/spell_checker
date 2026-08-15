#include "spell_checker.h"

#include <algorithm>

constexpr char kFirstLetter = 'a';
// constexpr is better than const because it allows the compiler to perform
// optimizations at compile time, and it can be used in contexts that require
// compile-time constants, such as array sizes or template parameters.
constexpr char kLastLetter = 'z';
constexpr std::size_t kMaxWordLength = 50;

std::string SpellChecker::normalize(std::string_view word) {
    // Function makes characters lowercase to allow case-insensitive matching.
    std::string result(word);
    for (char &c : result) {
        if (c >= 'A' && c <= 'Z') {
            c = static_cast<char>(c - 'A' + 'a');
        }
    }
    return result;
}

void SpellChecker::finalizeDictionary() {
    // Reserve space in the lookup table to avoid rehashing during candidate
    // collection.
    lookup_.reserve(dictionary_.size() * 2 + 1);
}

void SpellChecker::addDictionaryWord(const std::string &word) {
    // The input specification guarantees non-empty words of at most 50 letters.
    // Keeping the check here makes the class safe if it is reused elsewhere.
    if (word.empty() || word.size() > kMaxWordLength) {
        return;
    }

    const std::string normalized = normalize(word);
    // If the normalized word is already in the dictionary, we don't add it
    // again.
    if (lookup_.find(normalized) != lookup_.end()) {
        return;
    }

    const Index index = dictionary_.size();
    dictionary_.push_back(
        {word, normalized}); // add raw and normalized word to the dictionary
    lookup_.emplace(dictionary_.back().normalized,
                    index); // add normalized word to the lookup table with its
                            // index in the dictionary
}

void SpellChecker::addMatch(std::string_view normalizedCandidate,
                            std::vector<Index> &matches,
                            std::vector<unsigned char> &seen) const {
    // If the candidate is in the dictionary and we haven't seen it yet, add it
    // to matches.
    const auto it = lookup_.find(std::string(
        normalizedCandidate)); // find the candidate in the lookup table
    if (it == lookup_.end()) { // if not found, return
        return;
    }

    const Index index =
        it->second;         // get the index of the candidate in the dictionary
    if (seen[index] == 0) { // if we haven't seen this index yet, add it to
                            // matches and mark it as seen
        seen[index] = 1;
        matches.push_back(index);
    }
}

void SpellChecker::collectOneEditCandidates(std::string_view word,
                                            std::vector<Index> &matches) const {
    std::vector<unsigned char> seen(dictionary_.size(), 0);
    // create a vector of unsigned char to keep track of which
    // candidates have been seen, initialized to 0. It has the same size
    // as the dictionary, so we can use the index of the dictionary word
    // as the index in the seen vector.
    std::string candidate;
    candidate.reserve(word.size() + 1);
    // if we instert a letter, the candidate will be one letter longer than the
    // original word, so we reserve space for that.

    // One deletion.
    for (std::size_t i = 0; i < word.size(); ++i) {
        candidate.clear();
        candidate.append(word.substr(0, i));
        candidate.append(word.substr(i + 1));
        addMatch(candidate, matches, seen);
    }

    // One insertion. There are word.size() + 1 insertion gaps.
    for (std::size_t position = 0; position <= word.size(); ++position) {
        for (char c = kFirstLetter; c <= kLastLetter; ++c) {
            candidate.clear();
            candidate.append(word.substr(0, position));
            candidate.push_back(c);
            candidate.append(word.substr(position));
            addMatch(candidate, matches, seen);
        }
    }

    std::sort(matches.begin(), matches.end());
}

void SpellChecker::collectTwoEditCandidates(std::string_view word,
                                            std::vector<Index> &matches) const {
    std::vector<unsigned char> seen(dictionary_.size(), 0);
    std::string candidate;

    // Two deletions. Deleted positions must differ by at least two.
    for (std::size_t first = 0; first < word.size(); ++first) {
        for (std::size_t second = first + 2; second < word.size(); ++second) {
            candidate.clear();
            candidate.reserve(word.size() - 2);
            candidate.append(word.substr(0, first));
            candidate.append(word.substr(first + 1, second - first - 1));
            candidate.append(word.substr(second + 1));
            addMatch(candidate, matches, seen);
        }
    }

    // Two insertions. The two inserted letters must occupy different gaps in
    // the original word; inserting twice into the same gap would make them
    // adjacent characters.
    for (std::size_t first = 0; first < word.size(); ++first) {
        for (std::size_t second = first + 1; second <= word.size(); ++second) {
            for (char firstChar = kFirstLetter; firstChar <= kLastLetter;
                 ++firstChar) { // iterating insertions from a to z
                for (char secondChar = kFirstLetter; secondChar <= kLastLetter;
                     ++secondChar) { // iterating insertions from a to z
                    candidate.clear();
                    candidate.reserve(word.size() + 2);
                    candidate.append(word.substr(0, first));
                    candidate.push_back(firstChar);
                    candidate.append(word.substr(first, second - first));
                    candidate.push_back(secondChar);
                    candidate.append(word.substr(second));
                    addMatch(candidate, matches, seen);
                }
            }
        }
    }

    // One insertion and one deletion. The restriction on adjacent edits only
    // applies when both edits have the same type, so every mixed pair is legal.
    for (std::size_t deletion = 0; deletion < word.size(); ++deletion) {
        std::string afterDeletion(word);
        afterDeletion.erase(deletion, 1);
        // first we make a word with one deletion and then we insert one the
        // same way as in collectOneEditCandidates to get all possible
        // candidates with one deletion and one insertion
        for (std::size_t position = 0; position <= afterDeletion.size();
             ++position) {
            for (char c = kFirstLetter; c <= kLastLetter; ++c) {
                candidate.clear();
                candidate.append(afterDeletion.substr(0, position));
                candidate.push_back(c);
                candidate.append(afterDeletion.substr(position));
                addMatch(candidate, matches, seen);
            }
        }
    }

    std::sort(matches.begin(), matches.end());
}

// corect the word and return the corrected word or a set of possible
// corrections
std::string SpellChecker::correct(std::string_view word) const {
    const std::string normalized = normalize(word);

    // If the normalized word is in the dictionary, return the original word.
    if (lookup_.find(normalized) != lookup_.end()) {
        return std::string(word);
    }

    // Otherwise, collect candidates that are one or two edits away from the
    // normalized word.
    std::vector<Index> matches;
    collectOneEditCandidates(normalized, matches);

    // If no one-edit candidates were found, collect two-edit candidates.
    if (matches.empty()) { // Ignore any corrections that require two edits if
                           // there is at least one that requiresonly one edit;
                           // then . . .
        collectTwoEditCandidates(normalized, matches);
    }

    // If no corrections can be found, print “{W?}”.
    if (matches.empty()) {
        return std::string("{") + std::string(word) + "?}";
    }

    // If exactly one correction is left, print that word.
    if (matches.size() == 1) {
        return dictionary_[matches.front()].original;
    }

    // If more than one possible correction is left, print the set of
    // corrections as { W1 W2 · · · }, in the order they appear in the
    // dictionary
    std::string result = "{";
    for (std::size_t i = 0; i < matches.size(); ++i) {
        if (i != 0) {
            result.push_back(' ');
        }
        result += dictionary_[matches[i]].original;
        // we take the i-th match and get the original word from the dictionary
        // to print it
    }
    result.push_back('}');
    return result;
}
