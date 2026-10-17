CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -Iinclude
SRC = main.cpp src/File.cpp src/Directory.cpp src/FileSystem.cpp src/MiniShell.cpp src/Menu.cpp src/CommandCorrector.cpp src/HashTable.cpp src/PlagiarismChecker.cpp src/Trie.cpp src/Commands.cpp src/CommandManager.cpp src/Bookmarks.cpp src/Manual.cpp src/FileTools.cpp src/Persistence.cpp src/CompareTools.cpp
minishell: $(SRC)
	$(CXX) $(CXXFLAGS) -o minishell $(SRC)

clean:
	rm -f minishell
