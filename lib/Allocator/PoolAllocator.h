#pragma once
#ifndef POOL_ALLOCATOR_H
#define POOL_ALLOCATOR_H

#include <stdint.h>

#include "Allocator.h"
#include "StackLinkedList.h"

class PoolAllocator : public Allocator
{
private:
    typedef uintptr_t FreeHeader;
    using Node = StackLinkedList<FreeHeader>::Node;
    StackLinkedList<FreeHeader> m_freeList;

    void* m_start_ptr = nullptr;
    std::size_t m_chunkSize;
    std::size_t m_chunkSizeH;
    std::size_t m_capacity;

    size_t m_count = 0;
    size_t m_maximum = 0;
    size_t m_maximumPrev = 0;
public:
    PoolAllocator(const std::size_t totalSize, const std::size_t chunkSize);

    virtual ~PoolAllocator();

    virtual void* Allocate(const std::size_t size, const std::size_t alignment = 0) override;

    virtual void Free(void* ptr) override;

    virtual void Init() override;

    virtual void Reset();

    size_t maximum() const;
    size_t prevMaximum() const;
private:
    PoolAllocator(PoolAllocator& poolAllocator);

};

#endif // POOL_ALLOCATOR_H
