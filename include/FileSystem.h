#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <string>
#include "Directory.h"
#include "File.h"
#include "PlagiarismChecker.h"

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
    PlagiarismChecker checker;

    bool   validName(const std::string& n) const;
    Status splitParent(const std::string& path, Directory*& parentDir, std::string& leaf) const;

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
    // Validates the path and compares the text with all saved files. Does NOT save.
    Status checkContent(const std::string& path, const std::string& text, PlagiarismResult& result) const;
    Status writeFile(const std::string& path, const std::string& text);  // creates if missing
    Status cat(const std::string& path, std::string& out) const;
    Status ls(const std::string& path) const;
    Status canRemove(const std::string& path) const;   // checks root / current-dir protection
    Status remove(const std::string& path);

    // Absolute path of an existing node, e.g. "/docs/a.txt".
    std::string pathOf(const FSNode* n) const;

    // Absolute path for a (possibly not yet existing) target, e.g. "a.txt" -> "/docs/a.txt".
    Status absolutePath(const std::string& path, std::string& out) const;

    // Used by undo: take a node out of the tree WITHOUT deleting it, and put it back.
    Status detach(const std::string& path, FSNode*& out);
    Status attach(const std::string& parentPath, FSNode* node);

    std::string pwd() const { return pathOf(cwd); }
};

#endif
