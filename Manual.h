#ifndef MANUAL_H
#define MANUAL_H

#include <string>

// One table describes every command. 'help' lists them all, 'man <command>'
// shows the details of one, so the two can never disagree.
class Manual {
public:
    static void printHelp();                          // all commands, grouped
    static void printNames();                         // just the names
    static bool printEntry(const std::string& name);  // false if there is no such command
};

#endif
