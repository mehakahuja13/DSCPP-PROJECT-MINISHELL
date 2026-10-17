#ifndef DIRECTORY_H
#define DIRECTORY_H

#include "FSNode.h"

// A directory node. Its children are stored in a hand-written singly linked list.
//   TREE        : Directory -> children (File / Directory) -> ...
//   LINKED LIST : ChildNode chain holding the children of one directory
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
    ~Directory();   // recursively frees the entire subtree

    // Not copyable: copying would lead to double deletion of children.
    Directory(const Directory&) = delete;
    Directory& operator=(const Directory&) = delete;

    bool        isDirectory() const override { return true; }
    void        display() const override;
    std::string getLabel() const override;
    size_t      getSize() const override;          // sum of all children
    FSNode*     clone() const override;            // recursive deep copy
    bool        containsLocked() const override;
    void        accept(NodeVisitor& visitor, const std::string& path) const override;
    void        unregisterFrom(PlagiarismChecker& checker) const override;
    void        registerWith(PlagiarismChecker& checker, const std::string& dirPath) const override;

    FSNode* findChild(const std::string& childName) const;
    bool    addChild(FSNode* child);                       // false if name already exists
    bool    removeChild(const std::string& childName);     // unlinks AND deletes
    bool    unlinkChild(FSNode* child);                    // unlinks but keeps the node alive
    void    listChildren() const;
    void    printTree(const std::string& prefix) const;    // used by the 'tree' command

    int  getCount() const { return count; }
    bool isEmpty() const { return count == 0; }
};

#endif
