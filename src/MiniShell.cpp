#include "MiniShell.h"
#include "FileTools.h"
#include "Persistence.h"
#include "Manual.h"
#include <iostream>
#include <fstream>
#include <cctype>

MiniShell::MiniShell() {}

// ============================================================ small helpers

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

// Returns the part of the line after the first 'skipTokens' words (used by write/append).
std::string MiniShell::restAfter(const std::string& line, int skipTokens) const {
    size_t i = 0, n = line.size();
    for (int t = 0; t < skipTokens; t++) {
        while (i < n && std::isspace(static_cast<unsigned char>(line[i]))) i++;
        while (i < n && !std::isspace(static_cast<unsigned char>(line[i]))) i++;
    }
    while (i < n && std::isspace(static_cast<unsigned char>(line[i]))) i++;
    return line.substr(i);
}

std::string MiniShell::askLine(const std::string& prompt) const {
    std::cout << prompt;
    std::string ans;
    std::getline(std::cin, ans);
    size_t a = 0, b = ans.size();
    while (a < b && std::isspace(static_cast<unsigned char>(ans[a]))) a++;
    while (b > a && std::isspace(static_cast<unsigned char>(ans[b - 1]))) b--;
    return ans.substr(a, b - a);
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

// Shows the plagiarism result and decides whether the file may be saved.
bool MiniShell::approveSave(const PlagiarismResult& r) {
    if (r.percent < PlagiarismChecker::THRESHOLD) return true;

    std::cout << "\nWARNING: Possible plagiarism detected!\n";
    std::cout << "Similarity: " << r.percent << "%\n";
    std::cout << "Matching file: " << r.matchedPath << "\n";
    if (!confirm("Save this content anyway?")) {
        std::cout << "Save cancelled.\n";
        return false;
    }
    return true;
}

// Hands a command object to the CommandManager and reports the result.
Status MiniShell::runCommand(Command* cmd, const std::string& target) {
    Status s = commands.run(cmd);
    report(s, target);
    return s;
}

// Commands remember ABSOLUTE paths. An existing item gives its own path; a new name
// becomes "parent folder + name".
bool MiniShell::toAbsolute(const std::string& path, std::string& abs) const {
    FSNode* node = fs.resolve(path);
    if (node != nullptr) {
        abs = fs.pathOf(node);
        return true;
    }
    Status s = fs.absolutePath(path, abs);
    if (s != Status::OK) {
        report(s, path);
        return false;
    }
    return true;
}

// cd that remembers where we came from (for 'back').
Status MiniShell::changeDirectory(const std::string& path) {
    std::string before = fs.pwd();
    Status s = fs.cd(path);
    if (s == Status::OK && fs.pwd() != before) backStack.push(before);
    return s;
}

// ============================================================ history

// The stack gives newest first; moving everything to a temporary stack reverses
// the order (oldest on top), and moving it back restores the original.
bool MiniShell::historyAt(int number, std::string& out) {
    Stack<std::string> reversed;
    std::string item;
    while (history.pop(item)) reversed.push(item);

    int current = 1;
    bool found = false;
    while (reversed.pop(item)) {
        if (current == number) { out = item; found = true; }
        current++;
        history.push(item);
    }
    return found;
}

void MiniShell::showHistory(const std::string& filter) {
    if (history.isEmpty()) {
        std::cout << "No commands yet.\n";
        return;
    }
    std::string wanted;
    for (size_t i = 0; i < filter.size(); i++) wanted += static_cast<char>(std::tolower(static_cast<unsigned char>(filter[i])));

    Stack<std::string> reversed;
    std::string item;
    while (history.pop(item)) reversed.push(item);

    int number = 1, shown = 0;
    while (reversed.pop(item)) {
        std::string lower;
        for (size_t i = 0; i < item.size(); i++) lower += static_cast<char>(std::tolower(static_cast<unsigned char>(item[i])));
        if (wanted.empty() || lower.find(wanted) != std::string::npos) {
            std::cout << "  " << number << "  " << item << "\n";
            shown++;
        }
        number++;
        history.push(item);
    }
    if (shown == 0) std::cout << "No history entry contains '" << filter << "'.\n";
}

// "!3" runs history entry 3 again, "!!" runs the last one.
bool MiniShell::rerun(const std::string& bang) {
    int number = 0;
    if (bang == "!!") {
        number = history.size();
    } else {
        std::string digits = bang.substr(1);
        if (digits.empty() || digits.size() > 6) { std::cout << "Usage: !<number>  or  !!\n"; return true; }
        for (size_t i = 0; i < digits.size(); i++) {
            if (!std::isdigit(static_cast<unsigned char>(digits[i]))) { std::cout << "Usage: !<number>  or  !!\n"; return true; }
            number = number * 10 + (digits[i] - '0');
        }
    }

    std::string item;
    if (number < 1 || !historyAt(number, item)) {
        std::cout << "No command number " << number << " in the history.\n";
        return true;
    }
    std::cout << "Running: " << item << "\n";
    return execute(item);
}

// ============================================================ write / append

// Plagiarism check -> confirmation -> undoable write.
void MiniShell::doWrite(const std::string& path, const std::string& text) {
    PlagiarismResult result;
    Status s = fs.checkContent(path, text, result);
    if (s != Status::OK) {
        report(s, path);
        return;
    }
    if (!approveSave(result)) return;

    std::string abs;
    fs.absolutePath(path, abs);   // path was already validated by checkContent
    Status w = commands.run(new WriteCommand(fs, abs, text));
    report(w, path);
    if (w == Status::OK) std::cout << "Saved '" << path << "'.\n";
}

void MiniShell::cmdWrite(Tok tok[], int n, const std::string& line) {
    if (n < 3) { std::cout << "Usage: write <file> <text>\n"; return; }
    doWrite(tok[1], restAfter(line, 2));
}

void MiniShell::cmdAppend(Tok tok[], int n, const std::string& line) {
    if (n < 3) { std::cout << "Usage: append <file> <text>\n"; return; }

    std::string oldText;
    Status s = fs.cat(tok[1], oldText);
    if (s == Status::NOT_FOUND) oldText.clear();          // append creates the file, like write
    else if (s != Status::OK) { report(s, tok[1]); return; }

    std::string add = restAfter(line, 2);
    doWrite(tok[1], oldText.empty() ? add : oldText + "\n" + add);
}

// ============================================================ file commands

void MiniShell::cmdMkdir(Tok tok[], int n) {
    if (n < 2) { std::cout << "Usage: mkdir <name>...\n"; return; }
    for (int i = 1; i < n; i++) {
        std::string abs;
        Status s = fs.absolutePath(tok[i], abs);
        if (s != Status::OK) report(s, tok[i]);
        else runCommand(new MkdirCommand(fs, abs), tok[i]);
    }
}

void MiniShell::cmdTouch(Tok tok[], int n) {
    if (n < 2) { std::cout << "Usage: touch <name>...\n"; return; }
    for (int i = 1; i < n; i++) {
        std::string abs;
        Status s = fs.absolutePath(tok[i], abs);
        if (s != Status::OK) report(s, tok[i]);
        else runCommand(new TouchCommand(fs, abs), tok[i]);
    }
}

void MiniShell::cmdCat(Tok tok[], int n) {
    if (n < 2) { std::cout << "Usage: cat <file>...\n"; return; }
    for (int i = 1; i < n; i++) {
        std::string out;
        Status s = fs.cat(tok[i], out);
        if (s == Status::ENCRYPTED) {   // needs the password; the file itself stays encrypted
            std::string pw = askLine("Password for '" + tok[i] + "': ");
            s = fs.catWithPassword(tok[i], pw, out);
        }
        if (s == Status::OK) std::cout << out << "\n";
        else report(s, tok[i]);
    }
}

void MiniShell::cmdRm(Tok tok[], int n) {
    if (n < 2) { std::cout << "Usage: rm <name>...\n"; return; }
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
        runCommand(new RemoveCommand(fs, fs.pathOf(node)), tok[i]);
    }
}

