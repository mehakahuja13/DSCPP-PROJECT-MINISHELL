#ifndef STACK_H
#define STACK_H

// Hand-written generic stack (LIFO) built on a singly linked list.
// Used for: command history, the undo stack and the redo stack.
// It does not own what it stores; whoever pushes pointers must delete them.
template <typename T>
class Stack {
private:
    struct Node {
        T     data;
        Node* next;
        Node(const T& d, Node* n) : data(d), next(n) {}
    };

    Node* topNode;
    int   count;

public:
    Stack() : topNode(nullptr), count(0) {}
    ~Stack() { clear(); }

    Stack(const Stack&) = delete;
    Stack& operator=(const Stack&) = delete;

    void push(const T& value) {
        topNode = new Node(value, topNode);
        count++;
    }

    // Removes the top element into 'out'. Returns false if the stack is empty.
    bool pop(T& out) {
        if (topNode == nullptr) return false;
        Node* old = topNode;
        out = old->data;
        topNode = old->next;
        delete old;
        count--;
        return true;
    }

    bool isEmpty() const { return topNode == nullptr; }
    int  size() const { return count; }

    void clear() {
        while (topNode != nullptr) {
            Node* old = topNode;
            topNode = old->next;
            delete old;
        }
        count = 0;
    }
};

#endif
