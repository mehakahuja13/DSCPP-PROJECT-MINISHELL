#ifndef COMMANDCORRECTOR_H
#define COMMANDCORRECTOR_H

#include <string>

// Suggests the closest valid command for a mistyped one.
// Uses Levenshtein (edit) distance with adjacent-swap support, computed with
// DYNAMIC PROGRAMMING.
class CommandCorrector {
private:
    enum { MAX_LEN = 32 };   // longest word we compare; longer input is ignored

    // Number of single-character insertions, deletions, replacements or
    // adjacent swaps needed to turn 'a' into 'b'.
    int distance(const std::string& a, const std::string& b) const;
    std::string toLower(const std::string& s) const;

public:
    // True if 'cmd' is exactly one of the supported commands.
    bool isValid(const std::string& cmd) const;

    // Finds the closest command. Returns true and fills 'best' only if the
    // distance is small enough to be a believable typo.
    bool suggest(const std::string& cmd, std::string& best) const;
};

#endif
