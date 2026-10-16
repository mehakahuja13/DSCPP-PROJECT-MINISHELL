#ifndef MINISHELL_H
#define MINISHELL_H

#include <string>
#include "FileSystem.h"
#include "Menu.h"
#include "CommandCorrector.h"
#include "Stack.h"
#include "CommandManager.h"
#include "Commands.h"

// The command-line front end: reads a line, splits it, calls FileSystem.
class MiniShell {
private:
    FileSystem fs;
    Menu       menu;
    CommandCorrector corrector;
    Stack<std::string> history;     // every valid command typed (top = newest)
    CommandManager     commands;    // undo / redo stacks

    static const int MAX_TOKENS = 32;

    int         tokenize(const std::string& line, std::string tokens[], int maxTokens) const;
    std::string restAfter(const std::string& line, int skipTokens) const;
    bool        confirm(const std::string& question) const;
    void        report(Status s, const std::string& target) const;
    void        runCommand(Command* cmd, const std::string& target);
    void        showHistory();
    void        printHelp() const;
    bool        execute(const std::string& rawLine);   // returns false when user exits

public:
    void run();
};

#endif
