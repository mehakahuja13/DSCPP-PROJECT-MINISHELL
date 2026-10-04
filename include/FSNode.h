#ifndef FSNODE_H
#define FSNODE_H

#include <string>

class Directory;

// Abstract base class for everything that lives in the virtual file system.
// ABSTRACTION  : cannot be instantiated (pure virtual functions).
// ENCAPSULATION: name and parent are protected, accessed via getters.
class FSNode {
protected:
    std::string name;
    Directory*  parent;   // upward link in the tree (nullptr for root)

public:
    FSNode(const std::string& n, Directory* p = nullptr) : name(n), parent(p) {}
    virtual ~FSNode() {}  // virtual so deleting via FSNode* frees the derived class too

    const std::string& getName() const { return name; }
    Directory* getParent() const { return parent; }
    void setParent(Directory* p) { parent = p; }

    virtual bool isDirectory() const = 0;
    virtual void display() const = 0;   // POLYMORPHISM: File and Directory print differently
};

#endif
