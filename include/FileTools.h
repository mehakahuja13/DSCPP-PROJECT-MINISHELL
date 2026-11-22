/* Declares extra file utilities such as tree display, search and size information. */

#ifndef FILETOOLS_H
#define FILETOOLS_H

#include <string>
#include "FileSystem.h"

struct TreeStats {
    int    files;
    int    dirs;
    size_t bytes;
    int    locked;
    int    encrypted;
    TreeStats() : files(0), dirs(0), bytes(0), locked(0), encrypted(0) {}
};

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
