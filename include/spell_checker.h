#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

class SpellChecker {
  public:
    void addDictionaryWord(const std::string &word);
    void finalizeDictionary();
    std::string correct(std::string_view word) const;
    // string_view is a non‑owning, cheap view into character data, so it avoids
    // copies and allocations when you only need to read a substring.That makes
    // functions faster andclearer about not taking ownership.

  private:
    struct DictionaryWord {
        std::string original;
        std::string normalized;
    };

    using Index = std::size_t; // use alias for the index type to make it easier
                               // to change later if needed

    std::vector<DictionaryWord> dictionary_;
    std::unordered_map<std::string, Index>
        lookup_; // to make lookup O(1) quickly, we use a hash table to store
                 // the normalized words and their indices in the dictionary

    static std::string normalize(std::string_view word);

    void collectOneEditCandidates(std::string_view normalizedWord,
                                  std::vector<Index> &matches) const;
    void collectTwoEditCandidates(std::string_view normalizedWord,
                                  std::vector<Index> &matches) const;
    void addMatch(std::string_view normalizedCandidate,
                  std::vector<Index> &matches,
                  std::vector<unsigned char> &seen) const;
};
