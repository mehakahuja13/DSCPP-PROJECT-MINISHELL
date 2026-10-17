#ifndef COMMANDMANAGER_H
#define COMMANDMANAGER_H

#include <string>
#include "Stack.h"
#include "Command.h"

// Runs commands and keeps two stacks:
//   undoStack : commands that have been done (top = most recent)
//   redoStack : commands that were undone   (top = most recently undone)
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

    // Takes ownership of 'cmd'. Executes it; if it changed something it goes on
    // the undo stack and the redo stack is cleared (new action = new timeline).
    Status run(Command* cmd);

    std::string undo();   // returns a message to show the user
    std::string redo();

    int  undoCount() const { return undoStack.size(); }
    int  redoCount() const { return redoStack.size(); }
    void clearAll();      // forget all undo / redo history (used after load)
};

#endif
