#ifndef COMMANDS_H
#define COMMANDS_H

#include "Command.h"

// All commands store ABSOLUTE paths, so undo/redo work no matter which
// directory the user is currently in.

class MkdirCommand : public Command {
private:
    FileSystem& fs;
    std::string path;
    bool        applied;
public:
    MkdirCommand(FileSystem& f, const std::string& absPath);
    Status      execute() override;
    bool        undo() override;
    bool        isApplied() const override { return applied; }
    std::string describe() const override;
};

class TouchCommand : public Command {
private:
    FileSystem& fs;
    std::string path;
    bool        applied;      // false when the file already existed (nothing to undo)
public:
    TouchCommand(FileSystem& f, const std::string& absPath);
    Status      execute() override;
    bool        undo() override;
    bool        isApplied() const override { return applied; }
    std::string describe() const override;
};

class WriteCommand : public Command {
private:
    FileSystem& fs;
    std::string path;
    std::string text;           // new content
    std::string oldContent;     // content before the write (restored on undo)
    bool        existedBefore;  // false -> undo deletes the file again
    bool        applied;
public:
    WriteCommand(FileSystem& f, const std::string& absPath, const std::string& newText);
    Status      execute() override;
    bool        undo() override;
    bool        isApplied() const override { return applied; }
    std::string describe() const override;
};

// Remove does not destroy the node: it detaches it from the tree and keeps it
// ('held'), so undo can simply attach it back. The node is only really freed
// when this command is destroyed while still holding it.
class RemoveCommand : public Command {
private:
    FileSystem& fs;
    std::string path;
    std::string parentPath;
    FSNode*     held;
    bool        applied;
public:
    RemoveCommand(FileSystem& f, const std::string& absPath);
    ~RemoveCommand() override;
    Status      execute() override;
    bool        undo() override;
    bool        isApplied() const override { return applied; }
    std::string describe() const override;
};

#endif
