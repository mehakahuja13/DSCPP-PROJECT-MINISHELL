/* Declares a file node, including its content and lock/encryption state. */

#ifndef FILE_H
#define FILE_H

#include "FSNode.h"

class File : public FSNode {
private:
    static int         nextId;
    int                id;
    std::string        content;
    bool               encrypted;
    unsigned long long passHash;

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

    bool               isEncrypted() const { return encrypted; }
    unsigned long long getPassHash() const { return passHash; }
    void               encrypt(const std::string& password);
    bool               decrypt(const std::string& password);
    bool               peek(const std::string& password, std::string& out) const;
    void               restoreEncrypted(const std::string& cipherText, unsigned long long hash);
};

#endif