void MiniShell::cmdCp(Tok tok[], int n) {
    if (n != 3) { std::cout << "Usage: cp <source> <destination>\n"; return; }
    FSNode* node = fs.resolve(tok[1]);
    if (node == nullptr) { report(Status::NOT_FOUND, tok[1]); return; }

    std::string dstAbs;
    if (!toAbsolute(tok[2], dstAbs)) return;
    Status s = runCommand(new CopyCommand(fs, fs.pathOf(node), dstAbs), tok[2]);
    if (s == Status::OK) std::cout << "Copied '" << tok[1] << "' to '" << tok[2] << "'.\n";
}

void MiniShell::cmdMv(Tok tok[], int n) {
    if (n != 3) { std::cout << "Usage: mv <source> <destination>\n"; return; }
    FSNode* node = fs.resolve(tok[1]);
    if (node == nullptr) { report(Status::NOT_FOUND, tok[1]); return; }

    std::string dstAbs;
    if (!toAbsolute(tok[2], dstAbs)) return;
    Status s = runCommand(new MoveCommand(fs, fs.pathOf(node), dstAbs), tok[1]);
    if (s == Status::OK) std::cout << "Moved '" << tok[1] << "' to '" << tok[2] << "'.\n";
}

// ============================================================ security

void MiniShell::cmdLock(Tok tok[], int n, bool lockIt) {
    if (n < 2) { std::cout << "Usage: " << (lockIt ? "lock" : "unlock") << " <name>...\n"; return; }
    for (int i = 1; i < n; i++) {
        FSNode* node = fs.resolve(tok[i]);
        if (node == nullptr) { report(Status::NOT_FOUND, tok[i]); continue; }
        Status s = runCommand(new LockCommand(fs, fs.pathOf(node), lockIt), tok[i]);
        if (s == Status::OK) std::cout << (lockIt ? "Locked '" : "Unlocked '") << tok[i] << "'.\n";
    }
}

