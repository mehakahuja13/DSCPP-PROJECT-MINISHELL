/* Declares the built-in help pages for MiniShell commands. */

#ifndef MANUAL_H
#define MANUAL_H

#include <string>

class Manual {
public:
    static void printHelp();
    static void printNames();
    static bool printEntry(const std::string& name);
};

#endif
