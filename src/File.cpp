#include "File.h"
#include "PlagiarismChecker.h"
#include <iostream>

int File::nextId = 1;

File::File(const std::string& n, Directory* p)
    : FSNode(n, p), id(nextId++), encrypted(false), passHash(0) {}

// FNV-1a 64-bit hash.
unsigned long long File::hashPassword(const std::string& password) {
    unsigned long long h = 14695981039346656037ULL;
    for (size_t i = 0; i < password.size(); i++) {
        h ^= static_cast<unsigned char>(password[i]);
        h *= 1099511628211ULL;
    }
    return h;
}

// XOR every byte with a password byte and a position-dependent value.
// The same call both encrypts and decrypts.
std::string File::applyCipher(const std::string& text, const std::string& password) {
    std::string out = text;
    if (password.empty()) return out;
    for (size_t i = 0; i < out.size(); i++) {
        unsigned char key = static_cast<unsigned char>(password[i % password.size()]);
        unsigned char mix = static_cast<unsigned char>((i * 31 + 17) & 0xFF);
        out[i] = static_cast<char>(static_cast<unsigned char>(out[i]) ^ key ^ mix);
    }
    return out;
}

std::string File::getLabel() const {
    std::string label = name;
    if (locked)    label += " [locked]";
    if (encrypted) label += " [enc]";
    return label;
}

void File::display() const {
    std::cout << "[F] " << getLabel() << "  (" << content.size() << " bytes)\n";
}

// Copy used by 'cp': same content, but a NEW id and not locked.
FSNode* File::clone() const {
    File* copy = new File(name, nullptr);
    copy->content   = content;
    copy->encrypted = encrypted;
    copy->passHash  = passHash;
    return copy;
}

void File::accept(NodeVisitor& visitor, const std::string& path) const {
    visitor.visitFile(*this, path);
}

void File::unregisterFrom(PlagiarismChecker& checker) const {
    checker.removeFile(id);
}

// Encrypted files are never put in the plagiarism index.
void File::registerWith(PlagiarismChecker& checker, const std::string& dirPath) const {
    if (encrypted) return;
    checker.indexFile(id, joinPath(dirPath, name), content);
}

void File::encrypt(const std::string& password) {
    passHash  = hashPassword(password);
    content   = applyCipher(content, password);
    encrypted = true;
}

bool File::decrypt(const std::string& password) {
    if (!encrypted || hashPassword(password) != passHash) return false;
    content   = applyCipher(content, password);
    encrypted = false;
    passHash  = 0;
    return true;
}

bool File::peek(const std::string& password, std::string& out) const {
    if (!encrypted) { out = content; return true; }
    if (hashPassword(password) != passHash) return false;
    out = applyCipher(content, password);
    return true;
}

void File::restoreEncrypted(const std::string& cipherText, unsigned long long hash) {
    content   = cipherText;
    encrypted = true;
    passHash  = hash;
}