void MiniShell::cmdEncrypt(Tok tok[], int n, bool encryptIt) {
    const char* name = encryptIt ? "encrypt" : "decrypt";
    if (n < 2) { std::cout << "Usage: " << name << " <file> [password]\n"; return; }

    FSNode* node = fs.resolve(tok[1]);
    if (node == nullptr) { report(Status::NOT_FOUND, tok[1]); return; }

    std::string password = (n >= 3) ? tok[2] : askLine("Password: ");
    if (password.empty()) { std::cout << "Password cannot be empty.\n"; return; }

    Status s = runCommand(new EncryptCommand(fs, fs.pathOf(node), password, encryptIt), tok[1]);
    if (s == Status::OK) std::cout << (encryptIt ? "Encrypted '" : "Decrypted '") << tok[1] << "'.\n";
}

// ============================================================ navigation extras

void MiniShell::cmdBookmark(Tok tok[], int n) {
    std::string sub = (n >= 2) ? tok[1] : "list";

    if (sub == "add") {
        if (n < 3) { std::cout << "Usage: bookmark add <name> [path]\n"; return; }
        std::string target = (n >= 4) ? tok[3] : ".";
        FSNode* node = fs.resolve(target);
        if (node == nullptr) { report(Status::NOT_FOUND, target); return; }
        if (!node->isDirectory()) { report(Status::NOT_A_DIRECTORY, target); return; }

        bool isNew = fs.getBookmarks().set(tok[2], fs.pathOf(node));
        std::cout << (isNew ? "Bookmark '" : "Bookmark updated: '") << tok[2] << "' -> " << fs.pathOf(node) << "\n";
    } else if (sub == "remove") {
        if (n < 3) { std::cout << "Usage: bookmark remove <name>\n"; return; }
        if (fs.getBookmarks().remove(tok[2])) std::cout << "Removed bookmark '" << tok[2] << "'.\n";
        else std::cout << "No bookmark named '" << tok[2] << "'.\n";
    } else if (sub == "list") {
        int total = fs.getBookmarks().size();
        if (total == 0) { std::cout << "No bookmarks yet. Use: bookmark add <name> [path]\n"; return; }

        std::string* names = new std::string[total];
        std::string* paths = new std::string[total];
        for (int i = 0; i < total; i++) fs.getBookmarks().entryAt(i, names[i], paths[i]);

        for (int i = 1; i < total; i++) {          // insertion sort by name (A-Z)
            std::string nm = names[i], pa = paths[i];
            int j = i - 1;
            while (j >= 0 && names[j] > nm) { names[j + 1] = names[j]; paths[j + 1] = paths[j]; j--; }
            names[j + 1] = nm;
            paths[j + 1] = pa;
        }
        for (int i = 0; i < total; i++) std::cout << "  " << names[i] << "  ->  " << paths[i] << "\n";
        delete[] names;
        delete[] paths;
    } else {
        std::cout << "Usage: bookmark add <name> [path] | bookmark list | bookmark remove <name>\n";
    }
}

