CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -Iinclude
SRC = main.cpp src/File.cpp src/Directory.cpp src/FileSystem.cpp src/MiniShell.cpp src/Menu.cpp src/CommandCorrector.cpp

minishell: $(SRC)
	$(CXX) $(CXXFLAGS) -o minishell $(SRC)

clean:
	rm -f minishell
