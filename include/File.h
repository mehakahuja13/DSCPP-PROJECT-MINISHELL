#ifndef FILE_H
#define FILE_H

#include "FSNode.h"

// A file node. Content is private (encapsulation).
class File : public FSNode {
private:
    std::string content;

public:
    File(const std::string& n, Directory* p = nullptr);

    bool isDirectory() const override { return false; }
    void display() const override;

    const std::string& getContent() const { return content; }
    void setContent(const std::string& c) { content = c; }
    size_t getSize() const { return content.size(); }
};

#endif
