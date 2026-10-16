#ifndef FILE_H
#define FILE_H

#include "FSNode.h"

// A file node. Content is private (encapsulation).
class File : public FSNode {
private:
    static int  nextId;     // gives every file a unique id
    int         id;
    std::string content;

public:
    File(const std::string& n, Directory* p = nullptr);

    bool isDirectory() const override { return false; }
    void display() const override;
    void unregisterFrom(PlagiarismChecker& checker) const override;
    void registerWith(PlagiarismChecker& checker, const std::string& dirPath) const override;

    int getId() const { return id; }

    const std::string& getContent() const { return content; }
    void setContent(const std::string& c) { content = c; }
    size_t getSize() const { return content.size(); }
};

#endif
