/* Keeps track of executed commands so changes can be undone or redone. */

#ifndef COMMANDMANAGER_H
#define COMMANDMANAGER_H

#include <string>
#include "Stack.h"
#include "Command.h"

class CommandManager {
private:
    Stack<Command*> undoStack;
    Stack<Command*> redoStack;

    void deleteAll(Stack<Command*>& s);

public:
    CommandManager() {}
    ~CommandManager();

    CommandManager(const CommandManager&) = delete;
    CommandManager& operator=(const CommandManager&) = delete;

    Status run(Command* cmd);

    std::string undo();
    std::string redo();

    int  undoCount() const { return undoStack.size(); }
    int  redoCount() const { return redoStack.size(); }
    void clearAll();
};

#endif
