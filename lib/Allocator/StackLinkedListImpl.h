#pragma once
#ifndef STACK_LINKED_LIST_IMPL_H
#define STACK_LINKED_LIST_IMPL_H

#include "StackLinkedList.h"

template <class T>
void StackLinkedList<T>::push(Node* newNode)
{
    newNode->next = head;
    head = newNode;
    --m_counter;
}

template <class T>
typename StackLinkedList<T>::Node* StackLinkedList<T>::pop()
{
    if(!head || head < front || head > back)
        return nullptr;

    Node* top = head;
    head = head->next;

    ++m_counter;
    if(m_counter > m_maximum)
        m_maximum = m_counter;

    return top;
}

template <class T>
void StackLinkedList<T>::set_edges(Node *i_front, Node *i_back)
{
    m_counter = 0;
    front = i_front;
    back = i_back;
}

#endif // STACK_LINKED_LIST_IMPL_H
