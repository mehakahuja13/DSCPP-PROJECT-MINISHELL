
#include "MiniShell.h"
#include "PlagiarismChecker.h"
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
    if (s != Status::OK)
        std::cout << "Error: " << statusMessage(s, target) << "\n";
}

void MiniShell::runCommand(Command* cmd, const std::string& target) {
    report(commands.run(cmd), target);
}

// Prints command history from oldest to newest.
void MiniShell::showHistory() {
    if (history.isEmpty()) {
        std::cout << "No commands yet.\n";
        return;
    }

    Stack<std::string> reversed;
    std::string item;

    while (history.pop(item))
        reversed.push(item);

    int number = 1;

    while (reversed.pop(item)) {
        std::cout << "  " << number++ << "  " << item << "\n";
        history.push(item);
    }
}

void MiniShell::printHelp() const {
    std::cout <<
        "Available commands:\n"
        "  pwd                   show current directory\n"
        "  ls [path]             list directory contents\n"
        "  cd <path>             change directory (supports /, .., .)\n"
        "  mkdir <name>...       create directories\n"
        "  touch <name>...       create empty files\n"
        "  write <file> <text>   write text into a file\n"
        "  cat <file>...         show file contents\n"
        "  rm <name>...          remove files or directories\n"
        "  undo                  undo the last mkdir / touch / write / rm\n"
        "  redo                  redo what was undone\n"
        "  history               list commands typed so far\n"
        "  menu                  show the numbered menu\n"
        "  help                  show this list\n"
        "  exit                  quit MiniShell\n";
}

bool MiniShell::execute(const std::string& rawLine) {
    std::string line = rawLine;
    std::string translated;

    if (menu.toCommand(rawLine, translated)) {
        if (translated.empty()) return true;
        line = translated;
    }

    std::string tok[MAX_TOKENS];
    int n = tokenize(line, tok, MAX_TOKENS);

    if (n == 0) return true;

    const std::string& cmd = tok[0];

    // Command History
    if (corrector.isValid(cmd))
        history.push(line);

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
        if (n < 2)
            std::cout << "Usage: cd <path>\n";
        else
            report(fs.cd(tok[1]), tok[1]);

    } else if (cmd == "mkdir") {
        if (n < 2)
            std::cout << "Usage: mkdir <name>...\n";

        for (int i = 1; i < n; i++) {
            std::string abs;
            Status s = fs.absolutePath(tok[i], abs);

            if (s != Status::OK)
                report(s, tok[i]);
            else
                runCommand(new MkdirCommand(fs, abs), tok[i]);
        }

    } else if (cmd == "touch") {
        if (n < 2)
            std::cout << "Usage: touch <name>...\n";

        for (int i = 1; i < n; i++) {
            std::string abs;
            Status s = fs.absolutePath(tok[i], abs);

            if (s != Status::OK)
                report(s, tok[i]);
            else
                runCommand(new TouchCommand(fs, abs), tok[i]);
        }

    } else if (cmd == "write") {
        if (n < 3) {
            std::cout << "Usage: write <file> <text>\n";
        } else {
            std::string text = restAfter(line, 2);
            std::string abs;

            Status s = fs.absolutePath(tok[1], abs);

            if (s != Status::OK) {
                report(s, tok[1]);
            } else {
                PlagiarismResult result;
                Status check = fs.checkContent(abs, text, result);

                if (check != Status::OK) {
                    report(check, tok[1]);
                } else {
                    bool saveAllowed = true;

                    if (result.percent >= PlagiarismChecker::THRESHOLD) {
                        std::cout << "\nWARNING: Possible plagiarism detected!\n";
                        std::cout << "Similarity: " << result.percent << "%\n";
                        std::cout << "Matching file: "
                                  << result.matchedPath << "\n";

                        saveAllowed = confirm("Save this content anyway?");
                    }

                    if (!saveAllowed) {
                        std::cout << "Save cancelled.\n";
                    } else {
                        Status w = commands.run(
                            new WriteCommand(fs, abs, text)
                        );

                        report(w, tok[1]);

                        if (w == Status::OK)
                            std::cout << "Saved '" << tok[1] << "'.\n";
                    }
                }
            }
        }

    } else if (cmd == "undo") {
        std::cout << commands.undo() << "\n";

    } else if (cmd == "redo") {
        std::cout << commands.redo() << "\n";

    } else if (cmd == "history") {
        showHistory();

    } else if (cmd == "cat") {
        if (n < 2)
            std::cout << "Usage: cat <file>...\n";

        for (int i = 1; i < n; i++) {
            std::string out;
            Status s = fs.cat(tok[i], out);

            if (s == Status::OK)
                std::cout << out << "\n";
            else
                report(s, tok[i]);
        }

    } else if (cmd == "rm") {
        if (n < 2)
            std::cout << "Usage: rm <name>...\n";

        for (int i = 1; i < n; i++) {
            Status check = fs.canRemove(tok[i]);

            if (check != Status::OK) {
                report(check, tok[i]);
                continue;
            }

            FSNode* node = fs.resolve(tok[i]);

            if (node != nullptr && node->isDirectory()) {
                const bool isEmpty =
                    static_cast<Directory*>(node)->isEmpty();

                const std::string question = isEmpty
                    ? "Delete empty folder '" + tok[i] + "'?"
                    : "'" + tok[i] +
                      "' is not empty. Delete it and everything inside?";

                if (!confirm(question)) {
                    std::cout << "Cancelled.\n";
                    continue;
                }
            }

            runCommand(new RemoveCommand(fs, fs.pathOf(node)), tok[i]);
        }

    } else {
        std::string suggestion;

        if (corrector.suggest(cmd, suggestion)) {
            if (confirm(
                    "Unknown command '" + cmd +
                    "'. Did you mean '" + suggestion + "'?")) {
                return execute(suggestion + " " + restAfter(line, 1));
            }

            std::cout << "Cancelled.\n";
        } else {
            std::cout << "Unknown command: " << cmd
                      << " (type 'help')\n";
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

        if (!std::getline(std::cin, line))
            break;

        if (!execute(line))
            break;
    }

    std::cout << "Goodbye!\n";
}
