#ifndef FSNODE_H
#define FSNODE_H

#include <string>
#include <cstddef>
#include "NodeVisitor.h"

class Directory;
class PlagiarismChecker;

// Abstract base class for everything that lives in the virtual file system.
// ABSTRACTION  : cannot be instantiated (pure virtual functions).
// ENCAPSULATION: name, parent and lock flag are protected, accessed via methods.
class FSNode {
protected:
    std::string name;
    Directory*  parent;   // upward link in the tree (nullptr for root)
    bool        locked;   // read-only protection (lock / unlock)

public:
    FSNode(const std::string& n, Directory* p = nullptr) : name(n), parent(p), locked(false) {}
    virtual ~FSNode() {}   // virtual so deleting via FSNode* frees the derived class too

    const std::string& getName() const { return name; }
    void setName(const std::string& n) { name = n; }
    Directory* getParent() const { return parent; }
    void setParent(Directory* p) { parent = p; }

    bool isLocked() const { return locked; }
    void setLocked(bool value) { locked = value; }

    // ---- POLYMORPHISM: File and Directory answer these in their own way ----
    virtual bool        isDirectory() const = 0;
    virtual void        display() const = 0;
    virtual std::string getLabel() const = 0;      // name + marks like [locked]
    virtual size_t      getSize() const = 0;       // bytes (a folder adds up its children)
    virtual FSNode*     clone() const = 0;         // deep copy (used by cp)
    virtual bool        containsLocked() const = 0;
    virtual void        accept(NodeVisitor& visitor, const std::string& path) const = 0;

    // Plagiarism index support: called before a node is deleted / after it is restored.
    virtual void unregisterFrom(PlagiarismChecker& checker) const = 0;
    virtual void registerWith(PlagiarismChecker& checker, const std::string& dirPath) const = 0;

    // "/" + "a" -> "/a",   "/docs" + "a" -> "/docs/a"
    static std::string joinPath(const std::string& dir, const std::string& leaf) {
        return (dir == "/" ? "" : dir) + "/" + leaf;
    }
};

#endif
