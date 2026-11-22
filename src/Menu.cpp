/* Implements the menu shown at startup and translates menu choices into commands. */

#include "Menu.h"
#include <iostream>
#include <cctype>

struct MenuItem {
    const char* label;
    const char* keywords;
    const char* command;
    const char* prompt1;
    const char* prompt2;
    bool        textSecond;
};

static const MenuItem ITEMS[] = {
    { "Create folder",                  "mkdir make new directory",        "mkdir",         "Enter folder name: ", nullptr, false },
    { "Create file",                    "touch make new",                  "touch",         "Enter file name: ", nullptr, false },
    { "Write to file",                  "write edit text nano",            "write",         "Enter file name: ", "Enter text to write: ", true },
    { "Append to file",                 "append add more text",            "append",        "Enter file name: ", "Enter text to add: ", true },
    { "Show file content",              "cat read view open",              "cat",           "Enter file name: ", nullptr, false },
    { "Delete file or folder",          "rm remove",                       "rm",            "Enter file/folder name to delete: ", nullptr, false },
    { "Copy file or folder",            "cp duplicate",                    "cp",            "Enter source name: ", "Enter destination (folder or new name): ", false },
    { "Move or rename",                 "mv rename",                       "mv",            "Enter source name: ", "Enter destination (folder or new name): ", false },
    { "Change directory",               "cd open go folder",               "cd",            "Enter folder path (.. to go up): ", nullptr, false },
    { "List contents",                  "ls dir files",                    "ls",            nullptr, nullptr, false },
    { "Show current location",          "pwd path where",                  "pwd",           nullptr, nullptr, false },
    { "Folder tree",                    "tree structure hierarchy",        "tree",          nullptr, nullptr, false },
    { "Go back to previous folder",     "back previous return",            "back",          nullptr, nullptr, false },
    { "Add bookmark here",              "bookmark save place shortcut",    "bookmark add",  "Enter bookmark name: ", nullptr, false },
    { "List bookmarks",                 "bookmark shortcuts",              "bookmark list", nullptr, nullptr, false },
    { "Go to bookmark",                 "goto jump bookmark",              "goto",          "Enter bookmark name: ", nullptr, false },
    { "File or folder info",            "info details properties",         "info",          "Enter file/folder name: ", nullptr, false },
    { "Folder size",                    "du size space usage",             "du",            nullptr, nullptr, false },
    { "Count lines and words",          "wc characters",                    "wc",            "Enter file name: ", nullptr, false },
    { "Find file by name",              "find search locate",              "find",          "Enter part of the name: ", nullptr, false },
    { "Search text inside files",       "grep find word",                  "grep",          "Enter the word to search: ", nullptr, false },
    { "Compare two files",              "compare similarity plagiarism",   "compare",       "Enter first file: ", "Enter second file: ", false },
    { "Scan all files for plagiarism",  "scan check",               "scan",          nullptr, nullptr, false },
    { "Show differences between files", "diff difference changes",         "diff",          "Enter first file: ", "Enter second file: ", false },
    { "Lock file or folder",            "lock protect readonly",           "lock",          "Enter name to lock: ", nullptr, false },
    { "Unlock file or folder",          "unlock release",                  "unlock",        "Enter name to unlock: ", nullptr, false },
    { "Encrypt file",                   "encrypt password secure",         "encrypt",       "Enter file name: ", "Enter password: ", false },
    { "Decrypt file",                   "decrypt password",                "decrypt",       "Enter file name: ", "Enter password: ", false },
    { "Undo last change",               "undo revert back",                "undo",          nullptr, nullptr, false },
    { "Redo last undone change",        "redo again",                      "redo",          nullptr, nullptr, false },
    { "Command history",                "history past previous",           "history",       nullptr, nullptr, false },
    { "Re-run command by number",       "rerun repeat again history",      "!",             "Enter history number: ", nullptr, false },
    { "Statistics",                     "stats summary",             "stats",         nullptr, nullptr, false },
    { "Save to disk",                   "save store keep",                 "save",          nullptr, nullptr, false },
    { "Load from disk",                 "load restore open",               "load",          nullptr, nullptr, false },
    { "Manual for a command",           "man manual",                      "man",           "Enter command name: ", nullptr, false },
    { "Help (command list)",            "help",                   "help",          nullptr, nullptr, false },
    { "Exit",                           "quit close leave",                "exit",          nullptr, nullptr, false }
};
static const int NUM_ITEMS = sizeof(ITEMS) / sizeof(ITEMS[0]);

