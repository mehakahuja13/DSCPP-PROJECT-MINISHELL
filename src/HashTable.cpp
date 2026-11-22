/* Implements hash-table storage and the links that track which files contain each key. */

#include "HashTable.h"

HashTable::HashTable() {
    buckets = new Entry*[BUCKETS];
    for (int i = 0; i < BUCKETS; i++) buckets[i] = nullptr;
}

HashTable::~HashTable() {
    clear();
    delete[] buckets;
}

void HashTable::clear() {
    for (int i = 0; i < BUCKETS; i++) {
        Entry* e = buckets[i];
        while (e != nullptr) {
            Entry* nextEntry = e->next;
            OwnerNode* o = e->owners;
            while (o != nullptr) {
                OwnerNode* nextOwner = o->next;
                delete o;
                o = nextOwner;
            }
            delete e;
            e = nextEntry;
        }
        buckets[i] = nullptr;
    }
}

unsigned int HashTable::hashOf(const std::string& key) const {
    unsigned int h = 5381;
    for (size_t i = 0; i < key.size(); i++) {
        h = h * 33 + static_cast<unsigned char>(key[i]);
    }
    return h % BUCKETS;
}

HashTable::Entry* HashTable::find(const std::string& key) const {
    for (Entry* e = buckets[hashOf(key)]; e != nullptr; e = e->next) {
        if (e->key == key) return e;
    }
    return nullptr;
}

void HashTable::add(const std::string& key, int ownerId) {
    Entry* e = find(key);
    if (e == nullptr) {
        unsigned int b = hashOf(key);
        e = new Entry(key, buckets[b]);
        buckets[b] = e;
    }
    for (OwnerNode* o = e->owners; o != nullptr; o = o->next) {
        if (o->id == ownerId) return;
    }
    e->owners = new OwnerNode(ownerId, e->owners);
}

bool HashTable::contains(const std::string& key) const {
    return find(key) != nullptr;
}

const HashTable::OwnerNode* HashTable::ownersOf(const std::string& key) const {
    Entry* e = find(key);
    return (e == nullptr) ? nullptr : e->owners;
}

void HashTable::removeOwner(int ownerId) {
    for (int i = 0; i < BUCKETS; i++) {
        Entry* prevEntry = nullptr;
        Entry* e = buckets[i];
        while (e != nullptr) {

            OwnerNode* prevOwner = nullptr;
            OwnerNode* o = e->owners;
            while (o != nullptr) {
                if (o->id == ownerId) {
                    OwnerNode* dead = o;
                    o = o->next;
                    if (prevOwner == nullptr) e->owners = o;
                    else prevOwner->next = o;
                    delete dead;
                } else {
                    prevOwner = o;
                    o = o->next;
                }
            }

            if (e->owners == nullptr) {
                Entry* dead = e;
                e = e->next;
                if (prevEntry == nullptr) buckets[i] = e;
                else prevEntry->next = e;
                delete dead;
            } else {
                prevEntry = e;
                e = e->next;
            }
        }
    }
}
