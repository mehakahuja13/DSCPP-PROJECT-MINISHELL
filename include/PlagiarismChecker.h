#ifndef PLAGIARISMCHECKER_H
#define PLAGIARISMCHECKER_H

#include <string>
#include "HashTable.h"

// Result of comparing new content with the files already saved.
struct PlagiarismResult {
    int         percent;        // 0..100
    std::string matchedPath;    // file with the highest match (empty if percent == 0)
    PlagiarismResult() : percent(0) {}
};

// How it works:
//  1. Text is normalised (lower case, punctuation/extra spaces removed).
//  2. It is cut into overlapping groups of 3 words ("shingles").
//     "a b c d e"  ->  "a b c", "b c d", "c d e"
//  3. Every saved file's shingles are stored in a HashTable (shingle -> files).
//  4. For new content, each shingle is looked up; the percentage is
//        (shingles also found in file F) * 100 / (distinct shingles of new content)
//     and the file with the highest percentage is reported (containment).
//  Content shorter than 3 words is treated as ONE shingle (the whole text).
class PlagiarismChecker {
private:
    struct FileRecord {             // linked list: file id -> path (for display)
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
    // Warn the user (and ask before saving) when percent >= THRESHOLD.
    enum { THRESHOLD = 50 };

    PlagiarismChecker();
    ~PlagiarismChecker();

    PlagiarismChecker(const PlagiarismChecker&) = delete;
    PlagiarismChecker& operator=(const PlagiarismChecker&) = delete;

    // Compare 'text' with every indexed file except the one with id 'selfId'
    // (so a file never counts as a copy of its own old version).
    PlagiarismResult check(const std::string& text, int selfId) const;

    void indexFile(int id, const std::string& path, const std::string& text);  // replaces old entry
    void removeFile(int id);
    void clear();   // forget every file (used after load)

    // How much of text 'a' (in % of its 3-word groups) also appears in text 'b'.
    static int similarity(const std::string& a, const std::string& b);
};

#endif
