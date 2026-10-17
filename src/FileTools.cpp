#include "FileTools.h"
#include <iostream>
#include <cctype>

namespace {

std::string toLower(const std::string& s) {
    std::string r = s;
    for (size_t i = 0; i < r.size(); i++) {
        r[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(r[i])));
    }
    return r;
}

int countLines(const std::string& s) {
    if (s.empty()) return 0;
    int lines = 0;
    for (size_t i = 0; i < s.size(); i++) if (s[i] == '\n') lines++;
    if (s[s.size() - 1] != '\n') lines++;
    return lines;
}

int countWords(const std::string& s) {
    int words = 0;
    bool inWord = false;
    for (size_t i = 0; i < s.size(); i++) {
        bool space = std::isspace(static_cast<unsigned char>(s[i])) != 0;
        if (!space && !inWord) words++;
        inWord = !space;
    }
    return words;
}

// ---- counts files, folders, bytes, locked and encrypted items ----
class StatsVisitor : public NodeVisitor {
public:
    TreeStats st;
    void visitFile(const File& f, const std::string&) override {
        st.files++;
        st.bytes += f.getSize();
        if (f.isLocked()) st.locked++;
        if (f.isEncrypted()) st.encrypted++;
    }
    void visitDirectory(const Directory& d, const std::string&) override {
        st.dirs++;
        if (d.isLocked()) st.locked++;
    }
};

// ---- find: names containing the text (any letter case) ----
class FindVisitor : public NodeVisitor {
private:
    std::string pattern;
public:
    int count;
    explicit FindVisitor(const std::string& p) : pattern(toLower(p)), count(0) {}
    void visitFile(const File& f, const std::string& path) override {
        if (toLower(f.getName()).find(pattern) != std::string::npos) {
            std::cout << "  [F] " << path << "\n";
            count++;
        }
    }
    void visitDirectory(const Directory& d, const std::string& path) override {
        if (path == "/") return;   // the root has no real name
        if (toLower(d.getName()).find(pattern) != std::string::npos) {
            std::cout << "  [D] " << path << "/\n";
            count++;
        }
    }
};

// ---- grep: lines containing the word ----
class GrepVisitor : public NodeVisitor {
private:
    std::string word;
public:
    int matches;
    int skipped;   // encrypted files cannot be searched
    explicit GrepVisitor(const std::string& w) : word(toLower(w)), matches(0), skipped(0) {}

    void visitFile(const File& f, const std::string& path) override {
        if (f.isEncrypted()) { skipped++; return; }
        const std::string& text = f.getContent();
        size_t start = 0;
        int lineNo = 1;
        while (start <= text.size()) {
            size_t end = text.find('\n', start);
            if (end == std::string::npos) end = text.size();
            std::string line = text.substr(start, end - start);
            if (!line.empty() && toLower(line).find(word) != std::string::npos) {
                std::cout << "  " << path << ":" << lineNo << ": " << line << "\n";
                matches++;
            }
            if (end >= text.size()) break;
            start = end + 1;
            lineNo++;
        }
    }
};

} // namespace

TreeStats FileTools::collect(const FileSystem& fs, const FSNode* start) {
    StatsVisitor visitor;
    start->accept(visitor, fs.pathOf(start));
    if (start->isDirectory()) visitor.st.dirs--;   // do not count the starting folder itself
    return visitor.st;
}

Status FileTools::tree(const FileSystem& fs, const std::string& path) {
    FSNode* node = fs.resolve(path);
    if (node == nullptr) return Status::NOT_FOUND;

    if (!node->isDirectory()) {
        std::cout << node->getLabel() << "\n";
        return Status::OK;
    }
    const Directory* dir = static_cast<const Directory*>(node);
    std::cout << fs.pathOf(node) << "\n";
    dir->printTree("");

    TreeStats st = collect(fs, node);
    std::cout << "\n" << st.dirs << " folder(s), " << st.files << " file(s)\n";
    return Status::OK;
}

Status FileTools::info(const FileSystem& fs, const std::string& path) {
    FSNode* node = fs.resolve(path);
    if (node == nullptr) return Status::NOT_FOUND;

    std::cout << "Name      : " << (node->getParent() == nullptr ? "/" : node->getName()) << "\n";
    std::cout << "Path      : " << fs.pathOf(node) << "\n";
    std::cout << "Type      : " << (node->isDirectory() ? "folder" : "file") << "\n";
    std::cout << "Size      : " << node->getSize() << " bytes\n";

    if (node->isDirectory()) {
        const Directory* dir = static_cast<const Directory*>(node);
        TreeStats st = collect(fs, node);
        std::cout << "Contains  : " << dir->getCount() << " item(s) directly, "
                  << st.files << " file(s) and " << st.dirs << " folder(s) in total\n";
    } else {
        const File* f = static_cast<const File*>(node);
        if (f->isEncrypted()) {
            std::cout << "Text      : unreadable (encrypted)\n";
        } else {
            std::cout << "Text      : " << countLines(f->getContent()) << " line(s), "
                      << countWords(f->getContent()) << " word(s)\n";
        }
        std::cout << "Encrypted : " << (f->isEncrypted() ? "yes" : "no") << "\n";
    }
    std::cout << "Locked    : " << (node->isLocked() ? "yes" : "no") << "\n";
    return Status::OK;
}

Status FileTools::du(const FileSystem& fs, const std::string& path) {
    FSNode* node = fs.resolve(path);
    if (node == nullptr) return Status::NOT_FOUND;

    TreeStats st = collect(fs, node);
    std::cout << st.bytes << " bytes   " << fs.pathOf(node)
              << "   (" << st.files << " file(s), " << st.dirs << " folder(s))\n";
    return Status::OK;
}

Status FileTools::find(const FileSystem& fs, const std::string& text) {
    FindVisitor visitor(text);
    std::cout << "Searching the whole file system for '" << text << "':\n";
    fs.getRoot()->accept(visitor, "/");
    if (visitor.count == 0) std::cout << "  nothing found.\n";
    else std::cout << visitor.count << " match(es).\n";
    return Status::OK;
}

Status FileTools::grep(const FileSystem& fs, const std::string& word, const std::string& path) {
    FSNode* node = fs.resolve(path);
    if (node == nullptr) return Status::NOT_FOUND;

    GrepVisitor visitor(word);
    node->accept(visitor, fs.pathOf(node));
    if (visitor.matches == 0) std::cout << "No lines contain '" << word << "'.\n";
    else std::cout << visitor.matches << " matching line(s).\n";
    if (visitor.skipped > 0) std::cout << "(" << visitor.skipped << " encrypted file(s) skipped)\n";
    return Status::OK;
}

Status FileTools::wc(const FileSystem& fs, const std::string& path) {
    FSNode* node = fs.resolve(path);
    if (node == nullptr) return Status::NOT_FOUND;
    if (node->isDirectory()) return Status::NOT_A_FILE;

    const File* f = static_cast<const File*>(node);
    if (f->isEncrypted()) return Status::ENCRYPTED;

    std::cout << "  " << countLines(f->getContent()) << " line(s)  "
              << countWords(f->getContent()) << " word(s)  "
              << f->getContent().size() << " character(s)  " << fs.pathOf(node) << "\n";
    return Status::OK;
}
