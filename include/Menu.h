#ifndef MENU_H
#define MENU_H

#include <string>
#include "Trie.h"

// Numbered menu front end. Options are numbered in A-Z order of their names;
// "Exit" is always 0. It also supports prefix search: typing "cr" lists every
// option starting with "cr" so one can be chosen by number.
//
// It does NOT execute anything itself: it only turns a menu choice into the
// equivalent command line (e.g. "Create folder" + "docs" -> "mkdir docs"), which
// MiniShell then runs through the same handler used for typed commands.
class Menu {
private:
    enum { MAX_ITEMS = 64 };

    Trie trie;                  // words of every option -> item index (for prefix search)
    int  order[MAX_ITEMS];      // order[k-1] = table index of menu number k (sorted A-Z)
    int  numberOf[MAX_ITEMS];   // table index -> menu number
    int  count;                 // options with numbers 1..count (Exit is number 0 on top of these)
    int  exitIndex;

    std::string askLine(const std::string& prompt) const;
    std::string askName(const std::string& prompt) const;   // single word, no spaces
    bool        isNumber(const std::string& s) const;
    std::string trim(const std::string& s) const;
    std::string toLower(const std::string& s) const;
    int         collectMatches(const std::string& prefix, bool matched[]) const;
    void        printItem(int tableIndex) const;

public:
    Menu();

    void display() const;

    // How many options have a word starting with 'prefix'.
    int  countMatches(const std::string& prefix) const;

    // Lists the options starting with 'prefix' (A-Z) and lets the user pick one by
    // number. 'commandLine' is filled only if a valid option was chosen.
    void pick(const std::string& prefix, std::string& commandLine) const;

    // Returns true if 'input' is a menu number.
    // 'commandLine' is filled with the equivalent command, or left empty if the
    // choice was invalid or cancelled (the caller then simply does nothing).
    bool toCommand(const std::string& input, std::string& commandLine) const;
};

#endif
