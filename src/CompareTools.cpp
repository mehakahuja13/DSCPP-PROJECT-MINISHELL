#include "CompareTools.h"
#include <iostream>

namespace {

// Gets a readable file or returns the reason why not.
Status readableFile(const FileSystem& fs, const std::string& path, const File*& out) {
    FSNode* node = fs.resolve(path);
    if (node == nullptr) return Status::NOT_FOUND;
    if (node->isDirectory()) return Status::NOT_A_FILE;
    const File* f = static_cast<const File*>(node);
    if (f->isEncrypted()) return Status::ENCRYPTED;
    out = f;
    return Status::OK;
}

// ---- collects every readable, non-empty file for 'scan' ----
class CollectVisitor : public NodeVisitor {
public:
    struct Item {
        std::string path;
        std::string content;
        Item*       next;
        Item(const std::string& p, const std::string& c, Item* n) : path(p), content(c), next(n) {}
    };
    Item* head;
    int   skippedEncrypted;

    CollectVisitor() : head(nullptr), skippedEncrypted(0) {}
    ~CollectVisitor() {
        while (head != nullptr) { Item* nx = head->next; delete head; head = nx; }
    }
    void visitFile(const File& f, const std::string& path) override {
        if (f.isEncrypted()) { skippedEncrypted++; return; }
        if (f.getContent().empty()) return;
        head = new Item(path, f.getContent(), head);
    }
};

// Splits text into lines (a final newline does not create an empty extra line).
int splitLines(const std::string& text, std::string*& lines) {
    int count = 0;
    for (size_t i = 0; i < text.size(); i++) if (text[i] == '\n') count++;
    if (!text.empty() && text[text.size() - 1] != '\n') count++;

    lines = (count > 0) ? new std::string[count] : nullptr;
    size_t start = 0;
    for (int k = 0; k < count; k++) {
        size_t end = text.find('\n', start);
        if (end == std::string::npos) end = text.size();
        lines[k] = text.substr(start, end - start);
        start = end + 1;
    }
    return count;
}

} // namespace

Status CompareTools::compare(const FileSystem& fs, const std::string& a, const std::string& b) {
    const File* fa = nullptr;
    const File* fb = nullptr;
    Status s = readableFile(fs, a, fa);
    if (s != Status::OK) return s;
    s = readableFile(fs, b, fb);
    if (s != Status::OK) return s;

    int aInB = PlagiarismChecker::similarity(fa->getContent(), fb->getContent());
    int bInA = PlagiarismChecker::similarity(fb->getContent(), fa->getContent());
    std::string pa = fs.pathOf(fa), pb = fs.pathOf(fb);

    std::cout << "Comparing " << pa << " and " << pb << ":\n";
    std::cout << "  " << aInB << "% of " << pa << " also appears in " << pb << "\n";
    std::cout << "  " << bInA << "% of " << pb << " also appears in " << pa << "\n";

    int worst = (aInB > bInA) ? aInB : bInA;
    if (worst >= PlagiarismChecker::THRESHOLD) std::cout << "Result: high similarity - possible plagiarism.\n";
    else std::cout << "Result: the files look different enough.\n";
    return Status::OK;
}

Status CompareTools::scan(const FileSystem& fs) {
    CollectVisitor collected;
    fs.getRoot()->accept(collected, "/");

    int files = 0;
    for (CollectVisitor::Item* i = collected.head; i != nullptr; i = i->next) files++;
    if (files < 2) {
        std::cout << "Need at least 2 readable, non-empty files to scan.\n";
        return Status::OK;
    }

    std::cout << "Scanning " << files << " file(s) for similar content...\n";
    int suspicious = 0;
    for (CollectVisitor::Item* x = collected.head; x != nullptr; x = x->next) {
        for (CollectVisitor::Item* y = x->next; y != nullptr; y = y->next) {
            int xInY = PlagiarismChecker::similarity(x->content, y->content);
            int yInX = PlagiarismChecker::similarity(y->content, x->content);
            if (xInY >= PlagiarismChecker::THRESHOLD || yInX >= PlagiarismChecker::THRESHOLD) {
                std::cout << "  " << x->path << "  <->  " << y->path
                          << "   (" << xInY << "% / " << yInX << "%)\n";
                suspicious++;
            }
        }
    }
    if (suspicious == 0) std::cout << "No suspicious pairs found.\n";
    else std::cout << suspicious << " suspicious pair(s) (similarity of "
                   << static_cast<int>(PlagiarismChecker::THRESHOLD) << "% or more).\n";
    if (collected.skippedEncrypted > 0) {
        std::cout << "(" << collected.skippedEncrypted << " encrypted file(s) skipped)\n";
    }
    return Status::OK;
}

Status CompareTools::diff(const FileSystem& fs, const std::string& a, const std::string& b) {
    const File* fa = nullptr;
    const File* fb = nullptr;
    Status s = readableFile(fs, a, fa);
    if (s != Status::OK) return s;
    s = readableFile(fs, b, fb);
    if (s != Status::OK) return s;

    const int LIMIT = 300;   // keeps the LCS table small
    std::string* x = nullptr;
    std::string* y = nullptr;
    int n = splitLines(fa->getContent(), x);
    int m = splitLines(fb->getContent(), y);
    if (n > LIMIT || m > LIMIT) {
        delete[] x;
        delete[] y;
        return Status::TOO_LARGE;
    }

    // lcs[i][j] = length of the longest common subsequence of x[i..] and y[j..]
    int cols = m + 1;
    int* lcs = new int[(n + 1) * cols];
    for (int i = 0; i <= n; i++) lcs[i * cols + m] = 0;
    for (int j = 0; j <= m; j++) lcs[n * cols + j] = 0;
    for (int i = n - 1; i >= 0; i--) {
        for (int j = m - 1; j >= 0; j--) {
            if (x[i] == y[j]) lcs[i * cols + j] = lcs[(i + 1) * cols + (j + 1)] + 1;
            else {
                int down  = lcs[(i + 1) * cols + j];
                int right = lcs[i * cols + (j + 1)];
                lcs[i * cols + j] = (down >= right) ? down : right;
            }
        }
    }

    std::cout << "--- " << fs.pathOf(fa) << "\n+++ " << fs.pathOf(fb) << "\n";
    int i = 0, j = 0, removed = 0, added = 0;
    while (i < n || j < m) {
        if (i < n && j < m && x[i] == y[j]) {
            std::cout << "  " << x[i] << "\n";
            i++; j++;
        } else if (j >= m || (i < n && lcs[(i + 1) * cols + j] >= lcs[i * cols + (j + 1)])) {
            std::cout << "- " << x[i] << "\n";
            i++; removed++;
        } else {
            std::cout << "+ " << y[j] << "\n";
            j++; added++;
        }
    }
    if (removed == 0 && added == 0) std::cout << "The files are identical.\n";
    else std::cout << removed << " line(s) only in the first file, " << added << " line(s) only in the second.\n";

    delete[] lcs;
    delete[] x;
    delete[] y;
    return Status::OK;
}
