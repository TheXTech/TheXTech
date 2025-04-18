#include "PoolAllocator.h"
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>     /* malloc, free */
#include <fmt_format_ne.h>
#ifdef _BUILD
#include <iostream>
#endif
#include "core/msgbox.h"
#include "../sdl_proxy/sdl_stdinc.h"
#include "../sdl_proxy/sdl_assert.h"


PoolAllocator::PoolAllocator(const std::size_t totalSize, const std::size_t chunkSize)
    : Allocator(totalSize),
    m_capacity(totalSize)
{
    assert(chunkSize >= 8 && "Chunk size must be greater or equal to 8");
    assert(totalSize % chunkSize == 0 && "Total Size must be a multiple of Chunk Size");
    this->m_chunkSize = chunkSize;
}

void PoolAllocator::Init()
{
    m_start_ptr = SDL_malloc(m_totalSize);
    SDL_assert_release(m_start_ptr && "Out of memory: Can't allocate the pool allocator");
    this->Reset();
}

PoolAllocator::~PoolAllocator()
{
    SDL_free(m_start_ptr);
}

void* PoolAllocator::Allocate(const std::size_t allocationSize, const std::size_t /*alignment*/)
{
#ifdef NDEBUG
    (void)allocationSize;
#endif
    SDL_assert(allocationSize == this->m_chunkSize && "Allocation size must be equal to chunk size");

    Node* freePosition = m_freeList.pop();

    if(!freePosition)
    {
        // Fatal error
        XMsgBox::errorMsgBox("Fatal error",
            fmt::format_ne("The pool allocator is full:\n"
            "- Total capacity: {0}\n"
            "- Current size of free list: {1}\n"
            "- Maximum size of free list: {2}\n"
            "\n"
            "Game will be closed. Please check logs for details.",
            m_capacity,
            m_freeList.count(),
            m_freeList.maximum())
        );
        abort();
    }

    m_used += m_chunkSize;
    m_peak = SDL_max(m_peak, m_used);

#ifdef _BUILD
    std::cout << "A" << "\t@S " << m_start_ptr << "\t@R " << (void*) freePosition << "\tM " << m_used << std::endl;
    std::cout.flush();
#endif

    ++m_count;
    if(m_count > m_maximum)
        m_maximum = m_count;

    return (void*) freePosition;
}

void PoolAllocator::Free(void* ptr)
{
    --m_count;
    m_used -= m_chunkSize;

    m_freeList.push(reinterpret_cast<Node*>(ptr));

#ifdef _BUILD
    std::cout << "F" << "\t@S " << m_start_ptr << "\t@F " << ptr << "\tM " << m_used << std::endl;
    std::cout.flush();
#endif
}

void PoolAllocator::Reset()
{
    m_maximumPrev = m_maximum;
    m_count = 0;
    m_maximum = 0;
    m_used = 0;
    m_peak = 0;
    // Create a linked-list with all free positions
    const size_t nChunks = m_totalSize / m_chunkSize;

    uintptr_t front = reinterpret_cast<uintptr_t>(m_start_ptr);
    uintptr_t back = reinterpret_cast<uintptr_t>(m_start_ptr) + m_totalSize;

    m_freeList.set_edges(reinterpret_cast<Node*>(front), reinterpret_cast<Node*>(back));

    for(size_t i = 0; i < nChunks; ++i)
    {
        uintptr_t address = reinterpret_cast<uintptr_t>(m_start_ptr) + i * m_chunkSize;
        m_freeList.push(reinterpret_cast<Node*>(address));
    }
}

size_t PoolAllocator::maximum() const
{
    return m_maximum;
}

size_t PoolAllocator::prevMaximum() const
{
    return m_maximumPrev;
}
