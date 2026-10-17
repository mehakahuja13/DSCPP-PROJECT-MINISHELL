#include "Persistence.h"
#include <fstream>
#include <sstream>

namespace {

const char* HEADER = "MINISHELL-DATA 1";

// Content is stored as hex so newlines and any byte survive in a one-line record.
std::string toHex(const std::string& s) {
    if (s.empty()) return "-";
    const char* digits = "0123456789abcdef";
    std::string out;
    for (size_t i = 0; i < s.size(); i++) {
        unsigned char c = static_cast<unsigned char>(s[i]);
        out += digits[c >> 4];
        out += digits[c & 15];
    }
    return out;
}

int hexValue(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

bool fromHex(const std::string& hex, std::string& out) {
    out.clear();
    if (hex == "-") return true;
    if (hex.size() % 2 != 0) return false;
    for (size_t i = 0; i < hex.size(); i += 2) {
        int hi = hexValue(hex[i]), lo = hexValue(hex[i + 1]);
        if (hi < 0 || lo < 0) return false;
        out += static_cast<char>(hi * 16 + lo);
    }
    return true;
}

// Writes one line for every folder and file while walking the tree.
class SaveVisitor : public NodeVisitor {
private:
    std::ofstream& out;
public:
    explicit SaveVisitor(std::ofstream& o) : out(o) {}
    void visitDirectory(const Directory& d, const std::string& path) override {
        if (path == "/") return;   // the root always exists
        out << "D " << path << " " << (d.isLocked() ? 1 : 0) << "\n";
    }
    void visitFile(const File& f, const std::string& path) override {
        out << "F " << path << " " << (f.isLocked() ? 1 : 0) << " "
            << (f.isEncrypted() ? 1 : 0) << " " << f.getPassHash() << " "
            << toHex(f.getContent()) << "\n";
    }
};

} // namespace

Status Persistence::save(const FileSystem& fs, const std::string& filename) {
    std::ofstream out(filename.c_str());
    if (!out) return Status::IO_ERROR;

    out << HEADER << "\n";
    out << "CWD " << fs.pwd() << "\n";

    SaveVisitor visitor(out);
    fs.getRoot()->accept(visitor, "/");

    std::string name, path;
    for (int i = 0; fs.getBookmarks().entryAt(i, name, path); i++) {
        out << "B " << name << " " << path << "\n";
    }
    out << "END\n";
    out.flush();
    return out.good() ? Status::OK : Status::IO_ERROR;
}

Status Persistence::load(FileSystem& fs, const std::string& filename) {
    std::ifstream in(filename.c_str());
    if (!in) return Status::IO_ERROR;

    std::string line;
    if (!std::getline(in, line) || line != HEADER) return Status::BAD_FORMAT;

    FileSystem temp;              // everything is built here first
    std::string cwdPath = "/";
    bool sawEnd = false;

    while (std::getline(in, line)) {
        if (line.empty()) continue;
        std::istringstream parts(line);
        std::string kind;
        parts >> kind;

        if (kind == "CWD") {
            parts >> cwdPath;
        } else if (kind == "D") {
            std::string path;
            int lockedFlag = 0;
            if (!(parts >> path >> lockedFlag)) return Status::BAD_FORMAT;
            if (temp.mkdir(path) != Status::OK) return Status::BAD_FORMAT;
            bool changed = false;
            if (lockedFlag) temp.setLocked(path, true, changed);
        } else if (kind == "F") {
            std::string path, hex, content;
            int lockedFlag = 0, encryptedFlag = 0;
            unsigned long long hash = 0;
            if (!(parts >> path >> lockedFlag >> encryptedFlag >> hash >> hex)) return Status::BAD_FORMAT;
            if (!fromHex(hex, content)) return Status::BAD_FORMAT;
            if (temp.loadFile(path, content, lockedFlag != 0, encryptedFlag != 0, hash) != Status::OK) {
                return Status::BAD_FORMAT;
            }
        } else if (kind == "B") {
            std::string name, path;
            if (!(parts >> name >> path)) return Status::BAD_FORMAT;
            temp.getBookmarks().set(name, path);
        } else if (kind == "END") {
            sawEnd = true;
            break;
        } else {
            return Status::BAD_FORMAT;
        }
    }
    if (!sawEnd) return Status::BAD_FORMAT;

    temp.cd(cwdPath);   // if the folder no longer exists we simply stay in the root
    fs.adopt(temp);
    return Status::OK;
}
