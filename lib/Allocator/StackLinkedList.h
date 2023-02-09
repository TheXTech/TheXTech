#ifndef STACKLINKEDLIST_H
#define STACKLINKEDLIST_H

template <class T>
class StackLinkedList
{
public:
    struct Node
    {
        T data;
        Node* next = nullptr;
    };

    Node* head = nullptr;
    Node* front = nullptr;
    Node* back = nullptr;

public:
    StackLinkedList() = default;
    StackLinkedList(StackLinkedList& stackLinkedList) = delete;
    void set_edges(Node *front, Node *back);
    void push(Node* newNode);
    Node* pop();
};

#include "StackLinkedListImpl.h"

#endif /* STACKLINKEDLIST_H */
