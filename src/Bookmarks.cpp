#include "Bookmarks.h"

Bookmarks::Bookmarks() : count(0) {
    buckets = new Entry*[BUCKETS];
    for (int i = 0; i < BUCKETS; i++) buckets[i] = nullptr;
}

Bookmarks::~Bookmarks() {
    clear();
    delete[] buckets;
}

unsigned int Bookmarks::hashOf(const std::string& name) const {
    unsigned int h = 5381;
    for (size_t i = 0; i < name.size(); i++) h = h * 33 + static_cast<unsigned char>(name[i]);
    return h % BUCKETS;
}

bool Bookmarks::set(const std::string& name, const std::string& path) {
    unsigned int b = hashOf(name);
    for (Entry* e = buckets[b]; e != nullptr; e = e->next) {
        if (e->name == name) { e->path = path; return false; }   // replace existing
    }
    buckets[b] = new Entry(name, path, buckets[b]);
    count++;
    return true;
}

bool Bookmarks::get(const std::string& name, std::string& path) const {
    for (Entry* e = buckets[hashOf(name)]; e != nullptr; e = e->next) {
        if (e->name == name) { path = e->path; return true; }
    }
    return false;
}

bool Bookmarks::remove(const std::string& name) {
    unsigned int b = hashOf(name);
    Entry* prev = nullptr;
    for (Entry* e = buckets[b]; e != nullptr; prev = e, e = e->next) {
        if (e->name == name) {
            if (prev == nullptr) buckets[b] = e->next;
            else prev->next = e->next;
            delete e;
            count--;
            return true;
        }
    }
    return false;
}

void Bookmarks::clear() {
    for (int i = 0; i < BUCKETS; i++) {
        Entry* e = buckets[i];
        while (e != nullptr) {
            Entry* nextEntry = e->next;
            delete e;
            e = nextEntry;
        }
        buckets[i] = nullptr;
    }
    count = 0;
}

bool Bookmarks::entryAt(int index, std::string& name, std::string& path) const {
    int seen = 0;
    for (int i = 0; i < BUCKETS; i++) {
        for (Entry* e = buckets[i]; e != nullptr; e = e->next) {
            if (seen == index) { name = e->name; path = e->path; return true; }
            seen++;
        }
    }
    return false;
}
