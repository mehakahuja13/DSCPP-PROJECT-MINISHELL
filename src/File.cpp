#include "File.h"
#include "PlagiarismChecker.h"
#include <iostream>

int File::nextId = 1;

File::File(const std::string& n, Directory* p) : FSNode(n, p), id(nextId++) {}

void File::unregisterFrom(PlagiarismChecker& checker) const {
    checker.removeFile(id);
}

void File::registerWith(PlagiarismChecker& checker, const std::string& dirPath) const {
    checker.indexFile(id, joinPath(dirPath, name), content);
}

void File::display() const {
    std::cout << "[F] " << name << "  (" << content.size() << " bytes)\n";
}
