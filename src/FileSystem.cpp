#include "FileSystem.h"
#include <iostream>

std::string statusMessage(Status s, const std::string& t) {
    switch (s) {
        case Status::OK:                 return "";
        case Status::NOT_FOUND:          return "'" + t + "': no such file or directory";
        case Status::ALREADY_EXISTS:     return "'" + t + "': already exists";
        case Status::NOT_A_DIRECTORY:    return "'" + t + "': not a directory";
        case Status::NOT_A_FILE:         return "'" + t + "': is a directory, not a file";
        case Status::INVALID_NAME:       return "'" + t + "': invalid name";
        case Status::CANNOT_REMOVE_ROOT: return "cannot remove the root directory";
        case Status::CANNOT_REMOVE_CWD:  return "'" + t + "': cannot remove the current directory or one of its parents";
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

std::string FileSystem::pathOf(const Directory* d) const {
    if (d->getParent() == nullptr) return "/";
    std::string up = pathOf(d->getParent());
    return (up == "/" ? "" : up) + "/" + d->getName();
}

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
    } else if (existing->isDirectory()) {
        return Status::NOT_A_FILE;
    } else {
        static_cast<File*>(existing)->setContent(text);
    }
    return Status::OK;
}

Status FileSystem::cat(const std::string& path, std::string& out) const {
    FSNode* node = resolve(path);
    if (node == nullptr) return Status::NOT_FOUND;
    if (node->isDirectory()) return Status::NOT_A_FILE;
    out = static_cast<File*>(node)->getContent();
    return Status::OK;
}

Status FileSystem::ls(const std::string& path) const {
    FSNode* node = resolve(path);
    if (node == nullptr) return Status::NOT_FOUND;
    if (node->isDirectory()) static_cast<Directory*>(node)->listChildren();
    else node->display();
    return Status::OK;
}

Status FileSystem::canRemove(const std::string& path) const {
    FSNode* node = resolve(path);
    if (node == nullptr) return Status::NOT_FOUND;
    if (node == root) return Status::CANNOT_REMOVE_ROOT;

    // Never delete the directory we are standing in, or any ancestor of it.
    for (Directory* d = cwd; d != nullptr; d = d->getParent()) {
        if (d == node) return Status::CANNOT_REMOVE_CWD;
    }
    return Status::OK;
}

Status FileSystem::remove(const std::string& path) {
    Status s = canRemove(path);
    if (s != Status::OK) return s;

    FSNode* node = resolve(path);
    Directory* parentDir = node->getParent();
    std::string nm = node->getName();   // copy: node is deleted below
    parentDir->removeChild(nm);
    return Status::OK;
}
