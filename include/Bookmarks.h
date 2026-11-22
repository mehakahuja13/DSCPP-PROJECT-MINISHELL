/* Stores named paths so a user can jump back to frequently used locations. */

#ifndef BOOKMARKS_H
#define BOOKMARKS_H

#include <string>

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

    bool set(const std::string& name, const std::string& path);
    bool get(const std::string& name, std::string& path) const;
    bool remove(const std::string& name);
    void clear();
    int  size() const { return count; }

    bool entryAt(int index, std::string& name, std::string& path) const;
};

#endif
