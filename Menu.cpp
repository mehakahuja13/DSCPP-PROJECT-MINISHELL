#include "Menu.h"
#include <iostream>
#include <cctype>

// One row per menu option. 'keywords' are extra words that also find the option
// (command names and synonyms). The last row (Exit) is menu number 0; all other
// rows are sorted A-Z by 'label' when the menu is created, so new rows can be
// added anywhere in this table.
//   prompt1 / prompt2 : questions to ask (nullptr = not needed)
//   textSecond        : the second answer may contain spaces (write, append)
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
    { "Count lines and words",          "wc characters",             "wc",            "Enter file name: ", nullptr, false },
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

// Splits 'text' on spaces and inserts every word into the trie.
static void insertWords(Trie& trie, const std::string& text, int id) {
    std::string word;
    for (size_t i = 0; i <= text.size(); i++) {
        if (i == text.size() || text[i] == ' ' || text[i] == '/' || text[i] == '(' || text[i] == ')') {
            if (!word.empty()) trie.insert(word, id);
            word.clear();
        } else {
            word += static_cast<char>(std::tolower(static_cast<unsigned char>(text[i])));
        }
    }
}

Menu::Menu() {
    exitIndex = NUM_ITEMS - 1;
    count = NUM_ITEMS - 1;

    // Sort the numbered options A-Z by label (insertion sort on the index list).
    for (int i = 0; i < count; i++) {
        int idx = i;
        int j = i - 1;
        while (j >= 0 && toLower(ITEMS[order[j]].label) > toLower(ITEMS[idx].label)) {
            order[j + 1] = order[j];
            j--;
        }
        order[j + 1] = idx;
    }
    for (int k = 0; k < count; k++) numberOf[order[k]] = k + 1;
    numberOf[exitIndex] = 0;

    for (int i = 0; i < NUM_ITEMS; i++) {
        insertWords(trie, ITEMS[i].label, i);
        insertWords(trie, ITEMS[i].keywords, i);
    }
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

bool Menu::isNumber(const std::string& s) const {
    if (s.empty()) return false;
    for (size_t i = 0; i < s.size(); i++) {
        if (!std::isdigit(static_cast<unsigned char>(s[i]))) return false;
    }
    return true;
}

std::string Menu::askLine(const std::string& prompt) const {
    std::cout << prompt;
    std::string ans;
    std::getline(std::cin, ans);
    return trim(ans);
}

// Names are used as single command words, so spaces are not allowed.
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

    int rows = (count + 1) / 2;   // two columns, reading down: 1..rows | rows+1..count
    for (int r = 0; r < rows; r++) {
        int left  = r + 1;
        int right = r + 1 + rows;

        std::string l = (left < 10 ? "  " : " ") + std::to_string(left) + ". " + ITEMS[order[left - 1]].label;
        std::cout << padLabel(l, 36);
        if (right <= count) {
            std::cout << (right < 10 ? "  " : " ") << right << ". " << ITEMS[order[right - 1]].label;
        }
        std::cout << "\n";
    }
    std::cout << "  0. Exit\n"
        "--------------------------------------------------------------\n"
        "Enter a number, or type a command directly.\n"
        "Type a few letters (like cr) to list matching options and pick one.\n"
        "Type 'menu' to see this list again, 'help' for all commands.\n"
        "==============================================================\n\n";
}

// Marks every option having a word that starts with 'prefix'; returns how many.
int Menu::collectMatches(const std::string& prefix, bool matched[]) const {
    for (int i = 0; i < NUM_ITEMS; i++) matched[i] = false;
    int n = 0;
    for (const Trie::IdNode* node = trie.find(toLower(prefix)); node != nullptr; node = node->next) {
        matched[node->id] = true;
        n++;
    }
    return n;
}

int Menu::countMatches(const std::string& prefix) const {
    if (prefix.empty()) return 0;
    bool matched[MAX_ITEMS];
    return collectMatches(prefix, matched);
}

void Menu::printItem(int tableIndex) const {
    int num = numberOf[tableIndex];
    std::cout << "  " << (num < 10 ? " " : "") << num << ". " << ITEMS[tableIndex].label << "\n";
}

void Menu::pick(const std::string& prefix, std::string& commandLine) const {
    commandLine.clear();

    bool matched[MAX_ITEMS];
    int n = collectMatches(prefix, matched);
    if (n == 0) {
        std::cout << "No menu option starts with '" << prefix << "'. Type 'menu' to see all options.\n";
        return;
    }

    std::cout << "Menu options starting with '" << prefix << "' (A-Z):\n";
    for (int k = 0; k < count; k++) {          // numbers 1..count are already in A-Z order
        if (matched[order[k]]) printItem(order[k]);
    }
    if (matched[exitIndex]) printItem(exitIndex);

    std::string ans = askLine("Type the number to select it (or press Enter to cancel): ");
    if (ans.empty()) {
        std::cout << "Cancelled.\n";
        return;
    }
    if (!toCommand(ans, commandLine)) std::cout << "Please type a number from the list.\n";
}

bool Menu::toCommand(const std::string& input, std::string& commandLine) const {
    commandLine.clear();
    std::string t = trim(input);
    if (!isNumber(t)) return false;

    if (t.size() > 3) {
        std::cout << "Invalid menu option. Type 'menu' to see the list.\n";
        return true;
    }
    int number = 0;
    for (size_t i = 0; i < t.size(); i++) number = number * 10 + (t[i] - '0');

    int idx;
    if (number == 0) idx = exitIndex;
    else if (number >= 1 && number <= count) idx = order[number - 1];
    else {
        std::cout << "Invalid menu option. Type 'menu' to see the list.\n";
        return true;
    }

    const MenuItem& item = ITEMS[idx];
    std::string a1, a2;
    if (item.prompt1 != nullptr) {
        a1 = askName(item.prompt1);
        if (a1.empty()) return true;           // cancelled or invalid
    }
    if (item.prompt2 != nullptr) {
        a2 = item.textSecond ? askLine(item.prompt2) : askName(item.prompt2);
        if (a2.empty()) {
            if (item.textSecond) std::cout << "Nothing entered.\n";
            return true;
        }
    }

    std::string cmd = item.command;
    if (cmd == "!") {
        commandLine = "!" + a1;                // "!3" = run history entry 3
    } else {
        commandLine = cmd;
        if (!a1.empty()) commandLine += " " + a1;
        if (!a2.empty()) commandLine += " " + a2;
    }
    return true;
}
