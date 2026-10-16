
#ifndef FSNODE_H
#define FSNODE_H

#include <string>

class Directory;

class FSNode {
protected:
    std::string name;
    Directory* parent;
    int id;

private:
    static int& nextIdCounter() {
        static int nextId = 1;
        return nextId;
    }

public:
    FSNode(const std::string& n, Directory* p = nullptr)
        : name(n), parent(p), id(nextIdCounter()++) {}

    virtual ~FSNode() {}

    const std::string& getName() const { return name; }
    Directory* getParent() const { return parent; }
    int getId() const { return id; }

    void setParent(Directory* p) { parent = p; }

    virtual bool isDirectory() const = 0;
    virtual void display() const = 0;
};

#endif
