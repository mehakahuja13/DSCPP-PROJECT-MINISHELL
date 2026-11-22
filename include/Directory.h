/* Defines a directory node and the linked list used to keep its children. */

#ifndef DIRECTORY_H
#define DIRECTORY_H

#include "FSNode.h"

class Directory : public FSNode {
private:
    struct ChildNode {
        FSNode*    node;
        ChildNode* next;
        ChildNode(FSNode* n) : node(n), next(nullptr) {}
    };

    ChildNode* head;
    int        count;

public:
    Directory(const std::string& n, Directory* p = nullptr);
    ~Directory();

    Directory(const Directory&) = delete;
    Directory& operator=(const Directory&) = delete;

    bool        isDirectory() const override { return true; }
    void        display() const override;
    std::string getLabel() const override;
    size_t      getSize() const override;
    FSNode*     clone() const override;
    bool        containsLocked() const override;
    void        accept(NodeVisitor& visitor, const std::string& path) const override;
    void        unregisterFrom(PlagiarismChecker& checker) const override;
    void        registerWith(PlagiarismChecker& checker, const std::string& dirPath) const override;

    FSNode* findChild(const std::string& childName) const;
    bool    addChild(FSNode* child);
    bool    removeChild(const std::string& childName);
    bool    unlinkChild(FSNode* child);
    void    listChildren() const;
    void    printTree(const std::string& prefix) const;

    int  getCount() const { return count; }
    bool isEmpty() const { return count == 0; }
};

#endif
