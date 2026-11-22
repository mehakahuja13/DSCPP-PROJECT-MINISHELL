# Build MiniShell: main.cpp includes all the other source files.
CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -Iinclude -Isrc

minishell.exe: main.cpp
	$(CXX) $(CXXFLAGS) main.cpp -o minishell.exe

clean:
	rm -f minishell minishell.exe

.PHONY: clean
