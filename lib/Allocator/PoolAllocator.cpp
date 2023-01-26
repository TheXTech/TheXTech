#include "PoolAllocator.h"
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>     /* malloc, free */
#include <algorithm>    //max
#ifdef _BUILD
#include <iostream>
#endif
#include "../sdl_proxy/sdl_stdinc.h"
#include "../sdl_proxy/sdl_assert.h"

PoolAllocator::PoolAllocator(const std::size_t totalSize, const std::size_t chunkSize)
    : Allocator(totalSize)
{
    assert(chunkSize >= 8 && "Chunk size must be greater or equal to 8");
    assert(totalSize % chunkSize == 0 && "Total Size must be a multiple of Chunk Size");
    this->m_chunkSize = chunkSize;
}

void PoolAllocator::Init()
{
    m_start_ptr = SDL_malloc(m_totalSize);
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

    SDL_assert_release(freePosition != nullptr && "The pool allocator is full");

    m_used += m_chunkSize;
    m_peak = SDL_max(m_peak, m_used);

#ifdef _BUILD
    std::cout << "A" << "\t@S " << m_start_ptr << "\t@R " << (void*) freePosition << "\tM " << m_used << std::endl;
    std::cout.flush();
#endif

    return (void*) freePosition;
}

void PoolAllocator::Free(void* ptr)
{
    m_used -= m_chunkSize;

    m_freeList.push(reinterpret_cast<Node*>(ptr));

#ifdef _BUILD
    std::cout << "F" << "\t@S " << m_start_ptr << "\t@F " << ptr << "\tM " << m_used << std::endl;
    std::cout.flush();
#endif
}

void PoolAllocator::Reset()
{
    m_used = 0;
    m_peak = 0;
    // Create a linked-list with all free positions
    const int nChunks = m_totalSize / m_chunkSize;

    std::size_t front = reinterpret_cast<std::size_t>(m_start_ptr);
    std::size_t back = reinterpret_cast<std::size_t>(m_start_ptr) + m_totalSize;

    m_freeList.set_edges(reinterpret_cast<Node*>(front), reinterpret_cast<Node*>(back));

    for(int i = 0; i < nChunks; ++i)
    {
        std::size_t address = reinterpret_cast<std::size_t>(m_start_ptr) + i * m_chunkSize;
        m_freeList.push(reinterpret_cast<Node*>(address));
    }
}
