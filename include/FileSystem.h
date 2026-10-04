#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <string>
#include "Directory.h"
#include "File.h"

enum class Status {
    OK,
    NOT_FOUND,
    ALREADY_EXISTS,
    NOT_A_DIRECTORY,
    NOT_A_FILE,
    INVALID_NAME,
    CANNOT_REMOVE_ROOT,
    CANNOT_REMOVE_CWD
};

// Human-readable message for a Status code.
std::string statusMessage(Status s, const std::string& target);

// Owns the whole tree and the current working directory.
class FileSystem {
private:
    Directory* root;
    Directory* cwd;

    bool   validName(const std::string& n) const;
    Status splitParent(const std::string& path, Directory*& parentDir, std::string& leaf) const;
    std::string pathOf(const Directory* d) const;

public:
    FileSystem();
    ~FileSystem();

    FileSystem(const FileSystem&) = delete;
    FileSystem& operator=(const FileSystem&) = delete;

    // Supports absolute (/a/b), relative (a/b), "." and ".."
    FSNode* resolve(const std::string& path) const;

    Status mkdir(const std::string& path);
    Status cd(const std::string& path);
    Status touch(const std::string& path);
    Status writeFile(const std::string& path, const std::string& text);  // creates if missing
    Status cat(const std::string& path, std::string& out) const;
    Status ls(const std::string& path) const;
    Status canRemove(const std::string& path) const;   // checks root / current-dir protection
    Status remove(const std::string& path);

    std::string pwd() const { return pathOf(cwd); }
};

#endif
