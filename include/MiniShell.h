#ifndef MINISHELL_H
#define MINISHELL_H

#include <string>
#include "FileSystem.h"
#include "Menu.h"
#include "CommandCorrector.h"
#include "Stack.h"
#include "CommandManager.h"
#include "Commands.h"

// The command-line front end: reads a line, splits it, calls the right module.
class MiniShell {
private:
    FileSystem         fs;
    Menu               menu;
    CommandCorrector   corrector;
    Stack<std::string> history;       // every valid command typed (top = newest)
    CommandManager     commands;      // undo / redo stacks
    Stack<std::string> backStack;     // folders visited before the current one

    static const int MAX_TOKENS = 32;
    typedef const std::string Tok;    // handlers receive the words of the line

    // ---- small helpers ----
    int         tokenize(const std::string& line, std::string tokens[], int maxTokens) const;
    std::string restAfter(const std::string& line, int skipTokens) const;
    std::string askLine(const std::string& prompt) const;
    bool        confirm(const std::string& question) const;
    void        report(Status s, const std::string& target) const;
    bool        approveSave(const PlagiarismResult& r);
    Status      runCommand(Command* cmd, const std::string& target);
    bool        toAbsolute(const std::string& path, std::string& abs) const;
    Status      changeDirectory(const std::string& path);
    bool        historyAt(int number, std::string& out);
    void        showHistory(const std::string& filter);
    bool        rerun(const std::string& bang);
    void        doWrite(const std::string& path, const std::string& text);

    // ---- one handler per command group ----
    void cmdMkdir(Tok tok[], int n);
    void cmdTouch(Tok tok[], int n);
    void cmdWrite(Tok tok[], int n, const std::string& line);
    void cmdAppend(Tok tok[], int n, const std::string& line);
    void cmdCat(Tok tok[], int n);
    void cmdRm(Tok tok[], int n);
    void cmdCp(Tok tok[], int n);
    void cmdMv(Tok tok[], int n);
    void cmdLock(Tok tok[], int n, bool lockIt);
    void cmdEncrypt(Tok tok[], int n, bool encryptIt);
    void cmdBookmark(Tok tok[], int n);
    void cmdGoto(Tok tok[], int n);
    void cmdBack();
    void cmdStats();
    void cmdSave(Tok tok[], int n);
    void cmdLoad(Tok tok[], int n);
    void cmdMan(Tok tok[], int n);

    bool execute(const std::string& rawLine);   // returns false when user exits

public:
    MiniShell();
    void run();
};

#endif
