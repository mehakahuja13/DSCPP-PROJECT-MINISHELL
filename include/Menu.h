#ifndef MENU_H
#define MENU_H

#include <string>

// Numbered menu front end.
// It does NOT execute anything itself: it only turns a menu choice into the
// equivalent command line (e.g. 1 + "docs"  ->  "mkdir docs"), which MiniShell
// then runs through the same handler used for typed commands.
class Menu {
private:
    std::string askLine(const std::string& prompt) const;
    std::string askName(const std::string& prompt) const;   // single word, no spaces
    bool        isNumber(const std::string& s) const;
    std::string trim(const std::string& s) const;

public:
    void display() const;

    // Returns true if 'input' is a menu number.
    // 'commandLine' is filled with the equivalent command, or left empty if the
    // choice was invalid or cancelled (the caller then simply does nothing).
    bool toCommand(const std::string& input, std::string& commandLine) const;
};

#endif
