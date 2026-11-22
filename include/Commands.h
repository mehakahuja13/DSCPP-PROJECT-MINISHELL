/* Declares the file-system operations that can be executed and reversed. */

#ifndef COMMANDS_H
#define COMMANDS_H

#include "Command.h"

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
    bool        applied;
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
    std::string text;
    std::string oldContent;
    bool        existedBefore;
    bool        applied;
public:
    WriteCommand(FileSystem& f, const std::string& absPath, const std::string& newText);
    Status      execute() override;
    bool        undo() override;
    bool        isApplied() const override { return applied; }
    std::string describe() const override;
};

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

class CopyCommand : public Command {
private:
    FileSystem& fs;
    std::string src;
    std::string dst;
    std::string created;
    bool        applied;
public:
    CopyCommand(FileSystem& f, const std::string& absSrc, const std::string& absDst);
    Status      execute() override;
    bool        undo() override;
    bool        isApplied() const override { return applied; }
    std::string describe() const override;
};

class MoveCommand : public Command {
private:
    FileSystem& fs;
    std::string src;
    std::string dst;
    std::string finalPath;
    bool        applied;
public:
    MoveCommand(FileSystem& f, const std::string& absSrc, const std::string& absDst);
    Status      execute() override;
    bool        undo() override;
    bool        isApplied() const override { return applied; }
    std::string describe() const override;
};

class LockCommand : public Command {
private:
    FileSystem& fs;
    std::string path;
    bool        lockIt;
    bool        applied;
public:
    LockCommand(FileSystem& f, const std::string& absPath, bool lockValue);
    Status      execute() override;
    bool        undo() override;
    bool        isApplied() const override { return applied; }
    std::string describe() const override;
};

class EncryptCommand : public Command {
private:
    FileSystem& fs;
    std::string path;
    std::string password;
    bool        encryptIt;
    bool        applied;
public:
    EncryptCommand(FileSystem& f, const std::string& absPath, const std::string& pw, bool encryptValue);
    Status      execute() override;
    bool        undo() override;
    bool        isApplied() const override { return applied; }
    std::string describe() const override;
};

#endif
