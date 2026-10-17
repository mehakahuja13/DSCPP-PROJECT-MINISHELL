#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <string>
#include "Directory.h"
#include "File.h"
#include "PlagiarismChecker.h"
#include "Bookmarks.h"

enum class Status {
    OK,
    NOT_FOUND,
    ALREADY_EXISTS,
    NOT_A_DIRECTORY,
    NOT_A_FILE,
    INVALID_NAME,
    CANNOT_REMOVE_ROOT,
    CANNOT_REMOVE_CWD,
    LOCKED,
    ENCRYPTED,
    ALREADY_ENCRYPTED,
    NOT_ENCRYPTED,
    WRONG_PASSWORD,
    INSIDE_ITSELF,
    TOO_LARGE,
    IO_ERROR,
    BAD_FORMAT
};

// Human-readable message for a Status code.
std::string statusMessage(Status s, const std::string& target);

// Owns the whole tree, the current working directory, the plagiarism index and the bookmarks.
class FileSystem {
private:
    Directory*        root;
    Directory*        cwd;
    PlagiarismChecker checker;
    Bookmarks         bookmarks;

    bool   validName(const std::string& n) const;
    Status splitParent(const std::string& path, Directory*& parentDir, std::string& leaf) const;
    Status resolveDestination(const std::string& dst, const std::string& srcName,
                              Directory*& destDir, std::string& newName) const;
    void   reindexAll();

public:
    FileSystem();
    ~FileSystem();

    FileSystem(const FileSystem&) = delete;
    FileSystem& operator=(const FileSystem&) = delete;

    // Supports absolute (/a/b), relative (a/b), "." and ".."
    FSNode* resolve(const std::string& path) const;

    // Absolute path of an existing node, e.g. "/docs/a.txt".
    std::string pathOf(const FSNode* n) const;

    // Absolute path for a (possibly not yet existing) target, e.g. "a.txt" -> "/docs/a.txt".
    Status absolutePath(const std::string& path, std::string& out) const;

    const Directory* getRoot() const { return root; }
    Bookmarks&       getBookmarks() { return bookmarks; }
    const Bookmarks& getBookmarks() const { return bookmarks; }

    // ---- basic commands ----
    Status mkdir(const std::string& path);
    Status cd(const std::string& path);
    Status touch(const std::string& path);
    Status ls(const std::string& path) const;
    Status cat(const std::string& path, std::string& out) const;      // ENCRYPTED if a password is needed
    Status catWithPassword(const std::string& path, const std::string& password, std::string& out) const;

    // Validates the path and compares the text with all saved files. Does NOT save.
    Status checkContent(const std::string& path, const std::string& text, PlagiarismResult& result) const;
    Status writeFile(const std::string& path, const std::string& text);   // creates if missing

    // ---- removing / moving ----
    Status canRemove(const std::string& path) const;   // root, current folder, locked items are protected
    Status remove(const std::string& path);            // permanent
    Status detach(const std::string& path, FSNode*& out);        // take out of the tree, keep alive (undo)
    Status attach(const std::string& parentPath, FSNode* node);  // put it back

    Status copy(const std::string& src, const std::string& dst, std::string& createdPath);
    Status move(const std::string& src, const std::string& dst, std::string& finalPath);

    // ---- security ----
    Status setLocked(const std::string& path, bool value, bool& changed);
    Status encryptFile(const std::string& path, const std::string& password);
    Status decryptFile(const std::string& path, const std::string& password);

    // ---- used by load (rebuilding a saved file system) ----
    Status loadFile(const std::string& path, const std::string& content,
                    bool lockedFlag, bool encryptedFlag, unsigned long long hash);
    void   adopt(FileSystem& other);   // take over another file system's tree and bookmarks

    std::string pwd() const { return pathOf(cwd); }
};

#endif
