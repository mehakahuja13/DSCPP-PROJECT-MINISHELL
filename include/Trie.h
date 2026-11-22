/* Defines a trie for storing prefixes and the file IDs associated with them. */

#ifndef TRIE_H
#define TRIE_H

#include <string>

class Trie {
public:
    struct IdNode {
        int     id;
        IdNode* next;
        IdNode(int i, IdNode* n) : id(i), next(n) {}
    };

private:
    enum { ALPHABET = 128 };

    struct Node {
        Node*   child[ALPHABET];
        IdNode* ids;
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

    void insert(const std::string& word, int id);

    const IdNode* find(const std::string& prefix) const;
};

#endif
