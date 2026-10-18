#ifndef NODEVISITOR_H
#define NODEVISITOR_H

#include <string>

class File;
class Directory;

// VISITOR design pattern: walk the whole tree and run some action on every
// file / folder without adding that action to the File and Directory classes.
// (find, grep, du, stats, scan, save and the plagiarism re-index all use this.)
class NodeVisitor {
public:
    virtual ~NodeVisitor() {}
    virtual void visitFile(const File& file, const std::string& path) = 0;
    virtual void visitDirectory(const Directory&, const std::string&) {}   // optional
};

#endif
