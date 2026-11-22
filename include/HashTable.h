/* Defines the hash table used to index text shingles and their owning files. */

#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <string>

class HashTable {
public:
    struct OwnerNode {
        int        id;
        OwnerNode* next;
        OwnerNode(int i, OwnerNode* n) : id(i), next(n) {}
    };

private:
    struct Entry {
        std::string key;
        OwnerNode*  owners;
        Entry*      next;
        Entry(const std::string& k, Entry* n) : key(k), owners(nullptr), next(n) {}
    };

    enum { BUCKETS = 257 };
    Entry** buckets;

    unsigned int hashOf(const std::string& key) const;
    Entry*       find(const std::string& key) const;

public:
    HashTable();
    ~HashTable();

    HashTable(const HashTable&) = delete;
    HashTable& operator=(const HashTable&) = delete;

    void add(const std::string& key, int ownerId);
    bool contains(const std::string& key) const;
    const OwnerNode* ownersOf(const std::string& key) const;
    void removeOwner(int ownerId);
    void clear();
};

#endif
