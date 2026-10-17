#ifndef FILE_H
#define FILE_H

#include "FSNode.h"

// A file node. Content is private (encapsulation).
class File : public FSNode {
private:
    static int         nextId;     // gives every file a unique id
    int                id;
    std::string        content;    // plain text, or scrambled text when encrypted
    bool               encrypted;
    unsigned long long passHash;   // hash of the password (the password itself is never stored)

    static unsigned long long hashPassword(const std::string& password);
    static std::string        applyCipher(const std::string& text, const std::string& password);

public:
    File(const std::string& n, Directory* p = nullptr);

    bool        isDirectory() const override { return false; }
    void        display() const override;
    std::string getLabel() const override;
    size_t      getSize() const override { return content.size(); }
    FSNode*     clone() const override;
    bool        containsLocked() const override { return locked; }
    void        accept(NodeVisitor& visitor, const std::string& path) const override;
    void        unregisterFrom(PlagiarismChecker& checker) const override;
    void        registerWith(PlagiarismChecker& checker, const std::string& dirPath) const override;

    int                getId() const { return id; }
    const std::string& getContent() const { return content; }
    void               setContent(const std::string& c) { content = c; }

    // ---- encryption (simple XOR cipher: for learning, not real security) ----
    bool               isEncrypted() const { return encrypted; }
    unsigned long long getPassHash() const { return passHash; }
    void               encrypt(const std::string& password);
    bool               decrypt(const std::string& password);                    // false = wrong password
    bool               peek(const std::string& password, std::string& out) const; // read without decrypting
    void               restoreEncrypted(const std::string& cipherText, unsigned long long hash);  // used by load
};

#endif
