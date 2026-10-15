#include "PlagiarismChecker.h"
#include <cctype>

PlagiarismChecker::PlagiarismChecker() : records(nullptr) {}

PlagiarismChecker::~PlagiarismChecker() {
    while (records != nullptr) {
        FileRecord* nextRec = records->next;
        delete records;
        records = nextRec;
    }
}

// Lower-case; every run of non-letter/digit characters becomes a single space.
// Bytes >= 128 (non-English text) are kept as they are.
std::string PlagiarismChecker::normalize(const std::string& text) const {
    std::string out;
    bool pendingSpace = false;
    for (size_t i = 0; i < text.size(); i++) {
        unsigned char c = static_cast<unsigned char>(text[i]);
        if (c >= 128 || std::isalnum(c)) {
            if (pendingSpace && !out.empty()) out += ' ';
            pendingSpace = false;
            out += (c < 128) ? static_cast<char>(std::tolower(c)) : static_cast<char>(c);
        } else {
            pendingSpace = true;
        }
    }
    return out;
}

// Splits a normalised string on single spaces. Caller must delete[] 'words'.
int PlagiarismChecker::splitWords(const std::string& normalized, std::string*& words) const {
    if (normalized.empty()) { words = nullptr; return 0; }

    int count = 1;
    for (size_t i = 0; i < normalized.size(); i++) {
        if (normalized[i] == ' ') count++;
    }
    words = new std::string[count];

    int w = 0;
    size_t start = 0;
    for (size_t i = 0; i <= normalized.size(); i++) {
        if (i == normalized.size() || normalized[i] == ' ') {
            words[w++] = normalized.substr(start, i - start);
            start = i + 1;
        }
    }
    return count;
}

// Builds the 3-word groups. Caller must delete[] 'shingles'.
int PlagiarismChecker::buildShingles(const std::string& text, std::string*& shingles) const {
    std::string norm = normalize(text);
    if (norm.empty()) { shingles = nullptr; return 0; }

    std::string* words = nullptr;
    int w = splitWords(norm, words);

    int n;
    if (w < 3) {
        n = 1;
        shingles = new std::string[1];
        shingles[0] = norm;                     // very short text: the whole text is one shingle
    } else {
        n = w - 2;
        shingles = new std::string[n];
        for (int i = 0; i < n; i++) {
            shingles[i] = words[i] + " " + words[i + 1] + " " + words[i + 2];
        }
    }
    delete[] words;
    return n;
}

const PlagiarismChecker::FileRecord* PlagiarismChecker::findRecord(int id) const {
    for (const FileRecord* r = records; r != nullptr; r = r->next) {
        if (r->id == id) return r;
    }
    return nullptr;
}

PlagiarismResult PlagiarismChecker::check(const std::string& text, int selfId) const {
    PlagiarismResult result;

    std::string* shingles = nullptr;
    int n = buildShingles(text, shingles);
    if (n == 0) return result;

    // Running score per file: how many of the new shingles it also contains.
    struct Score {
        int   id;
        int   count;
        Score* next;
        Score(int i, Score* nx) : id(i), count(0), next(nx) {}
    };
    Score* scores = nullptr;

    HashTable seen;          // used to count each distinct shingle only once
    int distinct = 0;

    for (int i = 0; i < n; i++) {
        if (seen.contains(shingles[i])) continue;
        seen.add(shingles[i], 1);
        distinct++;

        for (const HashTable::OwnerNode* o = index.ownersOf(shingles[i]); o != nullptr; o = o->next) {
            if (o->id == selfId) continue;

            Score* s = scores;
            while (s != nullptr && s->id != o->id) s = s->next;
            if (s == nullptr) {
                s = new Score(o->id, scores);
                scores = s;
            }
            s->count++;
        }
    }

    // Pick the file with the highest score.
    int bestId = -1, bestCount = 0;
    for (Score* s = scores; s != nullptr; s = s->next) {
        if (s->count > bestCount) {
            bestCount = s->count;
            bestId = s->id;
        }
    }

    if (bestId != -1) {
        result.percent = bestCount * 100 / distinct;
        const FileRecord* rec = findRecord(bestId);
        if (rec != nullptr) result.matchedPath = rec->path;
    }

    while (scores != nullptr) {
        Score* nextScore = scores->next;
        delete scores;
        scores = nextScore;
    }
    delete[] shingles;
    return result;
}

void PlagiarismChecker::indexFile(int id, const std::string& path, const std::string& text) {
    removeFile(id);   // forget the old version of this file first

    std::string* shingles = nullptr;
    int n = buildShingles(text, shingles);
    if (n == 0) return;

    records = new FileRecord(id, path, records);
    for (int i = 0; i < n; i++) index.add(shingles[i], id);
    delete[] shingles;
}

void PlagiarismChecker::removeFile(int id) {
    index.removeOwner(id);

    FileRecord* prev = nullptr;
    FileRecord* cur = records;
    while (cur != nullptr) {
        if (cur->id == id) {
            if (prev == nullptr) records = cur->next;
            else prev->next = cur->next;
            delete cur;
            return;
        }
        prev = cur;
        cur = cur->next;
    }
}
