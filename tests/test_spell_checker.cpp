#include "spell_checker.h"

#include <cassert>
#include <iostream>
#include <string>

int main() {
    // Basic tests for the SpellChecker class. I wrote these plain tests because
    // I am not allowed to use a testing framework according to instructions. In
    // a real project, I would use a testing framework like Google Test or
    // doctest for better test organization and reporting.
    SpellChecker checker;
    // fill dictionary with words that are similar to each other, so we can test
    // the dictionary lookup and the edit distance logic. A range based for loop
    // is used to iterate over the words and add them to the dictionary.
    for (const std::string &word :
         {"rain", "spain", "plain", "plaint", "pain", "main", "mainly", "the",
          "in", "on", "fall", "falls", "his", "was"}) {
        checker.addDictionaryWord(word);
    }
    checker.finalizeDictionary();
    // use assert to check if the correct function returns the expected results
    // for so that debugger would stop at the first failed test. The correct
    // function should return the correct word from the dictionary if it is
    // found, or a string with the input word in curly braces and a question
    // mark if it is not found. The correct function should also handle
    // case-insensitive lookup, but preserve the input when it is already a
    // dictionary word.
    assert(checker.correct("pain") == "pain");
    assert(checker.correct("hte") == "the");
    assert(checker.correct("rame") == "{rame?}");
    assert(checker.correct("fells") == "falls");
    assert(checker.correct("mainy") == "{main mainly}");
    assert(checker.correct("oon") == "on");
    assert(checker.correct("teh") == "the");
    assert(checker.correct("lain") == "plain");
    assert(checker.correct("hints") == "{hints?}");
    assert(checker.correct("pliant") == "plaint");

    // Case-insensitive lookup, but preserve the input when it is already
    // a dictionary word.
    SpellChecker caseChecker;
    caseChecker.addDictionaryWord("Hello");
    caseChecker.finalizeDictionary();
    assert(caseChecker.correct("HELLO") == "HELLO");
    assert(caseChecker.correct("hellp") == "Hello");

    // Two insertions/deletions must not operate on adjacent characters.
    SpellChecker restricted;
    restricted.addDictionaryWord("abcd");
    restricted.finalizeDictionary();
    assert(restricted.correct("ad") ==
           "{ad?}"); // b and c would have to be inserted adjacently.
    assert(restricted.correct("abcd") == "abcd");

    std::cout << "All tests passed.\n";
}
