/* Program entry point. This one file pulls in every other source file,
   so the whole program compiles with a single command:

       g++ -std=c++11 main.cpp -o minishell.exe
*/

#include "Trie.cpp"
#include "HashTable.cpp"
#include "Bookmarks.cpp"
#include "PlagiarismChecker.cpp"
#include "File.cpp"
#include "Directory.cpp"
#include "FileSystem.cpp"
#include "FileTools.cpp"
#include "CompareTools.cpp"
#include "Persistence.cpp"
#include "CommandManager.cpp"
#include "Commands.cpp"
#include "CommandCorrector.cpp"
#include "Manual.cpp"
#include "Menu.cpp"
#include "MiniShell.cpp"

int main() {
    MiniShell shell;
    shell.run();
    return 0;
}
