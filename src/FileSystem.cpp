#include "FileSystem.h"
#include <iostream>
#include <utility>

std::string statusMessage(Status s, const std::string& t) {
    switch (s) {
        case Status::OK:                 return "";
        case Status::NOT_FOUND:          return "'" + t + "': no such file or directory";
        case Status::ALREADY_EXISTS:     return "'" + t + "': already exists";
        case Status::NOT_A_DIRECTORY:    return "'" + t + "': not a directory";
        case Status::NOT_A_FILE:         return "'" + t + "': is a directory, not a file";
        case Status::INVALID_NAME:       return "'" + t + "': invalid name";
        case Status::CANNOT_REMOVE_ROOT: return "cannot remove or move the root directory";
        case Status::CANNOT_REMOVE_CWD:  return "'" + t + "': cannot remove or move the current directory or one of its parents";
        case Status::LOCKED:             return "'" + t + "': is locked (use 'unlock' first)";
        case Status::ENCRYPTED:          return "'" + t + "': file is encrypted (use 'decrypt' first)";
        case Status::ALREADY_ENCRYPTED:  return "'" + t + "': already encrypted";
        case Status::NOT_ENCRYPTED:      return "'" + t + "': is not encrypted";
        case Status::WRONG_PASSWORD:     return "wrong password";
        case Status::INSIDE_ITSELF:      return "cannot copy or move a folder into itself";
        case Status::TOO_LARGE:          return "'" + t + "': too large for this operation";
        case Status::IO_ERROR:           return "'" + t + "': could not read or write the file on disk";
        case Status::BAD_FORMAT:         return "'" + t + "': not a valid MiniShell data file";
    }
    return "unknown error";
}

FileSystem::FileSystem() {
    root = new Directory("/", nullptr);
    cwd  = root;
}

FileSystem::~FileSystem() {
    delete root;   // frees the whole tree recursively
}

bool FileSystem::validName(const std::string& n) const {
    if (n.empty() || n == "." || n == "..") return false;
    return n.find('/') == std::string::npos;
}

// Walks the path component by component.
FSNode* FileSystem::resolve(const std::string& path) const {
    if (path.empty()) return cwd;

    FSNode* node = (path[0] == '/') ? static_cast<FSNode*>(root) : static_cast<FSNode*>(cwd);
    size_t i = 0, n = path.size();

    while (i < n) {
        while (i < n && path[i] == '/') i++;
        if (i >= n) break;
        size_t j = i;
        while (j < n && path[j] != '/') j++;
        std::string part = path.substr(i, j - i);
        i = j;

        if (!node->isDirectory()) return nullptr;
        Directory* dir = static_cast<Directory*>(node);

        if (part == ".") continue;
        if (part == "..") {
            node = (dir->getParent() != nullptr) ? static_cast<FSNode*>(dir->getParent()) : node;
            continue;
        }
        node = dir->findChild(part);
        if (node == nullptr) return nullptr;
    }
    return node;
}

// Splits "a/b/c" into parent directory (a/b) and leaf name (c).
Status FileSystem::splitParent(const std::string& rawPath, Directory*& parentDir, std::string& leaf) const {
    std::string path = rawPath;
    while (path.size() > 1 && path[path.size() - 1] == '/') path.erase(path.size() - 1);

    size_t pos = path.find_last_of('/');
    std::string parentPath;
    if (pos == std::string::npos) {
        parentPath = "";
        leaf = path;
    } else {
        parentPath = (pos == 0) ? "/" : path.substr(0, pos);
        leaf = path.substr(pos + 1);
    }

    if (!validName(leaf)) return Status::INVALID_NAME;

    FSNode* p = resolve(parentPath);
    if (p == nullptr) return Status::NOT_FOUND;
    if (!p->isDirectory()) return Status::NOT_A_DIRECTORY;
    parentDir = static_cast<Directory*>(p);
    return Status::OK;
}

std::string FileSystem::pathOf(const FSNode* n) const {
    if (n->getParent() == nullptr) return "/";
    std::string up = pathOf(n->getParent());
    return (up == "/" ? "" : up) + "/" + n->getName();
}

