/**
 * LooseQuadtree written by Zozo
 * use freely under MIT license
 */

#include "LooseQuadtree.h"
#include "LooseQuadtree-impl.tcc"

namespace loose_quadtree
{

namespace detail
{

BlocksAllocator::BlocksAllocator() {}


BlocksAllocator::~BlocksAllocator()
{
    for(auto &size_block_pair : size_to_blocks_)
    {
        BlocksHead &blocks_head = size_block_pair.second;
        for(auto &address_empty_pair : blocks_head.address_to_empty_slot_number)
        {
            assert(address_empty_pair.second == blocks_head.slots_in_a_block_);
            Block *block = address_empty_pair.first;
            delete block;
        }
    }
}


void *BlocksAllocator::Allocate(std::size_t object_size)
{
#ifdef LQT_USE_OWN_ALLOCATOR
    if(object_size < sizeof(void *)) object_size = sizeof(void *);
    assert(object_size <= kMaxAllowedAlloc);
    auto size_blocks_it = size_to_blocks_.find(object_size);
    if(size_blocks_it == size_to_blocks_.end())
        size_blocks_it = size_to_blocks_.emplace(object_size, object_size).first;
    BlocksHead &blocks_head = size_blocks_it->second;
    assert(blocks_head.slots_in_a_block_ == kBlockSize / object_size);
    if(blocks_head.first_empty_slot == nullptr)
    {
        Block *new_block = new Block();
        std::size_t empties = blocks_head.slots_in_a_block_;
        blocks_head.address_to_empty_slot_number.emplace(new_block, empties);
        blocks_head.first_empty_slot = reinterpret_cast<void *>(new_block);
        void *current_slot = reinterpret_cast<void *>(new_block);
        empties--;
        while(empties > 0)
        {
            void *next_slot =
                reinterpret_cast<void *>(reinterpret_cast<char *>(current_slot)
                                         + object_size);
            *reinterpret_cast<void **>(current_slot) = next_slot;
            current_slot = next_slot;
            empties--;
        }
        *reinterpret_cast<void **>(current_slot) = nullptr;
    }
    assert(blocks_head.first_empty_slot != nullptr);
    void *slot = blocks_head.first_empty_slot;
    blocks_head.first_empty_slot = *reinterpret_cast<void **>(slot);
    auto address_empties_it =
        blocks_head.address_to_empty_slot_number.upper_bound(reinterpret_cast<Block *>(slot));
    assert(address_empties_it != blocks_head.address_to_empty_slot_number.begin());
    address_empties_it--;
    assert(address_empties_it->first <= reinterpret_cast<Block *>(slot) &&
           (std::size_t)(reinterpret_cast<char *>(slot) -
                         reinterpret_cast<char *>(address_empties_it->first)) < kBlockSize);
    assert((std::size_t)(reinterpret_cast<char *>(slot) -
                         reinterpret_cast<char *>(address_empties_it->first)) % object_size == 0);
    assert(address_empties_it->second > 0 &&
           address_empties_it->second <= blocks_head.slots_in_a_block_);
    address_empties_it->second--;
    return slot;
#else
    return reinterpret_cast<void *>(new char[object_size]);
#endif
}


void BlocksAllocator::Deallocate(void *p, std::size_t object_size)
{
#ifdef LQT_USE_OWN_ALLOCATOR
    if(object_size < sizeof(void *)) object_size = sizeof(void *);
    assert(object_size <= kMaxAllowedAlloc);
    auto size_blocks_it = size_to_blocks_.find(object_size);
    assert(size_blocks_it != size_to_blocks_.end());
    BlocksHead &blocks_head = size_blocks_it->second;
    assert(blocks_head.slots_in_a_block_ == kBlockSize / object_size);
    auto address_empties_it =
        blocks_head.address_to_empty_slot_number.upper_bound(reinterpret_cast<Block *>(p));
    assert(address_empties_it != blocks_head.address_to_empty_slot_number.begin());
    address_empties_it--;
    assert(address_empties_it->first <= reinterpret_cast<Block *>(p) &&
           (std::size_t)(reinterpret_cast<char *>(p) -
                         reinterpret_cast<char *>(address_empties_it->first)) < kBlockSize);
    assert((std::size_t)(reinterpret_cast<char *>(p) -
                         reinterpret_cast<char *>(address_empties_it->first)) % object_size == 0);
    assert(address_empties_it->second < blocks_head.slots_in_a_block_);
    void *slot = p;
    *reinterpret_cast<void **>(slot) = blocks_head.first_empty_slot;
    blocks_head.first_empty_slot = slot;
    address_empties_it->second++;
    assert(address_empties_it->second > 0 &&
           address_empties_it->second <= blocks_head.slots_in_a_block_);
#else
    (void)object_size;
    delete[] reinterpret_cast<char *>(p);
#endif
}


void BlocksAllocator::ReleaseFreeBlocks()
{
    for(auto &size_block_pair : size_to_blocks_)
    {
        BlocksHead &blocks_head = size_block_pair.second;
        void **current = &blocks_head.first_empty_slot;
        while(*current != nullptr)
        {
            auto address_empties_it =
                blocks_head.address_to_empty_slot_number.upper_bound(reinterpret_cast<Block *>(*current));
            assert(address_empties_it != blocks_head.address_to_empty_slot_number.begin());
            address_empties_it--;
            assert(address_empties_it->first <= reinterpret_cast<Block *>(*current) &&
                   (std::size_t)(reinterpret_cast<char *>(*current) -
                                 reinterpret_cast<char *>(address_empties_it->first)) < kBlockSize);
            assert((std::size_t)(reinterpret_cast<char *>(*current) -
                                 reinterpret_cast<char *>(address_empties_it->first)) % size_block_pair.first == 0);
            assert(address_empties_it->second > 0 &&
                   address_empties_it->second <= blocks_head.slots_in_a_block_);
            if(address_empties_it->second >= blocks_head.slots_in_a_block_)
                *current = **reinterpret_cast<void ***>(current);
            else
                current = *reinterpret_cast<void ***>(current);
        }
        auto address_empties_it = blocks_head.address_to_empty_slot_number.begin();
        while(address_empties_it != blocks_head.address_to_empty_slot_number.end())
        {
            if(address_empties_it->second >= blocks_head.slots_in_a_block_)
            {
                auto prev_address_empties_it = address_empties_it;
                address_empties_it++;
                blocks_head.address_to_empty_slot_number.erase(prev_address_empties_it);
            }
            else
                address_empties_it++;
        }
    }
}

} //detail

} //loose_quadtree

