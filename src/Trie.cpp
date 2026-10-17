#include "Trie.h"

Trie::Node::Node() : ids(nullptr) {
    for (int i = 0; i < ALPHABET; i++) child[i] = nullptr;
}

Trie::Trie() : root(new Node()) {}

Trie::~Trie() {
    destroy(root);
}

void Trie::destroy(Node* n) {
    if (n == nullptr) return;
    for (int i = 0; i < ALPHABET; i++) destroy(n->child[i]);
    IdNode* cur = n->ids;
    while (cur != nullptr) {
        IdNode* nextId = cur->next;
        delete cur;
        cur = nextId;
    }
    delete n;
}

// Adds the id to a node's list unless it is already there.
void Trie::addId(Node* n, int id) {
    for (IdNode* cur = n->ids; cur != nullptr; cur = cur->next) {
        if (cur->id == id) return;
    }
    n->ids = new IdNode(id, n->ids);
}

void Trie::insert(const std::string& word, int id) {
    Node* cur = root;
    addId(cur, id);                       // the empty prefix matches every item
    for (size_t i = 0; i < word.size(); i++) {
        unsigned char c = static_cast<unsigned char>(word[i]);
        if (c >= ALPHABET) return;        // only plain ASCII words are supported
        if (cur->child[c] == nullptr) cur->child[c] = new Node();
        cur = cur->child[c];
        addId(cur, id);
    }
}

const Trie::IdNode* Trie::find(const std::string& prefix) const {
    const Node* cur = root;
    for (size_t i = 0; i < prefix.size(); i++) {
        unsigned char c = static_cast<unsigned char>(prefix[i]);
        if (c >= ALPHABET || cur->child[c] == nullptr) return nullptr;
        cur = cur->child[c];
    }
    return cur->ids;
}