void MiniShell::cmdGoto(Tok tok[], int n) {
    if (n < 2) { std::cout << "Usage: goto <bookmark>\n"; return; }
    std::string path;
    if (!fs.getBookmarks().get(tok[1], path)) {
        std::cout << "No bookmark named '" << tok[1] << "'. Use 'bookmark list'.\n";
        return;
    }
    report(changeDirectory(path), path);
}

void MiniShell::cmdBack() {
    std::string target;
    while (backStack.pop(target)) {
        if (target == fs.pwd()) continue;
        if (fs.cd(target) == Status::OK) {
            std::cout << "Back to " << fs.pwd() << "\n";
            return;
        }
        // that folder no longer exists: try the one before it
    }
    std::cout << "No previous folder.\n";
}

// ============================================================ system

void MiniShell::cmdStats() {
    TreeStats st = FileTools::collect(fs, fs.getRoot());
    std::cout << "Files: " << st.files << "   Folders: " << st.dirs << "   Total size: " << st.bytes << " bytes\n";
    std::cout << "Locked: " << st.locked << "   Encrypted: " << st.encrypted
              << "   Bookmarks: " << fs.getBookmarks().size() << "\n";
    std::cout << "Commands typed: " << history.size() << "   Undo available: " << commands.undoCount()
              << "   Redo available: " << commands.redoCount() << "\n";
}

void MiniShell::cmdSave(Tok tok[], int n) {
    std::string filename = (n >= 2) ? tok[1] : "minishell.dat";

    std::ifstream existing(filename.c_str());
    if (existing.good()) {
        existing.close();
        if (!confirm("'" + filename + "' already exists. Overwrite?")) { std::cout << "Cancelled.\n"; return; }
    }

    Status s = Persistence::save(fs, filename);
    if (s == Status::OK) std::cout << "Saved the file system to '" << filename << "'.\n";
    else report(s, filename);
}

void MiniShell::cmdLoad(Tok tok[], int n) {
    std::string filename = (n >= 2) ? tok[1] : "minishell.dat";
    if (!confirm("This replaces everything currently in MiniShell. Load '" + filename + "'?")) {
        std::cout << "Cancelled.\n";
        return;
    }

    Status s = Persistence::load(fs, filename);
    if (s == Status::OK) {
        commands.clearAll();     // old undo steps belong to the old tree
        backStack.clear();
        std::cout << "Loaded '" << filename << "'. You are in " << fs.pwd() << "\n";
    } else {
        report(s, filename);
    }
}

void MiniShell::cmdMan(Tok tok[], int n) {
    if (n < 2) {
        Manual::printNames();
        std::cout << "Type 'man <command>' for details.\n";
        return;
    }
    if (!Manual::printEntry(tok[1])) {
        std::cout << "No manual entry for '" << tok[1] << "'.";
        std::string suggestion;
        if (corrector.suggest(tok[1], suggestion)) std::cout << " Did you mean '" << suggestion << "'?";
        std::cout << "\n";
    }
}