Menu::Menu() {
    count = NUM_ITEMS;
    for (int i = 0; i < NUM_ITEMS; i++) order[i] = i;

    // A-Z by label, with Exit always last.
    int exitIndex = NUM_ITEMS - 1;
    for (int i = 0; i < exitIndex; i++) {
        int idx = i;
        int j = i - 1;
        while (j >= 0 && toLower(ITEMS[order[j]].label) > toLower(ITEMS[idx].label)) {
            order[j + 1] = order[j];
            j--;
        }
        order[j + 1] = idx;
    }
    order[exitIndex] = exitIndex;
}

std::string Menu::trim(const std::string& s) const {
    size_t a = 0, b = s.size();
    while (a < b && std::isspace(static_cast<unsigned char>(s[a]))) a++;
    while (b > a && std::isspace(static_cast<unsigned char>(s[b - 1]))) b--;
    return s.substr(a, b - a);
}

std::string Menu::toLower(const std::string& s) const {
    std::string r = s;
    for (size_t i = 0; i < r.size(); i++) {
        r[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(r[i])));
    }
    return r;
}

// Lower-case, trimmed, with repeated spaces collapsed to one.
std::string Menu::normalize(const std::string& s) const {
    std::string t = toLower(trim(s)), r;
    bool space = false;
    for (size_t i = 0; i < t.size(); i++) {
        if (std::isspace(static_cast<unsigned char>(t[i]))) {
            space = true;
        } else {
            if (space && !r.empty()) r += ' ';
            space = false;
            r += t[i];
        }
    }
    return r;
}

// The words a person types to pick an option: its label without any "(...)" note.
std::string Menu::phraseOf(int tableIndex) const {
    std::string label = ITEMS[tableIndex].label;
    size_t p = label.find('(');
    if (p != std::string::npos) label = label.substr(0, p);
    return normalize(label);
}

bool Menu::hasKeyword(int tableIndex, const std::string& word) const {
    std::string kw = std::string(ITEMS[tableIndex].keywords) + " ";
    size_t start = 0;
    while (start < kw.size()) {
        size_t e = kw.find(' ', start);
        if (e == std::string::npos) break;
        if (e > start && kw.compare(start, e - start, word) == 0) return true;
        start = e + 1;
    }
    return false;
}

std::string Menu::askLine(const std::string& prompt) const {
    std::cout << prompt;
    std::string ans;
    std::getline(std::cin, ans);
    return trim(ans);
}

std::string Menu::askName(const std::string& prompt) const {
    std::string ans = askLine(prompt);
    if (ans.find_first_of(" \t") != std::string::npos) {
        std::cout << "Names cannot contain spaces.\n";
        return "";
    }
    return ans;
}

static std::string padLabel(const std::string& s, size_t width) {
    return s.size() >= width ? s : s + std::string(width - s.size(), ' ');
}

void Menu::display() const {
    std::cout << "\n==================== MiniShell Menu (A-Z) ====================\n";

    int rows = (count + 1) / 2;
    for (int r = 0; r < rows; r++) {
        int left  = r;
        int right = r + rows;

        std::cout << padLabel(std::string("  - ") + ITEMS[order[left]].label, 36);
        if (right < count) std::cout << "  - " << ITEMS[order[right]].label;
        std::cout << "\n";
    }
    std::cout << "--------------------------------------------------------------\n"
        "Just type what you want, like: change directory\n"
        "You can add the name too: create file notes.txt\n"
        "Type a few letters (like cr) to see matching options.\n"
        "Type 'menu' to see this list again, 'help' for all commands.\n"
        "==============================================================\n\n";
}

