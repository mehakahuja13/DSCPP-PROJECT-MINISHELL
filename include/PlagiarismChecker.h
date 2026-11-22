/* Declares the shingle-based checker used to find similar file contents. */

#ifndef PLAGIARISMCHECKER_H
#define PLAGIARISMCHECKER_H

#include <string>
#include "HashTable.h"

struct PlagiarismResult {
    int         percent;
    std::string matchedPath;
    PlagiarismResult() : percent(0) {}
};

class PlagiarismChecker {
private:
    struct FileRecord {
        int         id;
        std::string path;
        FileRecord* next;
        FileRecord(int i, const std::string& p, FileRecord* n) : id(i), path(p), next(n) {}
    };

    HashTable   index;
    FileRecord* records;

    std::string normalize(const std::string& text) const;
    int         splitWords(const std::string& normalized, std::string*& words) const;
    int         buildShingles(const std::string& text, std::string*& shingles) const;
    const FileRecord* findRecord(int id) const;

public:

    enum { THRESHOLD = 50 };

    PlagiarismChecker();
    ~PlagiarismChecker();

    PlagiarismChecker(const PlagiarismChecker&) = delete;
    PlagiarismChecker& operator=(const PlagiarismChecker&) = delete;

    PlagiarismResult check(const std::string& text, int selfId) const;

    void indexFile(int id, const std::string& path, const std::string& text);
    void removeFile(int id);
    void clear();

    static int similarity(const std::string& a, const std::string& b);
};

#endif
