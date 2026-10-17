#ifndef FILETOOLS_H
#define FILETOOLS_H

#include <string>
#include "FileSystem.h"

// Totals collected by walking a part of the tree.
struct TreeStats {
    int    files;
    int    dirs;       // folders INSIDE the starting point (the starting folder itself is not counted)
    size_t bytes;
    int    locked;
    int    encrypted;
    TreeStats() : files(0), dirs(0), bytes(0), locked(0), encrypted(0) {}
};

// Read-only commands that look at the tree. Most of them walk it with a NodeVisitor.
class FileTools {
public:
    static TreeStats collect(const FileSystem& fs, const FSNode* start);

    static Status tree(const FileSystem& fs, const std::string& path);
    static Status info(const FileSystem& fs, const std::string& path);
    static Status du(const FileSystem& fs, const std::string& path);
    static Status find(const FileSystem& fs, const std::string& text);
    static Status grep(const FileSystem& fs, const std::string& word, const std::string& path);
    static Status wc(const FileSystem& fs, const std::string& path);
};

#endif
