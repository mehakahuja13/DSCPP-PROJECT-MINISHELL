/* Defines the base type shared by files and directories in the virtual file system. */

#ifndef FSNODE_H
#define FSNODE_H

#include <string>
#include <cstddef>
#include "NodeVisitor.h"

class Directory;
class PlagiarismChecker;

class FSNode {
protected:
    std::string name;
    Directory*  parent;
    bool        locked;

public:
    FSNode(const std::string& n, Directory* p = nullptr) : name(n), parent(p), locked(false) {}
    virtual ~FSNode() {}

    const std::string& getName() const { return name; }
    void setName(const std::string& n) { name = n; }
    Directory* getParent() const { return parent; }
    void setParent(Directory* p) { parent = p; }

    bool isLocked() const { return locked; }
    void setLocked(bool value) { locked = value; }

    virtual bool        isDirectory() const = 0;
    virtual void        display() const = 0;
    virtual std::string getLabel() const = 0;
    virtual size_t      getSize() const = 0;
    virtual FSNode*     clone() const = 0;
    virtual bool        containsLocked() const = 0;
    virtual void        accept(NodeVisitor& visitor, const std::string& path) const = 0;

    virtual void unregisterFrom(PlagiarismChecker& checker) const = 0;
    virtual void registerWith(PlagiarismChecker& checker, const std::string& dirPath) const = 0;

    static std::string joinPath(const std::string& dir, const std::string& leaf) {
        return (dir == "/" ? "" : dir) + "/" + leaf;
    }
};

#endif