Status FileSystem::absolutePath(const std::string& path, std::string& out) const {
    Directory* parentDir = nullptr;
    std::string leaf;
    Status s = splitParent(path, parentDir, leaf);
    if (s != Status::OK) return s;
    out = FSNode::joinPath(pathOf(parentDir), leaf);
    return Status::OK;
}

// ---------------------------------------------------------------- basic commands

Status FileSystem::mkdir(const std::string& path) {
    Directory* parentDir = nullptr;
    std::string leaf;
    Status s = splitParent(path, parentDir, leaf);
    if (s != Status::OK) return s;
    if (parentDir->findChild(leaf) != nullptr) return Status::ALREADY_EXISTS;
    parentDir->addChild(new Directory(leaf, parentDir));
    return Status::OK;
}

Status FileSystem::cd(const std::string& path) {
    FSNode* node = resolve(path);
    if (node == nullptr) return Status::NOT_FOUND;
    if (!node->isDirectory()) return Status::NOT_A_DIRECTORY;
    cwd = static_cast<Directory*>(node);
    return Status::OK;
}

Status FileSystem::touch(const std::string& path) {
    Directory* parentDir = nullptr;
    std::string leaf;
    Status s = splitParent(path, parentDir, leaf);
    if (s != Status::OK) return s;
    if (parentDir->findChild(leaf) != nullptr) return Status::OK;  // like real touch: no-op if it exists
    parentDir->addChild(new File(leaf, parentDir));
    return Status::OK;
}

Status FileSystem::ls(const std::string& path) const {
    FSNode* node = resolve(path);
    if (node == nullptr) return Status::NOT_FOUND;
    if (node->isDirectory()) static_cast<Directory*>(node)->listChildren();
    else node->display();
    return Status::OK;
}

Status FileSystem::cat(const std::string& path, std::string& out) const {
    FSNode* node = resolve(path);
    if (node == nullptr) return Status::NOT_FOUND;
    if (node->isDirectory()) return Status::NOT_A_FILE;
    File* f = static_cast<File*>(node);
    if (f->isEncrypted()) return Status::ENCRYPTED;
    out = f->getContent();
    return Status::OK;
}

// Reads a file without changing it; an encrypted file needs the right password.
Status FileSystem::catWithPassword(const std::string& path, const std::string& password, std::string& out) const {
    FSNode* node = resolve(path);
    if (node == nullptr) return Status::NOT_FOUND;
    if (node->isDirectory()) return Status::NOT_A_FILE;
    File* f = static_cast<File*>(node);
    if (!f->peek(password, out)) return Status::WRONG_PASSWORD;
    return Status::OK;
}

Status FileSystem::checkContent(const std::string& path, const std::string& text, PlagiarismResult& result) const {
    Directory* parentDir = nullptr;
    std::string leaf;
    Status s = splitParent(path, parentDir, leaf);
    if (s != Status::OK) return s;

    int selfId = 0;   // 0 = file does not exist yet
    FSNode* existing = parentDir->findChild(leaf);
    if (existing != nullptr) {
        if (existing->isDirectory()) return Status::NOT_A_FILE;
        File* f = static_cast<File*>(existing);
        if (f->isLocked())    return Status::LOCKED;
        if (f->isEncrypted()) return Status::ENCRYPTED;
        selfId = f->getId();
    }
    result = checker.check(text, selfId);
    return Status::OK;
}

Status FileSystem::writeFile(const std::string& path, const std::string& text) {
    Directory* parentDir = nullptr;
    std::string leaf;
    Status s = splitParent(path, parentDir, leaf);
    if (s != Status::OK) return s;

    FSNode* existing = parentDir->findChild(leaf);
    if (existing == nullptr) {
        File* f = new File(leaf, parentDir);
        f->setContent(text);
        parentDir->addChild(f);
        checker.indexFile(f->getId(), pathOf(f), text);
    } else if (existing->isDirectory()) {
        return Status::NOT_A_FILE;
    } else {
        File* f = static_cast<File*>(existing);
        if (f->isLocked())    return Status::LOCKED;
        if (f->isEncrypted()) return Status::ENCRYPTED;
        f->setContent(text);
        checker.indexFile(f->getId(), pathOf(f), text);
    }
    return Status::OK;
}

