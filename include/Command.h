#ifndef COMMAND_H
#define COMMAND_H

#include <string>
#include "FileSystem.h"

// Abstract base class of every change that can be undone (COMMAND design pattern).
// Each concrete command knows how to do its work AND how to reverse it.
class Command {
public:
    Command() {}
    virtual ~Command() {}

    Command(const Command&) = delete;
    Command& operator=(const Command&) = delete;

    virtual Status      execute() = 0;               // do it (also used for redo)
    virtual bool        undo() = 0;                  // reverse it; false if that is not possible now
    virtual bool        isApplied() const = 0;       // did execute() really change something?
    virtual std::string describe() const = 0;        // e.g. "mkdir /docs"
};

#endif