// An option matches if its name starts with what was typed, or if the typed word is one of its keywords.
int Menu::collectMatches(const std::string& typed, bool matched[]) const {
    for (int i = 0; i < NUM_ITEMS; i++) matched[i] = false;

    std::string t = normalize(typed);
    if (t.empty()) return 0;

    int matches = 0;
    for (int i = 0; i < NUM_ITEMS; i++) {
        std::string phrase = phraseOf(i);
        bool ok = phrase.size() >= t.size() && phrase.compare(0, t.size(), t) == 0;
        if (!ok && t.find(' ') == std::string::npos && t.size() >= 3) ok = hasKeyword(i, t);
        if (ok) {
            matched[i] = true;
            matches++;
        }
    }
    return matches;
}

int Menu::countMatches(const std::string& typed) const {
    if (typed.empty()) return 0;
    bool matched[MAX_ITEMS];
    return collectMatches(typed, matched);
}

void Menu::pick(const std::string& typed, std::string& commandLine) const {
    commandLine.clear();
    std::string current = typed;

    for (int attempt = 0; attempt < 4; attempt++) {
        bool matched[MAX_ITEMS];
        int n = collectMatches(current, matched);
        if (n == 0) {
            std::cout << "I couldn't find an option for '" << current << "'. Type 'menu' to see all options.\n";
            return;
        }

        int only = -1;
        for (int k = 0; k < count; k++) if (matched[order[k]]) only = order[k];

        if (n == 1) {
            build(only, "", commandLine);
            return;
        }

        std::cout << "Options matching '" << current << "' (A-Z):\n";
        for (int k = 0; k < count; k++) {
            if (matched[order[k]]) std::cout << "  - " << ITEMS[order[k]].label << "\n";
        }

        std::string ans = askLine("Type the name of the one you want (or press Enter to cancel): ");
        if (ans.empty()) {
            std::cout << "Cancelled.\n";
            return;
        }
        if (toCommand(ans, commandLine)) return;
        current = ans;
    }
    std::cout << "Cancelled.\n";
}

// Turns a chosen option (plus any words typed after its name) into a real command line.
bool Menu::build(int tableIndex, const std::string& rest, std::string& commandLine) const {
    commandLine.clear();
    const MenuItem& item = ITEMS[tableIndex];

    std::string a1, a2;
    std::string r = trim(rest);
    if (!r.empty()) {
        size_t sp = r.find_first_of(" \t");
        a1 = (sp == std::string::npos) ? r : r.substr(0, sp);
        if (sp != std::string::npos) a2 = trim(r.substr(sp));
    }

    if (item.prompt1 != nullptr && a1.empty()) {
        a1 = askName(item.prompt1);
        if (a1.empty()) return true;
    }
    if (item.prompt2 != nullptr) {
        if (a2.empty()) {
            a2 = item.textSecond ? askLine(item.prompt2) : askName(item.prompt2);
            if (a2.empty()) {
                if (item.textSecond) std::cout << "Nothing entered.\n";
                return true;
            }
        } else if (!item.textSecond && a2.find_first_of(" \t") != std::string::npos) {
            std::cout << "Names cannot contain spaces.\n";
            return true;
        }
    } else {
        a2.clear();
    }

    std::string cmd = item.command;
    if (cmd == "!") {
        commandLine = "!" + a1;
    } else {
        commandLine = cmd;
        if (!a1.empty()) commandLine += " " + a1;
        if (!a2.empty()) commandLine += " " + a2;
    }
    return true;
}

// If the input starts with an option's full name (any letter case), run that option.
bool Menu::toCommand(const std::string& input, std::string& commandLine) const {
    commandLine.clear();
    std::string t = trim(input);
    std::string low = normalize(t);
    if (low.empty()) return false;

    int best = -1;
    size_t bestLen = 0;
    for (int i = 0; i < NUM_ITEMS; i++) {
        std::string phrase = phraseOf(i);
        if (low.size() < phrase.size() || low.compare(0, phrase.size(), phrase) != 0) continue;
        if (low.size() > phrase.size() && low[phrase.size()] != ' ') continue;
        if (phrase.size() > bestLen) { best = i; bestLen = phrase.size(); }
    }
    if (best < 0) return false;

    // Everything after the option's name (keeps the original letter case of file names).
    std::string rest;
    size_t words = 0;
    for (size_t i = 0; i < bestLen; i++) if (low[i] == ' ') words++;
    size_t pos = 0;
    for (size_t w = 0; w <= words; w++) {
        while (pos < t.size() && std::isspace(static_cast<unsigned char>(t[pos]))) pos++;
        while (pos < t.size() && !std::isspace(static_cast<unsigned char>(t[pos]))) pos++;
    }
    rest = t.substr(pos);

    return build(best, rest, commandLine);
}

