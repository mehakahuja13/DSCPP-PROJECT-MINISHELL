#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <string>

// Hand-written hash table with CHAINING.
//   key   : a string (here: a group of 3 words, called a "shingle")
//   value : the list of owner ids (files) that contain that key
//
// Because the real key string is stored and compared inside each bucket,
// two different keys with the same hash value can never be confused.
class HashTable {
public:
    struct OwnerNode {              // linked list of owners of one key
        int        id;
        OwnerNode* next;
        OwnerNode(int i, OwnerNode* n) : id(i), next(n) {}
    };

private:
    struct Entry {                  // one key inside a bucket chain
        std::string key;
        OwnerNode*  owners;
        Entry*      next;
        Entry(const std::string& k, Entry* n) : key(k), owners(nullptr), next(n) {}
    };

    enum { BUCKETS = 257 };         // prime number of buckets
    Entry** buckets;

    unsigned int hashOf(const std::string& key) const;
    Entry*       find(const std::string& key) const;

public:
    HashTable();
    ~HashTable();

    HashTable(const HashTable&) = delete;
    HashTable& operator=(const HashTable&) = delete;

    void add(const std::string& key, int ownerId);      // no duplicate owners
    bool contains(const std::string& key) const;
    const OwnerNode* ownersOf(const std::string& key) const;   // nullptr if key absent
    void removeOwner(int ownerId);                      // removes owner from every key
    void clear();                                       // forget everything
};

#endif
