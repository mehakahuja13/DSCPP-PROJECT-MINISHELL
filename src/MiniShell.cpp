#include "MiniShell.h"
#include <iostream>
#include <cctype>

int MiniShell::tokenize(const std::string& line, std::string tokens[], int maxTokens) const {
    int count = 0;
    size_t i = 0, n = line.size();
    while (i < n && count < maxTokens) {
        while (i < n && std::isspace(static_cast<unsigned char>(line[i]))) i++;
        if (i >= n) break;
        size_t j = i;
        while (j < n && !std::isspace(static_cast<unsigned char>(line[j]))) j++;
        tokens[count++] = line.substr(i, j - i);
        i = j;
    }
    return count;
}

// Returns the part of the line after the first 'skipTokens' words (used by write).
std::string MiniShell::restAfter(const std::string& line, int skipTokens) const {
    size_t i = 0, n = line.size();
    for (int t = 0; t < skipTokens; t++) {
        while (i < n && std::isspace(static_cast<unsigned char>(line[i]))) i++;
        while (i < n && !std::isspace(static_cast<unsigned char>(line[i]))) i++;
    }
    while (i < n && std::isspace(static_cast<unsigned char>(line[i]))) i++;
    return line.substr(i);
}

bool MiniShell::confirm(const std::string& question) const {
    std::cout << question << " (y/n): ";
    std::string ans;
    if (!std::getline(std::cin, ans)) return false;
    return !ans.empty() && (ans[0] == 'y' || ans[0] == 'Y');
}

void MiniShell::report(Status s, const std::string& target) const {
    if (s != Status::OK) std::cout << "Error: " << statusMessage(s, target) << "\n";
}

void MiniShell::printHelp() const {
    std::cout <<
        "Available commands:\n"
        "  pwd                   show current directory\n"
        "  ls [path]             list directory contents\n"
        "  cd <path>             change directory (supports /, .., .)\n"
        "  mkdir <name>...       create directories\n"
        "  touch <name>...       create empty files\n"
        "  write <file> <text>   write text into a file (creates it if missing)\n"
        "  cat <file>...         show file contents\n"
        "  rm <name>...          remove files or directories\n"
        "  menu                  show the numbered menu\n"
        "  help                  show this list\n"
        "  exit                  quit MiniShell\n";
}

bool MiniShell::execute(const std::string& rawLine) {
    // A menu number is converted into the equivalent typed command first.
    std::string line = rawLine;
    std::string translated;
    if (menu.toCommand(rawLine, translated)) {
        if (translated.empty()) return true;   // invalid or cancelled choice
        line = translated;
    }

    std::string tok[MAX_TOKENS];
    int n = tokenize(line, tok, MAX_TOKENS);
    if (n == 0) return true;

    const std::string& cmd = tok[0];

    if (cmd == "exit" || cmd == "quit") {
        return false;
    } else if (cmd == "menu") {
        menu.display();
    } else if (cmd == "help") {
        printHelp();
    } else if (cmd == "pwd") {
        std::cout << fs.pwd() << "\n";
    } else if (cmd == "ls") {
        std::string target = (n > 1) ? tok[1] : "";
        report(fs.ls(target), target);
    } else if (cmd == "cd") {
        if (n < 2) { std::cout << "Usage: cd <path>\n"; }
        else report(fs.cd(tok[1]), tok[1]);
    } else if (cmd == "mkdir") {
        if (n < 2) { std::cout << "Usage: mkdir <name>...\n"; }
        for (int i = 1; i < n; i++) report(fs.mkdir(tok[i]), tok[i]);
    } else if (cmd == "touch") {
        if (n < 2) { std::cout << "Usage: touch <name>...\n"; }
        for (int i = 1; i < n; i++) report(fs.touch(tok[i]), tok[i]);
    } else if (cmd == "write") {
        if (n < 3) { std::cout << "Usage: write <file> <text>\n"; }
        else report(fs.writeFile(tok[1], restAfter(line, 2)), tok[1]);
    } else if (cmd == "cat") {
        if (n < 2) { std::cout << "Usage: cat <file>...\n"; }
        for (int i = 1; i < n; i++) {
            std::string out;
            Status s = fs.cat(tok[i], out);
            if (s == Status::OK) std::cout << out << "\n";
            else report(s, tok[i]);
        }
    } else if (cmd == "rm") {
        if (n < 2) { std::cout << "Usage: rm <name>...\n"; }
        for (int i = 1; i < n; i++) {
            Status check = fs.canRemove(tok[i]);
            if (check != Status::OK) { report(check, tok[i]); continue; }
            FSNode* node = fs.resolve(tok[i]);
            if (node != nullptr && node->isDirectory() &&
                !static_cast<Directory*>(node)->isEmpty()) {
                if (!confirm("'" + tok[i] + "' is not empty. Delete it and everything inside?")) {
                    std::cout << "Cancelled.\n";
                    continue;
                }
            }
            report(fs.remove(tok[i]), tok[i]);
        }
    } else {
        std::string suggestion;
        if (corrector.suggest(cmd, suggestion)) {
            if (confirm("Unknown command '" + cmd + "'. Did you mean '" + suggestion + "'?")) {
                // Run the corrected command with the same arguments.
                return execute(suggestion + " " + restAfter(line, 1));
            }
            std::cout << "Cancelled.\n";
        } else {
            std::cout << "Unknown command: " << cmd << " (type 'help')\n";
        }
    }
    return true;
}

void MiniShell::run() {
    std::cout << "=== MiniShell - Virtual Terminal ===\n";
    menu.display();
    std::string line;
    while (true) {
        std::cout << "MiniShell:" << fs.pwd() << "$ ";
        if (!std::getline(std::cin, line)) break;
        if (!execute(line)) break;
    }
    std::cout << "Goodbye!\n";
}
