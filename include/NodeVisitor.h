/* Defines the visitor interface used to process file and directory nodes. */

#ifndef NODEVISITOR_H
#define NODEVISITOR_H

#include <string>

class File;
class Directory;

class NodeVisitor {
public:
    virtual ~NodeVisitor() {}
    virtual void visitFile(const File& file, const std::string& path) = 0;
    virtual void visitDirectory(const Directory&, const std::string&) {}
};

#endif
