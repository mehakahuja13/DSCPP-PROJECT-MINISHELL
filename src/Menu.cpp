#include "Menu.h"
#include <iostream>
#include <cctype>

void Menu::display() const {
    std::cout <<
        "\n================ MiniShell Menu ================\n"
        "  1. Create folder         6. Delete file/folder\n"
        "  2. Create file           7. List contents\n"
        "  3. Write to file         8. Show current location\n"
        "  4. Change directory      9. Help (command list)\n"
        "  5. Show file content     0. Exit\n"
        "------------------------------------------------\n"
        "Enter a number, or type a command directly.\n"
        "Type 'menu' anytime to see this again.\n"
        "================================================\n\n";
}

std::string Menu::trim(const std::string& s) const {
    size_t a = 0, b = s.size();
    while (a < b && std::isspace(static_cast<unsigned char>(s[a]))) a++;
    while (b > a && std::isspace(static_cast<unsigned char>(s[b - 1]))) b--;
    return s.substr(a, b - a);
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

bool Menu::toCommand(const std::string& input, std::string& commandLine) const {
    commandLine.clear();
    std::string t = trim(input);
    if (!isNumber(t)) return false;

    std::string name, text;

    if (t == "1") {
        name = askName("Enter folder name: ");
        if (!name.empty()) commandLine = "mkdir " + name;
    } else if (t == "2") {
        name = askName("Enter file name: ");
        if (!name.empty()) commandLine = "touch " + name;
    } else if (t == "3") {
        name = askName("Enter file name: ");
        if (name.empty()) return true;
        text = askLine("Enter text to write: ");
        if (text.empty()) { std::cout << "Nothing to write.\n"; return true; }
        commandLine = "write " + name + " " + text;
    } else if (t == "4") {
        name = askName("Enter folder path (.. to go up): ");
        if (!name.empty()) commandLine = "cd " + name;
    } else if (t == "5") {
        name = askName("Enter file name: ");
        if (!name.empty()) commandLine = "cat " + name;
    } else if (t == "6") {
        name = askName("Enter file/folder name to delete: ");
        if (!name.empty()) commandLine = "rm " + name;
    } else if (t == "7") {
        commandLine = "ls";
    } else if (t == "8") {
        commandLine = "pwd";
    } else if (t == "9") {
        commandLine = "help";
    } else if (t == "0") {
        commandLine = "exit";
    } else {
        std::cout << "Invalid menu option. Type 'menu' to see the list.\n";
    }
    return true;
}