// ---------------------------------------------------------------- remove / move / copy

Status FileSystem::canRemove(const std::string& path) const {
    FSNode* node = resolve(path);
    if (node == nullptr) return Status::NOT_FOUND;
    if (node == root) return Status::CANNOT_REMOVE_ROOT;

    // Never delete the directory we are standing in, or any ancestor of it.
    for (Directory* d = cwd; d != nullptr; d = d->getParent()) {
        if (d == node) return Status::CANNOT_REMOVE_CWD;
    }
    // A locked item (or a folder holding one) is protected.
    if (node->containsLocked()) return Status::LOCKED;
    return Status::OK;
}

// Takes the node out of the tree (and out of the plagiarism index) but keeps it alive.
Status FileSystem::detach(const std::string& path, FSNode*& out) {
    Status s = canRemove(path);
    if (s != Status::OK) return s;

    FSNode* node = resolve(path);
    Directory* parentDir = node->getParent();
    node->unregisterFrom(checker);
    parentDir->unlinkChild(node);
    out = node;
    return Status::OK;
}

// Puts a detached node back under 'parentPath' and re-indexes its files.
Status FileSystem::attach(const std::string& parentPath, FSNode* node) {
    FSNode* p = resolve(parentPath);
    if (p == nullptr) return Status::NOT_FOUND;
    if (!p->isDirectory()) return Status::NOT_A_DIRECTORY;

    Directory* dir = static_cast<Directory*>(p);
    if (!dir->addChild(node)) return Status::ALREADY_EXISTS;
    node->registerWith(checker, pathOf(dir));
    return Status::OK;
}

// Permanent removal: detach, then free the node.
Status FileSystem::remove(const std::string& path) {
    FSNode* node = nullptr;
    Status s = detach(path, node);
    if (s != Status::OK) return s;
    delete node;
    return Status::OK;
}

// 'dst' is either an existing folder (the item goes inside it and keeps its name)
// or a new name inside an existing folder.
Status FileSystem::resolveDestination(const std::string& dst, const std::string& srcName,
                                      Directory*& destDir, std::string& newName) const {
    FSNode* target = resolve(dst);
    if (target != nullptr) {
        if (!target->isDirectory()) return Status::ALREADY_EXISTS;   // dst is an existing file
        destDir = static_cast<Directory*>(target);
        newName = srcName;
        if (destDir->findChild(newName) != nullptr) return Status::ALREADY_EXISTS;
        return Status::OK;
    }
    return splitParent(dst, destDir, newName);
}

Status FileSystem::copy(const std::string& src, const std::string& dst, std::string& createdPath) {
    FSNode* node = resolve(src);
    if (node == nullptr) return Status::NOT_FOUND;

    Directory* destDir = nullptr;
    std::string newName;
    Status s = resolveDestination(dst, node->getName(), destDir, newName);
    if (s != Status::OK) return s;

    // A folder cannot be copied into itself or into one of its own subfolders.
    if (node->isDirectory()) {
        for (Directory* d = destDir; d != nullptr; d = d->getParent()) {
            if (d == node) return Status::INSIDE_ITSELF;
        }
    }

    FSNode* copyNode = node->clone();
    copyNode->setName(newName);
    destDir->addChild(copyNode);
    copyNode->registerWith(checker, pathOf(destDir));   // the copy takes part in plagiarism checks
    createdPath = pathOf(copyNode);
    return Status::OK;
}

