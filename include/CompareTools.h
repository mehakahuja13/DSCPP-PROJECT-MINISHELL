#ifndef COMPARETOOLS_H
#define COMPARETOOLS_H

#include <string>
#include "FileSystem.h"

// Commands that compare the CONTENT of files: compare, scan, diff.
class CompareTools {
public:
    // How much of each file's text (3-word groups) also appears in the other.
    static Status compare(const FileSystem& fs, const std::string& a, const std::string& b);

    // Checks every pair of readable files and lists the suspicious ones.
    static Status scan(const FileSystem& fs);

    // Line-by-line difference using the Longest Common Subsequence (dynamic programming).
    static Status diff(const FileSystem& fs, const std::string& a, const std::string& b);
};

#endif
