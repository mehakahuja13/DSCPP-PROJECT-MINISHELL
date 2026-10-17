#include "Commands.h"

// ---------------- MkdirCommand ----------------
MkdirCommand::MkdirCommand(FileSystem& f, const std::string& absPath)
    : fs(f), path(absPath), applied(false) {}

Status MkdirCommand::execute() {
    Status s = fs.mkdir(path);
    applied = (s == Status::OK);
    return s;
}

// A directory created by mkdir is empty again by the time it is undone,
// because later commands are always undone first (stack order).
bool MkdirCommand::undo() {
    return fs.remove(path) == Status::OK;
}

std::string MkdirCommand::describe() const { return "mkdir " + path; }

// ---------------- TouchCommand ----------------
TouchCommand::TouchCommand(FileSystem& f, const std::string& absPath)
    : fs(f), path(absPath), applied(false) {}

Status TouchCommand::execute() {
    bool existed = (fs.resolve(path) != nullptr);
    Status s = fs.touch(path);
    applied = (s == Status::OK && !existed);
    return s;
}

bool TouchCommand::undo() {
    if (!applied) return true;
    return fs.remove(path) == Status::OK;
}

std::string TouchCommand::describe() const { return "touch " + path; }

// ---------------- WriteCommand ----------------
WriteCommand::WriteCommand(FileSystem& f, const std::string& absPath, const std::string& newText)
    : fs(f), path(absPath), text(newText), existedBefore(false), applied(false) {}

Status WriteCommand::execute() {
    FSNode* node = fs.resolve(path);
    existedBefore = (node != nullptr && !node->isDirectory());
    oldContent.clear();
    if (existedBefore) fs.cat(path, oldContent);   // remember what we are about to overwrite

    Status s = fs.writeFile(path, text);
    applied = (s == Status::OK);
    return s;
}

bool WriteCommand::undo() {
    if (!applied) return true;
    if (!existedBefore) return fs.remove(path) == Status::OK;   // the write created the file
    return fs.writeFile(path, oldContent) == Status::OK;        // put the old content back
}

std::string WriteCommand::describe() const { return "write " + path; }

// ---------------- RemoveCommand ----------------
RemoveCommand::RemoveCommand(FileSystem& f, const std::string& absPath)
    : fs(f), path(absPath), held(nullptr), applied(false) {}

RemoveCommand::~RemoveCommand() {
    delete held;   // frees the node (and its subtree) only if it was never restored
}

Status RemoveCommand::execute() {
    FSNode* node = fs.resolve(path);
    if (node != nullptr && node->getParent() != nullptr) {
        parentPath = fs.pathOf(node->getParent());
    }

    FSNode* detached = nullptr;
    Status s = fs.detach(path, detached);
    if (s == Status::OK) {
        held = detached;
        applied = true;
    }
    return s;
}

bool RemoveCommand::undo() {
    if (held == nullptr) return false;
    if (fs.attach(parentPath, held) != Status::OK) return false;
    held = nullptr;   // the tree owns the node again
    return true;
}

std::string RemoveCommand::describe() const { return "rm " + path; }

// ---------------- CopyCommand ----------------
CopyCommand::CopyCommand(FileSystem& f, const std::string& absSrc, const std::string& absDst)
    : fs(f), src(absSrc), dst(absDst), applied(false) {}

Status CopyCommand::execute() {
    Status s = fs.copy(src, dst, created);
    applied = (s == Status::OK);
    return s;
}

bool CopyCommand::undo() {
    return fs.remove(created) == Status::OK;
}

std::string CopyCommand::describe() const { return "cp " + src + " " + dst; }

// ---------------- MoveCommand ----------------
MoveCommand::MoveCommand(FileSystem& f, const std::string& absSrc, const std::string& absDst)
    : fs(f), src(absSrc), dst(absDst), applied(false) {}

Status MoveCommand::execute() {
    Status s = fs.move(src, dst, finalPath);
    applied = (s == Status::OK);
    return s;
}

// Moving the item from its new place back to the old path restores it
// (the old path no longer exists, so it is treated as "parent folder + name").
bool MoveCommand::undo() {
    std::string back;
    return fs.move(finalPath, src, back) == Status::OK;
}

std::string MoveCommand::describe() const { return "mv " + src + " " + dst; }

// ---------------- LockCommand ----------------
LockCommand::LockCommand(FileSystem& f, const std::string& absPath, bool lockValue)
    : fs(f), path(absPath), lockIt(lockValue), applied(false) {}

Status LockCommand::execute() {
    bool changed = false;
    Status s = fs.setLocked(path, lockIt, changed);
    applied = (s == Status::OK && changed);
    return s;
}

bool LockCommand::undo() {
    if (!applied) return true;
    bool changed = false;
    return fs.setLocked(path, !lockIt, changed) == Status::OK;
}

std::string LockCommand::describe() const { return std::string(lockIt ? "lock " : "unlock ") + path; }

// ---------------- EncryptCommand ----------------
EncryptCommand::EncryptCommand(FileSystem& f, const std::string& absPath, const std::string& pw, bool encryptValue)
    : fs(f), path(absPath), password(pw), encryptIt(encryptValue), applied(false) {}

Status EncryptCommand::execute() {
    Status s = encryptIt ? fs.encryptFile(path, password) : fs.decryptFile(path, password);
    applied = (s == Status::OK);
    return s;
}

bool EncryptCommand::undo() {
    if (!applied) return true;
    Status s = encryptIt ? fs.decryptFile(path, password) : fs.encryptFile(path, password);
    return s == Status::OK;
}

std::string EncryptCommand::describe() const { return std::string(encryptIt ? "encrypt " : "decrypt ") + path; }
