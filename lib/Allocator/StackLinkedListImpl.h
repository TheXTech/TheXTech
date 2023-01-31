#include "StackLinkedList.h"

template <class T>
void StackLinkedList<T>::push(Node* newNode)
{
    newNode->next = head;
    head = newNode;
}

template <class T>
typename StackLinkedList<T>::Node* StackLinkedList<T>::pop()
{
    if(!head || head < front || head > back)
        return nullptr;
    Node* top = head;
    head = head->next;
    return top;
}

template <class T>
void StackLinkedList<T>::set_edges(Node *i_front, Node *i_back)
{
    front = i_front;
    back = i_back;
}
