/* Declares saving and loading for the virtual file-system state. */

#ifndef PERSISTENCE_H
#define PERSISTENCE_H

#include <string>
#include "FileSystem.h"

class Persistence {
public:
    static Status save(const FileSystem& fs, const std::string& filename);

    static Status load(FileSystem& fs, const std::string& filename);
};

#endif