// For input like "read notes.txt": the first word is a keyword of exactly one option.
bool Menu::byWord(const std::string& input, std::string& commandLine) const {
    commandLine.clear();
    std::string t = trim(input);
    size_t sp = t.find_first_of(" \t");
    if (sp == std::string::npos) return false;

    std::string word = toLower(t.substr(0, sp));
    int found = -1, hits = 0;
    for (int i = 0; i < NUM_ITEMS; i++) {
        if (hasKeyword(i, word)) { found = i; hits++; }
    }
    if (hits != 1) return false;
    return build(found, t.substr(sp), commandLine);
}

// Plural "s" removed, so "files" and "file", "differences" and "difference" compare equal.
std::string Menu::stem(const std::string& w) const {
    std::string s = toLower(w);
    if (s.size() > 3 && s.compare(s.size() - 2, 2, "es") == 0 && s[s.size() - 3] != 'c') s.erase(s.size() - 2);
    else if (s.size() > 3 && s[s.size() - 1] == 's') s.erase(s.size() - 1);
    return s;
}

// Small words people add in normal sentences; they never decide which option is meant.
bool Menu::isFiller(const std::string& w) const {
    static const char* const FILLER[] = {
        "the", "a", "an", "of", "to", "between", "two", "me", "my", "please",
        "in", "for", "and", "from", "all", "with", "this", "that", "on"
    };
    std::string l = toLower(w);
    for (size_t i = 0; i < sizeof(FILLER) / sizeof(FILLER[0]); i++) {
        if (l == FILLER[i]) return true;
    }
    return false;
}

// True if the word is part of the option's name or one of its keywords.
bool Menu::inVocab(int tableIndex, const std::string& stemmed) const {
    std::string text = phraseOf(tableIndex) + " " + ITEMS[tableIndex].keywords + " ";
    size_t start = 0;
    while (start < text.size()) {
        size_t e = text.find(' ', start);
        if (e == std::string::npos) break;
        if (e > start && stem(text.substr(start, e - start)) == stemmed) return true;
        start = e + 1;
    }
    return false;
}

// Understands loosely worded input such as "show difference between files".
// Leading words must all belong to one option; whatever follows is used as its arguments.
bool Menu::fuzzy(const std::string& input, std::string& commandLine) const {
    commandLine.clear();
    std::string t = trim(input);
    if (t.empty()) return false;

    std::string words[32];
    int n = 0;
    size_t pos = 0;
    while (pos < t.size() && n < 32) {
        while (pos < t.size() && std::isspace(static_cast<unsigned char>(t[pos]))) pos++;
        size_t b = pos;
        while (pos < t.size() && !std::isspace(static_cast<unsigned char>(t[pos]))) pos++;
        if (pos > b) words[n++] = t.substr(b, pos - b);
    }

    int best = -1, bestRun = 0, bestUsed = 0;
    bool tie = false;
    for (int i = 0; i < NUM_ITEMS; i++) {
        int used = 0, real = 0;
        while (used < n) {
            if (isFiller(words[used])) { used++; continue; }
            if (!inVocab(i, stem(words[used]))) break;
            real++;
            used++;
        }
        if (real < 2) continue;
        if (real > bestRun) { best = i; bestRun = real; bestUsed = used; tie = false; }
        else if (real == bestRun) tie = true;
    }
    if (best < 0 || tie) return false;

    std::string rest;
    for (int k = bestUsed; k < n; k++) {
        if (!ITEMS[best].textSecond && isFiller(words[k])) continue;   // "a.txt and b.txt" -> "a.txt b.txt"
        rest += (rest.empty() ? "" : " ") + words[k];
    }
    return build(best, rest, commandLine);
}
