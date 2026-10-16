#include "File.h"
#include <iostream>

File::File(const std::string& n, Directory* p) : FSNode(n, p) {}

void File::display() const {
    std::cout << "[F] " << name << "  (" << content.size() << " bytes)\n";
}
