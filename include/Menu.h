/* Declares the interactive menu: options are chosen by typing their names, never by number. */

#ifndef MENU_H
#define MENU_H

#include <string>

class Menu {
private:
    enum { MAX_ITEMS = 64 };

    int  order[MAX_ITEMS];   // table indexes sorted A-Z by label
    int  count;              // number of options shown (Exit is last)

    std::string askLine(const std::string& prompt) const;
    std::string askName(const std::string& prompt) const;
    std::string trim(const std::string& s) const;
    std::string toLower(const std::string& s) const;
    std::string normalize(const std::string& s) const;
    std::string phraseOf(int tableIndex) const;
    bool        hasKeyword(int tableIndex, const std::string& word) const;
    int         collectMatches(const std::string& typed, bool matched[]) const;
    std::string stem(const std::string& w) const;
    bool        isFiller(const std::string& w) const;
    bool        inVocab(int tableIndex, const std::string& stemmed) const;
    bool        build(int tableIndex, const std::string& rest, std::string& commandLine) const;

public:
    Menu();

    void display() const;

    int  countMatches(const std::string& typed) const;

    void pick(const std::string& typed, std::string& commandLine) const;

    bool byWord(const std::string& input, std::string& commandLine) const;

    bool fuzzy(const std::string& input, std::string& commandLine) const;

    bool toCommand(const std::string& input, std::string& commandLine) const;
};

#endif