Status FileSystem::move(const std::string& src, const std::string& dst, std::string& finalPath) {
    Status c = canRemove(src);   // same protection as rm: root, current folder, locked items
    if (c != Status::OK) return c;

    FSNode* node = resolve(src);
    Directory* destDir = nullptr;
    std::string newName;
    Status s = resolveDestination(dst, node->getName(), destDir, newName);
    if (s != Status::OK) return s;

    if (node->isDirectory()) {
        for (Directory* d = destDir; d != nullptr; d = d->getParent()) {
            if (d == node) return Status::INSIDE_ITSELF;
        }
    }

    FSNode* detached = nullptr;
    s = detach(src, detached);
    if (s != Status::OK) return s;

    detached->setName(newName);        // rename (same name when only moving)
    destDir->addChild(detached);
    detached->registerWith(checker, pathOf(destDir));
    finalPath = pathOf(detached);
    return Status::OK;
}

// ---------------------------------------------------------------- security

Status FileSystem::setLocked(const std::string& path, bool value, bool& changed) {
    FSNode* node = resolve(path);
    if (node == nullptr) return Status::NOT_FOUND;
    changed = (node->isLocked() != value);
    node->setLocked(value);
    return Status::OK;
}

Status FileSystem::encryptFile(const std::string& path, const std::string& password) {
    FSNode* node = resolve(path);
    if (node == nullptr) return Status::NOT_FOUND;
    if (node->isDirectory()) return Status::NOT_A_FILE;
    File* f = static_cast<File*>(node);
    if (f->isLocked())    return Status::LOCKED;
    if (f->isEncrypted()) return Status::ALREADY_ENCRYPTED;

    f->encrypt(password);
    checker.removeFile(f->getId());   // scrambled text must not appear in plagiarism checks
    return Status::OK;
}

Status FileSystem::decryptFile(const std::string& path, const std::string& password) {
    FSNode* node = resolve(path);
    if (node == nullptr) return Status::NOT_FOUND;
    if (node->isDirectory()) return Status::NOT_A_FILE;
    File* f = static_cast<File*>(node);
    if (f->isLocked())     return Status::LOCKED;
    if (!f->isEncrypted()) return Status::NOT_ENCRYPTED;
    if (!f->decrypt(password)) return Status::WRONG_PASSWORD;

    checker.indexFile(f->getId(), pathOf(f), f->getContent());   // readable again
    return Status::OK;
}

// ---------------------------------------------------------------- load support

Status FileSystem::loadFile(const std::string& path, const std::string& content,
                            bool lockedFlag, bool encryptedFlag, unsigned long long hash) {
    Directory* parentDir = nullptr;
    std::string leaf;
    Status s = splitParent(path, parentDir, leaf);
    if (s != Status::OK) return s;
    if (parentDir->findChild(leaf) != nullptr) return Status::ALREADY_EXISTS;

    File* f = new File(leaf, parentDir);
    if (encryptedFlag) f->restoreEncrypted(content, hash);
    else f->setContent(content);
    f->setLocked(lockedFlag);
    parentDir->addChild(f);
    if (!encryptedFlag) checker.indexFile(f->getId(), pathOf(f), content);
    return Status::OK;
}

namespace {
// Puts every readable file of a tree into the plagiarism index.
class IndexVisitor : public NodeVisitor {
private:
    PlagiarismChecker& checker;
public:
    explicit IndexVisitor(PlagiarismChecker& c) : checker(c) {}
    void visitFile(const File& f, const std::string& path) override {
        if (!f.isEncrypted()) checker.indexFile(f.getId(), path, f.getContent());
    }
};
}

void FileSystem::reindexAll() {
    IndexVisitor visitor(checker);
    root->accept(visitor, "/");
}

// Takes over the tree and bookmarks of 'other'. 'other' ends up holding our old
// tree, which is freed when 'other' is destroyed.
void FileSystem::adopt(FileSystem& other) {
    // The current checker has no clear() method, so remove the old tree's
    // entries through its normal unregister hook before replacing the tree.
    root->unregisterFrom(checker);
    std::swap(root, other.root);
    cwd = other.cwd;            // other.cwd points into the tree we now own
    other.cwd = other.root;

    bookmarks.clear();
    std::string name, path;
    for (int i = 0; other.bookmarks.entryAt(i, name, path); i++) bookmarks.set(name, path);

    reindexAll();               // rebuild the plagiarism index for the new tree
}
