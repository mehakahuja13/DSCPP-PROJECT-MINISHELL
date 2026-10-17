#include "Directory.h"
#include "PlagiarismChecker.h"
#include <iostream>

Directory::Directory(const std::string& n, Directory* p)
    : FSNode(n, p), head(nullptr), count(0) {}

// Walk the list, delete every child (a child Directory deletes its own subtree).
Directory::~Directory() {
    ChildNode* cur = head;
    while (cur != nullptr) {
        ChildNode* nextNode = cur->next;
        delete cur->node;
        delete cur;
        cur = nextNode;
    }
}

std::string Directory::getLabel() const {
    std::string label = name + "/";
    if (locked) label += " [locked]";
    return label;
}

void Directory::display() const {
    std::cout << "[D] " << getLabel() << "\n";
}

size_t Directory::getSize() const {
    size_t total = 0;
    for (ChildNode* cur = head; cur != nullptr; cur = cur->next) total += cur->node->getSize();
    return total;
}

// Deep copy: clone every child (each File / Directory clones itself).
FSNode* Directory::clone() const {
    Directory* copy = new Directory(name, nullptr);
    for (ChildNode* cur = head; cur != nullptr; cur = cur->next) {
        copy->addChild(cur->node->clone());
    }
    return copy;
}

bool Directory::containsLocked() const {
    if (locked) return true;
    for (ChildNode* cur = head; cur != nullptr; cur = cur->next) {
        if (cur->node->containsLocked()) return true;
    }
    return false;
}

// Visit this folder first, then everything inside it (depth-first).
void Directory::accept(NodeVisitor& visitor, const std::string& path) const {
    visitor.visitDirectory(*this, path);
    for (ChildNode* cur = head; cur != nullptr; cur = cur->next) {
        cur->node->accept(visitor, joinPath(path, cur->node->getName()));
    }
}

// Forward to every child, so all files in this subtree get removed from the index.
void Directory::unregisterFrom(PlagiarismChecker& checker) const {
    for (ChildNode* cur = head; cur != nullptr; cur = cur->next) {
        cur->node->unregisterFrom(checker);
    }
}

void Directory::registerWith(PlagiarismChecker& checker, const std::string& dirPath) const {
    std::string myPath = joinPath(dirPath, name);
    for (ChildNode* cur = head; cur != nullptr; cur = cur->next) {
        cur->node->registerWith(checker, myPath);
    }
}

FSNode* Directory::findChild(const std::string& childName) const {
    for (ChildNode* cur = head; cur != nullptr; cur = cur->next) {
        if (cur->node->getName() == childName) return cur->node;
    }
    return nullptr;
}

bool Directory::addChild(FSNode* child) {
    if (findChild(child->getName()) != nullptr) return false;

    ChildNode* newNode = new ChildNode(child);
    if (head == nullptr) {
        head = newNode;
    } else {
        ChildNode* cur = head;
        while (cur->next != nullptr) cur = cur->next;
        cur->next = newNode;
    }
    child->setParent(this);
    count++;
    return true;
}

bool Directory::removeChild(const std::string& childName) {
    ChildNode* prev = nullptr;
    ChildNode* cur  = head;
    while (cur != nullptr) {
        if (cur->node->getName() == childName) {
            if (prev == nullptr) head = cur->next;
            else prev->next = cur->next;
            delete cur->node;
            delete cur;
            count--;
            return true;
        }
        prev = cur;
        cur  = cur->next;
    }
    return false;
}

// Removes the child from the list WITHOUT deleting it (the caller keeps it).
bool Directory::unlinkChild(FSNode* child) {
    ChildNode* prev = nullptr;
    ChildNode* cur  = head;
    while (cur != nullptr) {
        if (cur->node == child) {
            if (prev == nullptr) head = cur->next;
            else prev->next = cur->next;
            delete cur;           // only the list cell, not the node itself
            count--;
            child->setParent(nullptr);
            return true;
        }
        prev = cur;
        cur  = cur->next;
    }
    return false;
}

void Directory::listChildren() const {
    if (head == nullptr) {
        std::cout << "(empty)\n";
        return;
    }
    for (ChildNode* cur = head; cur != nullptr; cur = cur->next) {
        cur->node->display();   // polymorphic call
    }
}

// Draws the subtree like:
//   |-- docs/
//   |   `-- a.txt
//   `-- b.txt
void Directory::printTree(const std::string& prefix) const {
    for (ChildNode* cur = head; cur != nullptr; cur = cur->next) {
        bool last = (cur->next == nullptr);
        std::cout << prefix << (last ? "`-- " : "|-- ") << cur->node->getLabel() << "\n";
        if (cur->node->isDirectory()) {
            static_cast<const Directory*>(cur->node)->printTree(prefix + (last ? "    " : "|   "));
        }
    }
}
