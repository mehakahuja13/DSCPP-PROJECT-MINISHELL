#include "CommandManager.h"

CommandManager::~CommandManager() {
    deleteAll(undoStack);
    deleteAll(redoStack);
}

void CommandManager::deleteAll(Stack<Command*>& s) {
    Command* c = nullptr;
    while (s.pop(c)) delete c;
}

Status CommandManager::run(Command* cmd) {
    Status s = cmd->execute();
    if (s == Status::OK && cmd->isApplied()) {
        undoStack.push(cmd);
        deleteAll(redoStack);     // a new change makes the old "redo" history invalid
    } else {
        delete cmd;               // failed or changed nothing: nothing to undo later
    }
    return s;
}

void CommandManager::clearAll() {
    deleteAll(undoStack);
    deleteAll(redoStack);
}

std::string CommandManager::undo() {
    Command* cmd = nullptr;
    if (!undoStack.pop(cmd)) return "Nothing to undo.";

    if (cmd->undo()) {
        redoStack.push(cmd);
        return "Undone: " + cmd->describe();
    }
    undoStack.push(cmd);          // could not undo now, keep it on the stack
    return "Cannot undo: " + cmd->describe() +
           "\n(If you are inside that folder, move out with 'cd ..' and try again.)";
}

std::string CommandManager::redo() {
    Command* cmd = nullptr;
    if (!redoStack.pop(cmd)) return "Nothing to redo.";

    if (cmd->execute() == Status::OK) {
        undoStack.push(cmd);
        return "Redone: " + cmd->describe();
    }
    redoStack.push(cmd);
    return "Cannot redo: " + cmd->describe();
}
