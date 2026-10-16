#include "Directory.h"
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

void Directory::display() const {
    std::cout << "[D] " << name << "/\n";
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

void Directory::listChildren() const {
    if (head == nullptr) {
        std::cout << "(empty)\n";
        return;
    }
    for (ChildNode* cur = head; cur != nullptr; cur = cur->next) {
        cur->node->display();   // polymorphic call
    }
}
