#ifndef TRIE_H
#define TRIE_H

#include <string>

// Prefix tree (trie) used for menu auto-suggest.
// Every node on the path of a word remembers the ids of the items that word
// belongs to, so looking up a prefix is just "walk down the prefix" -- the
// time depends only on the length of the prefix, not on how many items exist.
class Trie {
public:
    struct IdNode {                 // linked list of item ids
        int     id;
        IdNode* next;
        IdNode(int i, IdNode* n) : id(i), next(n) {}
    };

private:
    enum { ALPHABET = 128 };        // plain ASCII characters

    struct Node {
        Node*   child[ALPHABET];
        IdNode* ids;                // items having a word that passes through this node
        Node();
    };

    Node* root;

    void destroy(Node* n);
    void addId(Node* n, int id);

public:
    Trie();
    ~Trie();

    Trie(const Trie&) = delete;
    Trie& operator=(const Trie&) = delete;

    // Stores 'word' (lower-case) and links it with item 'id'.
    void insert(const std::string& word, int id);

    // Items having a word that starts with 'prefix' (nullptr if none).
    const IdNode* find(const std::string& prefix) const;
};

#endif
