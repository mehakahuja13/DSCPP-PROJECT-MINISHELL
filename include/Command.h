/* Defines the common interface used by commands that support execution and undo. */

#ifndef COMMAND_H
#define COMMAND_H

#include <string>
#include "FileSystem.h"

class Command {
public:
    Command() {}
    virtual ~Command() {}

    Command(const Command&) = delete;
    Command& operator=(const Command&) = delete;

    virtual Status      execute() = 0;
    virtual bool        undo() = 0;
    virtual bool        isApplied() const = 0;
    virtual std::string describe() const = 0;
};

#endif
