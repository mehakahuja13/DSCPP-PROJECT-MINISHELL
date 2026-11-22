/* Declares the helper that checks command names and suggests likely corrections. */

#ifndef COMMANDCORRECTOR_H
#define COMMANDCORRECTOR_H

#include <string>

class CommandCorrector {
private:
    enum { MAX_LEN = 32 };

    int distance(const std::string& a, const std::string& b) const;
    std::string toLower(const std::string& s) const;

public:

    bool isValid(const std::string& cmd) const;

    bool suggest(const std::string& cmd, std::string& best) const;
};

#endif