// ============================================================ the command handler

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

    // "!3" / "!!": run a command from the history again.
    if (cmd[0] == '!') return rerun(cmd);

    // Remember valid commands (a mistyped one is stored only after it is corrected).
    if (corrector.isValid(cmd)) history.push(line);

    if (cmd == "exit" || cmd == "quit") {
        return false;
    } else if (cmd == "menu") {
        if (n < 2) {
            menu.display();
        } else {
            // "menu cr" lists the options starting with "cr"; run the one the user picks.
            std::string chosen;
            menu.pick(tok[1], chosen);
            if (!chosen.empty()) return execute(chosen);
        }
    } else if (cmd == "help") {
        Manual::printHelp();
    } else if (cmd == "man") {
        cmdMan(tok, n);
    // ---- basic ----
    } else if (cmd == "pwd") {
        std::cout << fs.pwd() << "\n";
    } else if (cmd == "ls") {
        std::string target = (n > 1) ? tok[1] : "";
        report(fs.ls(target), target);
    } else if (cmd == "cd") {
        if (n < 2) std::cout << "Usage: cd <path>\n";
        else report(changeDirectory(tok[1]), tok[1]);
    } else if (cmd == "mkdir") {
        cmdMkdir(tok, n);
    } else if (cmd == "touch") {
        cmdTouch(tok, n);
    } else if (cmd == "write") {
        cmdWrite(tok, n, line);
    } else if (cmd == "append") {
        cmdAppend(tok, n, line);
    } else if (cmd == "cat") {
        cmdCat(tok, n);
    } else if (cmd == "rm") {
        cmdRm(tok, n);
    } else if (cmd == "cp") {
        cmdCp(tok, n);
    } else if (cmd == "mv") {
        cmdMv(tok, n);
    // ---- navigation extras ----
    } else if (cmd == "tree") {
        std::string target = (n > 1) ? tok[1] : "";
        report(FileTools::tree(fs, target), target);
    } else if (cmd == "back") {
        cmdBack();
    } else if (cmd == "bookmark") {
        cmdBookmark(tok, n);
    } else if (cmd == "goto") {
        cmdGoto(tok, n);
    // ---- search and info ----
    } else if (cmd == "info") {
        std::string target = (n > 1) ? tok[1] : "";
        report(FileTools::info(fs, target), target);
    } else if (cmd == "du") {
        std::string target = (n > 1) ? tok[1] : "";
        report(FileTools::du(fs, target), target);
    } else if (cmd == "wc") {
        if (n < 2) std::cout << "Usage: wc <file>...\n";
        for (int i = 1; i < n; i++) report(FileTools::wc(fs, tok[i]), tok[i]);
    } else if (cmd == "find") {
        if (n < 2) std::cout << "Usage: find <text>\n";
        else FileTools::find(fs, tok[1]);
    } else if (cmd == "grep") {
        if (n < 2) {
            std::cout << "Usage: grep <word> [path]\n";
        } else {
            std::string target = (n > 2) ? tok[2] : "";
            report(FileTools::grep(fs, tok[1], target), target);
        }
    // ---- security ----
    } else if (cmd == "lock") {
        cmdLock(tok, n, true);
    } else if (cmd == "unlock") {
        cmdLock(tok, n, false);
    } else if (cmd == "encrypt") {
        cmdEncrypt(tok, n, true);
    } else if (cmd == "decrypt") {
        cmdEncrypt(tok, n, false);
    // ---- history and system ----
    } else if (cmd == "undo") {
        std::cout << commands.undo() << "\n";
    } else if (cmd == "redo") {
        std::cout << commands.redo() << "\n";
    } else if (cmd == "history") {
        showHistory(n > 1 ? tok[1] : "");
    } else if (cmd == "stats") {
        cmdStats();
    } else if (cmd == "save") {
        cmdSave(tok, n);
    } else if (cmd == "load") {
        cmdLoad(tok, n);
    } else {
        // Typed letters such as "cr": list every menu option starting with them (A-Z).
        if (n == 1 && menu.countMatches(cmd) > 0) {
            std::string chosen;
            menu.pick(cmd, chosen);
            if (!chosen.empty()) return execute(chosen);
            return true;
        }

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
