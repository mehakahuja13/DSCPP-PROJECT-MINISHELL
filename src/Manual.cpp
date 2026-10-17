#include "Manual.h"
#include <iostream>

namespace {

struct Entry {
    const char* section;
    const char* name;
    const char* usage;
    const char* summary;
    const char* details;
};

const Entry ENTRIES[] = {
    { "FILES AND FOLDERS", "mkdir", "mkdir <name>...", "Create folders",
      "Creates empty folders. A path like docs/notes works if docs exists. Can be undone." },
    { "FILES AND FOLDERS", "touch", "touch <name>...", "Create empty files",
      "Does nothing if the file already exists. Can be undone." },
    { "FILES AND FOLDERS", "write", "write <file> <text>", "Write text into a file",
      "Creates the file if needed and replaces its content. The text is checked for plagiarism first; at 50% or more you are asked before saving. Locked or encrypted files cannot be written." },
    { "FILES AND FOLDERS", "append", "append <file> <text>", "Add a new line to a file",
      "Adds the text as a new last line and keeps the old content. The whole new content is checked for plagiarism." },
    { "FILES AND FOLDERS", "cat", "cat <file>...", "Show file content",
      "An encrypted file asks for its password first (the file stays encrypted)." },
    { "FILES AND FOLDERS", "rm", "rm <name>...", "Delete files or folders",
      "Asks before deleting a folder that is not empty. The root, your current folder and locked items cannot be deleted. 'undo' brings deleted items back." },
    { "FILES AND FOLDERS", "cp", "cp <source> <destination>", "Copy a file or folder",
      "If the destination is an existing folder the copy goes inside it, otherwise the destination is the new name. Folders are copied with everything inside. Can be undone." },
    { "FILES AND FOLDERS", "mv", "mv <source> <destination>", "Move or rename",
      "Same rules as cp for the destination. Locked items cannot be moved. Can be undone." },

    { "NAVIGATION", "cd", "cd <path>", "Change directory",
      "Use / for the root, .. for the parent folder and . for the current one." },
    { "NAVIGATION", "ls", "ls [path]", "List contents",
      "[D] marks folders and [F] files. [locked] and [enc] mark protected items." },
    { "NAVIGATION", "pwd", "pwd", "Show the current location", "Prints the full path of the current folder." },
    { "NAVIGATION", "tree", "tree [path]", "Show the folder tree", "Draws everything below a folder." },
    { "NAVIGATION", "back", "back", "Return to the previous folder",
      "Goes back through the folders you visited with cd or goto." },
    { "NAVIGATION", "bookmark", "bookmark add|list|remove ...", "Folder shortcuts",
      "bookmark add <name> [path] gives a short name to a folder (default: the current one). bookmark list shows them, bookmark remove <name> deletes one. Use 'goto <name>' to jump there. 'save' keeps bookmarks." },
    { "NAVIGATION", "goto", "goto <bookmark>", "Jump to a bookmarked folder", "See 'bookmark'." },

    { "SEARCH AND INFO", "info", "info [path]", "Show details of a file or folder",
      "Name, path, type, size, lines and words, lock and encryption state." },
    { "SEARCH AND INFO", "du", "du [path]", "Show the size of a folder or file",
      "Total bytes, with the number of files and folders inside." },
    { "SEARCH AND INFO", "wc", "wc <file>...", "Count lines, words and characters", "Encrypted files cannot be counted." },
    { "SEARCH AND INFO", "find", "find <text>", "Find files and folders by name",
      "Searches the whole file system for names containing the text (letter case is ignored)." },
    { "SEARCH AND INFO", "grep", "grep <word> [path]", "Search for a word inside files",
      "Prints every line containing the word with its file name and line number. Encrypted files are skipped." },

    { "PLAGIARISM", "compare", "compare <file1> <file2>", "Compare two files",
      "Shows how much of each file (in 3-word groups) also appears in the other one." },
    { "PLAGIARISM", "scan", "scan", "Check all files against each other",
      "Lists every pair of readable files with 50% or more similarity." },
    { "PLAGIARISM", "diff", "diff <file1> <file2>", "Show the line differences",
      "Lines starting with - exist only in the first file, lines starting with + only in the second (longest common subsequence)." },
    { "SECURITY", "lock", "lock <name>...", "Make files or folders read-only",
      "A locked file cannot be written, deleted, moved or encrypted. A folder containing a locked item cannot be deleted. Can be undone." },
    { "SECURITY", "unlock", "unlock <name>...", "Remove the lock", "Can be undone." },
    { "SECURITY", "encrypt", "encrypt <file> [password]", "Encrypt a file",
      "Scrambles the content with the password (a simple XOR cipher, for learning - not real security). Only a hash of the password is kept. Encrypted files are left out of plagiarism checks. Can be undone." },
    { "SECURITY", "decrypt", "decrypt <file> [password]", "Decrypt a file", "Needs the same password. Can be undone." },

    { "HISTORY AND SYSTEM", "undo", "undo", "Undo the last change",
      "Works for mkdir, touch, write, append, rm, cp, mv, lock, unlock, encrypt and decrypt." },
    { "HISTORY AND SYSTEM", "redo", "redo", "Redo what was undone", "Any new change clears the redo list." },
    { "HISTORY AND SYSTEM", "history", "history [word]", "Show the commands typed so far",
      "With a word, only matching commands are shown. Type !n to run command number n again, or !! for the last one." },
    { "HISTORY AND SYSTEM", "stats", "stats", "Show statistics",
      "Number of files, folders, bytes, locked and encrypted items, history and plagiarism warnings." },
    { "HISTORY AND SYSTEM", "save", "save [file]", "Save everything to a real file",
      "Writes the whole virtual file system (with locks, encryption and bookmarks) to disk. Default name: minishell.dat." },
    { "HISTORY AND SYSTEM", "load", "load [file]", "Load a saved file",
      "Replaces everything in MiniShell with the saved data. The undo history is cleared." },
    { "HISTORY AND SYSTEM", "menu", "menu [letters]", "Show or search the menu",
      "'menu' shows the numbered menu (A-Z). 'menu cr' lists the options starting with cr. You can also just type letters like cr." },
    { "HISTORY AND SYSTEM", "man", "man [command]", "Show the manual of a command", "Without a name it lists all commands." },
    { "HISTORY AND SYSTEM", "help", "help", "List all commands", "" },
    { "HISTORY AND SYSTEM", "exit", "exit", "Quit MiniShell", "'quit' works too." }
};

const int COUNT = sizeof(ENTRIES) / sizeof(ENTRIES[0]);

} // namespace

void Manual::printHelp() {
    const char* currentSection = "";
    for (int i = 0; i < COUNT; i++) {
        const Entry& e = ENTRIES[i];
        if (std::string(currentSection) != e.section) {
            currentSection = e.section;
            std::cout << "\n" << currentSection << "\n";
        }
        std::string usage = e.usage;
        std::string pad = usage.size() < 44 ? std::string(44 - usage.size(), ' ') : " ";
        std::cout << "  " << usage << pad << e.summary << "\n";
    }
    std::cout << "\nType 'man <command>' for details, or just type a few letters (like cr) to find commands.\n";
}

void Manual::printNames() {
    for (int i = 0; i < COUNT; i++) std::cout << ENTRIES[i].name << (i + 1 < COUNT ? "  " : "\n");
}

bool Manual::printEntry(const std::string& name) {
    for (int i = 0; i < COUNT; i++) {
        if (name == ENTRIES[i].name) {
            const Entry& e = ENTRIES[i];
            std::cout << e.name << " - " << e.summary << "\n";
            std::cout << "Usage  : " << e.usage << "\n";
            if (e.details[0] != '\0') std::cout << "Details: " << e.details << "\n";
            return true;
        }
    }
    return false;
}
