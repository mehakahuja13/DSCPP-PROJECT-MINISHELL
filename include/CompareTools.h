/* Declares utilities for comparing files, scanning for matches and showing differences. */

#ifndef COMPARETOOLS_H
#define COMPARETOOLS_H

#include <string>
#include "FileSystem.h"

class CompareTools {
public:

    static Status compare(const FileSystem& fs, const std::string& a, const std::string& b);

    static Status scan(const FileSystem& fs);

    static Status diff(const FileSystem& fs, const std::string& a, const std::string& b);
};

#endif
