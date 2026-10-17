#ifndef BOOKMARKS_H
#define BOOKMARKS_H

#include <string>

// Folder shortcuts: name -> absolute path.
// A second hand-written HASH TABLE with chaining (the first one is in HashTable.h).
class Bookmarks {
private:
    struct Entry {
        std::string name;
        std::string path;
        Entry*      next;
        Entry(const std::string& n, const std::string& p, Entry* nx) : name(n), path(p), next(nx) {}
    };

    enum { BUCKETS = 31 };
    Entry** buckets;
    int     count;

    unsigned int hashOf(const std::string& name) const;

public:
    Bookmarks();
    ~Bookmarks();

    Bookmarks(const Bookmarks&) = delete;
    Bookmarks& operator=(const Bookmarks&) = delete;

    bool set(const std::string& name, const std::string& path);   // true if it is a new name
    bool get(const std::string& name, std::string& path) const;
    bool remove(const std::string& name);
    void clear();
    int  size() const { return count; }

    // Reads the index-th bookmark (0..size()-1); false when index is out of range.
    bool entryAt(int index, std::string& name, std::string& path) const;
};

#endif
