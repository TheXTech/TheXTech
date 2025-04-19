#ifndef STACKLINKEDLIST_H
#define STACKLINKEDLIST_H

#include <stddef.h>

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
    size_t m_counter = 0;
    size_t m_maximum = 0;

public:
    StackLinkedList() = default;
    StackLinkedList(StackLinkedList& stackLinkedList) = delete;
    void set_edges(Node *front, Node *back);
    void push(Node* newNode);
    Node* pop();

    size_t count() const
    {
        return m_counter;
    }

    size_t maximum() const
    {
        return m_maximum;
    }
};

#include "StackLinkedListImpl.h"

#endif /* STACKLINKEDLIST_H */
