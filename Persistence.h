#ifndef PERSISTENCE_H
#define PERSISTENCE_H

#include <string>
#include "FileSystem.h"

// Saves the whole virtual file system into a real text file on disk and loads it back.
// File format (one item per line, parents always before children):
//   MINISHELL-DATA 1
//   CWD <path>
//   D <path> <locked>
//   F <path> <locked> <encrypted> <password-hash> <content as hex, or - if empty>
//   B <bookmark name> <path>
//   END
class Persistence {
public:
    static Status save(const FileSystem& fs, const std::string& filename);

    // Builds the data in a temporary FileSystem first; only if everything is valid
    // does it replace the real one, so a damaged file can never ruin the current state.
    static Status load(FileSystem& fs, const std::string& filename);
};

#endif
