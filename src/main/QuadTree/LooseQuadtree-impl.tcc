/**
 * LooseQuadtree written by Zozo
 * use freely under MIT license
 */

#ifndef LOOSEQUADTREE_LOOSEQUADTREE_IMPL_H
#define LOOSEQUADTREE_LOOSEQUADTREE_IMPL_H

#include "LooseQuadtree.h"

#include <array>
#include <cassert>
#include <cstddef>
#include <deque>
#include <forward_list>
#include <limits>
#include <map>
#include <memory>
#include <unordered_map>
#include <type_traits>
#include <vector>

namespace loose_quadtree
{
namespace detail
{



#define LQT_USE_OWN_ALLOCATOR


class BlocksAllocator
{
public:
    const static std::size_t kBlockAlign = alignof(long double);
    const static std::size_t kBlockSize = 16384;
    const static std::size_t kMaxAllowedAlloc = sizeof(void *) * 8;

    BlocksAllocator();
    ~BlocksAllocator();
    BlocksAllocator(const BlocksAllocator &) = delete;
    BlocksAllocator &operator=(const BlocksAllocator &) = delete;

    void *Allocate(std::size_t object_size);
    void Deallocate(void *p, std::size_t object_size);
    void ReleaseFreeBlocks();
    template <typename T, typename... Args>
    T *New(Args &&... args);
    template <typename T>
    void Delete(T *p);

private:
    using Block = std::aligned_storage<kBlockSize, kBlockAlign>::type;

    //template <std::size_t kSizeT>
    //union Slot {
    //  Slot* next_empty_slot;
    //  std::array<char, kSizeT> data;
    //};

    using BlocksAndEmptySlots = std::map<Block *, std::size_t>;

    struct BlocksHead
    {
        BlocksAndEmptySlots address_to_empty_slot_number;
        void *first_empty_slot;
        std::size_t slots_in_a_block_;

        BlocksHead(std::size_t object_size)
        {
            first_empty_slot = nullptr;
            slots_in_a_block_ = kBlockSize / object_size;
        }
    };

    std::map<std::size_t, BlocksHead> size_to_blocks_;
};


template <typename T>
struct BlocksAllocatorAdaptor
{
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    template<typename U>
    struct rebind
    {
        using other = BlocksAllocatorAdaptor<U>;
    };

    BlocksAllocatorAdaptor(BlocksAllocator &allocator);
    template <typename U>
    BlocksAllocatorAdaptor(const BlocksAllocatorAdaptor<U> &other);
    template <typename U>
    BlocksAllocatorAdaptor &operator=(const BlocksAllocatorAdaptor<U> &other);

    pointer address(reference r) const
    {
        return &r;
    }
    const_pointer address(const_reference r) const
    {
        return &r;
    }
    size_type max_size() const
    {
        return std::numeric_limits<size_type>::max();
    }
    template <typename U, typename... Args>
    void construct(U *p, Args &&... args)
    {
        new((void *)p) U(std::forward<Args>(args)...);
    }
    template <typename U>
    void destroy(U *p)
    {
        p->~U();
    }

    T *allocate(std::size_t n);
    void deallocate(T *p, std::size_t n);

    BlocksAllocator *allocator_;
};


template <typename T, typename... Args>
T *BlocksAllocator::New(Args &&... args)
{
    return new(Allocate(sizeof(T))) T(std::forward<Args>(args)...);
}


template <typename T>
void BlocksAllocator::Delete(T *p)
{
    p->~T();
    Deallocate(p, sizeof(T));
}


template <typename T>
BlocksAllocatorAdaptor<T>::BlocksAllocatorAdaptor(BlocksAllocator &allocator)
    : allocator_(&allocator)
{
}

template <typename T>
template <typename U>
BlocksAllocatorAdaptor<T>::BlocksAllocatorAdaptor(const BlocksAllocatorAdaptor<U> &other)
    : allocator_(other.allocator_)
{
}

template <typename T>
template <typename U>
BlocksAllocatorAdaptor<T> &
BlocksAllocatorAdaptor<T>::operator=(const BlocksAllocatorAdaptor<U> &other)
{
    allocator_ = other.allocator_;
}

template <typename T>
T *BlocksAllocatorAdaptor<T>::allocate(std::size_t n)
{
    if(sizeof(T) <= BlocksAllocator::kMaxAllowedAlloc && n == 1)
        return reinterpret_cast<T *>(allocator_->Allocate(sizeof(T)));
    else
        return reinterpret_cast<T *>(new char[sizeof(T) * n]);
}

template <typename T>
void BlocksAllocatorAdaptor<T>::deallocate(T *p, std::size_t n)
{
    if(sizeof(T) <= BlocksAllocator::kMaxAllowedAlloc && n == 1)
        allocator_->Deallocate(p, sizeof(T));
    else
        delete[] reinterpret_cast<char *>(p);
}






template <typename NumberT>
struct MakeDistance
{
    using Type = typename std::make_unsigned<NumberT>::type;
};

template <>
struct MakeDistance<float>
{
    using Type = float;
};

template <>
struct MakeDistance<double>
{
    using Type = double;
};

template <>
struct MakeDistance<long double>
{
    using Type = long double;
};



enum class ChildPosition
{
    kNone,
    kTopLeft,
    kTopRight,
    kBottomRight,
    kBottomLeft,
};



template <typename ObjectT>
struct TreeNode
{
    using Object = ObjectT;
    using ObjectContainer =
        std::forward_list<Object *, BlocksAllocatorAdaptor<Object *>>;

    TreeNode(BlocksAllocator &allocator) :
        top_left(nullptr), top_right(nullptr), bottom_right(nullptr),
        bottom_left(nullptr), objects(BlocksAllocatorAdaptor<Object*>(allocator))
    {}

    TreeNode<Object> *top_left;
    TreeNode<Object> *top_right;
    TreeNode<Object> *bottom_right;
    TreeNode<Object> *bottom_left;
    ObjectContainer objects;
};



template <typename NumberT, typename ObjectT>
class ForwardTreeTraversal
{
public:
    using Number = NumberT;
    using Object = ObjectT;

    struct TreePosition
    {
        TreePosition(const BoundingBox<Number> &_bbox, TreeNode<Object> *_node) :
            bounding_box(_bbox), node(_node)
        {
            current_child = ChildPosition::kNone;
        }

        BoundingBox<Number> bounding_box;
        TreeNode<Object> *node;
        ChildPosition current_child;
    };

    ForwardTreeTraversal();
    void StartAt(TreeNode<Object> *root, const BoundingBox<Number> &root_bounds);
    int GetDepth() const; ///< starting from 0
    TreeNode<Object> *GetNode() const;
    const BoundingBox<Number> &GetNodeBoundingBox() const;
    void GoTopLeft();
    void GoTopRight();
    void GoBottomRight();
    void GoBottomLeft();

protected:
    TreePosition position_;
    int depth_;
};



template <typename NumberT, typename ObjectT>
class FullTreeTraversal : public ForwardTreeTraversal<NumberT, ObjectT>
{
public:
    using Number = NumberT;
    using Object = ObjectT;
    using typename ForwardTreeTraversal<Number, Object>::TreePosition;

    void StartAt(TreeNode<Object> *root, const BoundingBox<Number> &root_bounds);
    ChildPosition GetNodeCurrentChild() const;
    void SetNodeCurrentChild(ChildPosition child_position);
    void GoUp();
    void GoTopLeft();
    void GoTopRight();
    void GoBottomRight();
    void GoBottomLeft();

private:
    using ForwardTreeTraversal<Number, Object>::position_;
    using ForwardTreeTraversal<Number, Object>::depth_;

    std::vector<TreePosition> position_stack_;
};



} //detail



template <typename NumberT, typename ObjectT, typename BoundingBoxExtractorT>
class
    LooseQuadtree<NumberT, ObjectT, BoundingBoxExtractorT>::Query::
    Impl
{
public:
    enum class QueryType {kIntersects, kInside, kContains, kEndOfQuery};

    Impl();
    void Acquire(typename LooseQuadtree<Number, Object, BoundingBoxExtractor>::Impl *quadtree,
                 const BoundingBox<Number> *query_region, QueryType query_type);
    void Release();
    bool IsAvailable() const;
    bool EndOfQuery() const;
    Object *GetCurrent() const;
    void Next();

private:
    enum class FitType {kNoFit = 0, kPartialFit, kFreeRide};

    bool CurrentObjectFits() const;
    FitType CurrentNodeFits() const;

    typename LooseQuadtree<Number, Object, BoundingBoxExtractor>::Impl *quadtree_;
    detail::FullTreeTraversal<Number, Object> traversal_;
    typename detail::TreeNode<Object>::ObjectContainer::iterator object_iterator_;
    typename detail::TreeNode<Object>::ObjectContainer::iterator object_iterator_before_;
    BoundingBox<Number> query_region_;
    QueryType query_type_;
    int free_ride_from_level_;
};



template <typename NumberT, typename ObjectT, typename BoundingBoxExtractorT>
class
    LooseQuadtree<NumberT, ObjectT, BoundingBoxExtractorT>::
    Impl
{
public:
    constexpr static int kInternalMinDepth = 4;
    constexpr static int kInternalMaxDepth = (sizeof(long long) * 8 - 1) / 2;
    constexpr static Number kMinimalObjectExtent =
        std::is_integral<Number>::value ? 1 :
        std::numeric_limits<Number>::min() * 16;

    Impl();
    ~Impl();
    Impl(const Impl &) = delete;
    Impl &operator=(const Impl &) = delete;

    bool Insert(Object *object);
    bool Update(Object *object);
    bool Remove(Object *object);
    bool Contains(Object *object) const;
    Query QueryIntersectsRegion(const BoundingBox<Number> &region);
    Query QueryInsideRegion(const BoundingBox<Number> &region);
    Query QueryContainsRegion(const BoundingBox<Number> &region);
    const BoundingBox<Number> &GetBoundingBox() const; ///< loose sense bounds
    int GetSize() const;
    void Clear();
    void ForceCleanup();

private:
    friend class Query::Impl;
    using ObjectPointerContainer =
        std::unordered_map<Object *, Object **,
        std::hash<Object *>, std::equal_to<Object *>,
        detail::BlocksAllocatorAdaptor<std::pair<Object *const, Object **>>>;
    using QueryPoolContainer =
        std::deque<typename LooseQuadtree<Number, Object, BoundingBoxExtractor>::Query::Impl>;

    void RecalculateMaximalDepth();
    void DeleteTree();
    Object **InsertIntoTree(Object *object);
    typename Query::Impl *GetAvailableQueryFromPool();

    detail::BlocksAllocator allocator_;
    detail::TreeNode<Object> *root_;
    BoundingBox<Number> bounding_box_;
    ObjectPointerContainer object_pointers_;
    int number_of_objects_;
    int maximal_depth_;
    detail::FullTreeTraversal<Number, Object> internal_traversal_;
    QueryPoolContainer query_pool_;
    int running_queries_; ///< queries which are opened and not at their end
};






template <typename NumberT>
bool
BoundingBox<NumberT>::
Intersects(const BoundingBox<Number> &other) const
{
    return !(
               left + width <= other.left || other.left + other.width <= left ||
               top + height <= other.top || other.top + other.height <= top
           );
}

template <typename NumberT>
bool
BoundingBox<NumberT>::
Contains(const BoundingBox<Number> &other)  const
{
    return left <= other.left && left + width >= other.left + other.width &&
           top <= other.top && top + height >= other.top + other.height;
}

template <typename NumberT>
bool
BoundingBox<NumberT>::
Contains(Number x, Number y) const
{
    return left <= x && x < left + width &&
           top <= y && y < top + height;
}



template <typename NumberT, typename ObjectT>
detail::ForwardTreeTraversal<NumberT, ObjectT>::
ForwardTreeTraversal() :
    position_(BoundingBox<Number>(0, 0, 0, 0), nullptr), depth_(0)
{
}

template <typename NumberT, typename ObjectT>
void
detail::ForwardTreeTraversal<NumberT, ObjectT>::
StartAt(TreeNode<Object> *root, const BoundingBox<Number> &root_bounds)
{
    position_.bounding_box = root_bounds;
    position_.node = root;
    depth_ = 0;
}

template <typename NumberT, typename ObjectT>
int
detail::ForwardTreeTraversal<NumberT, ObjectT>::
GetDepth() const
{
    return depth_;
}

template <typename NumberT, typename ObjectT>
detail::TreeNode<ObjectT> *
detail::ForwardTreeTraversal<NumberT, ObjectT>::
GetNode() const
{
    return position_.node;
}

template <typename NumberT, typename ObjectT>
const BoundingBox<NumberT> &
detail::ForwardTreeTraversal<NumberT, ObjectT>::
GetNodeBoundingBox() const
{
    return position_.bounding_box;
}

template <typename NumberT, typename ObjectT>
void
detail::ForwardTreeTraversal<NumberT, ObjectT>::
GoTopLeft()
{
    BoundingBox<Number> &bbox = position_.bounding_box;
    bbox.width = (Number)((typename MakeDistance<Number>::Type)bbox.width / 2);
    bbox.height = (Number)((typename MakeDistance<Number>::Type)bbox.height / 2);
    position_.node = position_.node->top_left;
    assert(position_.node != nullptr);
    depth_++;
}

template <typename NumberT, typename ObjectT>
void
detail::ForwardTreeTraversal<NumberT, ObjectT>::
GoTopRight()
{
    BoundingBox<Number> &bbox = position_.bounding_box;
    Number right = (Number)(bbox.left + bbox.width);
    bbox.left = (Number)(bbox.left +
                         (Number)((typename MakeDistance<Number>::Type)bbox.width / 2));
    bbox.width = (Number)(right - bbox.left);
    bbox.height = (Number)((typename MakeDistance<Number>::Type)bbox.height / 2);
    position_.node = position_.node->top_right;
    assert(position_.node != nullptr);
    depth_++;
}

template <typename NumberT, typename ObjectT>
void
detail::ForwardTreeTraversal<NumberT, ObjectT>::
GoBottomRight()
{
    BoundingBox<Number> &bbox = position_.bounding_box;
    Number right = (Number)(bbox.left + bbox.width);
    bbox.left = (Number)(bbox.left +
                         (Number)((typename MakeDistance<Number>::Type)bbox.width / 2));
    bbox.width = (Number)(right - bbox.left);
    Number bottom = (Number)(bbox.top + bbox.height);
    bbox.top = (Number)(bbox.top +
                        (Number)((typename MakeDistance<Number>::Type)bbox.height / 2));
    bbox.height = (Number)(bottom - bbox.top);
    position_.node = position_.node->bottom_right;
    assert(position_.node != nullptr);
    depth_++;
}

template <typename NumberT, typename ObjectT>
void
detail::ForwardTreeTraversal<NumberT, ObjectT>::
GoBottomLeft()
{
    BoundingBox<Number> &bbox = position_.bounding_box;
    bbox.width = (Number)((typename MakeDistance<Number>::Type)bbox.width / 2);
    Number bottom = (Number)(bbox.top + bbox.height);
    bbox.top = (Number)(bbox.top +
                        (Number)((typename MakeDistance<Number>::Type)bbox.height / 2));
    bbox.height = (Number)(bottom - bbox.top);
    position_.node = position_.node->bottom_left;
    assert(position_.node != nullptr);
    depth_++;
}



template <typename NumberT, typename ObjectT>
void
detail::FullTreeTraversal<NumberT, ObjectT>::
StartAt(TreeNode<Object> *root, const BoundingBox<Number> &root_bounds)
{
    ForwardTreeTraversal<Number, Object>::StartAt(root, root_bounds);
    position_.current_child = ChildPosition::kNone;
    position_stack_.clear();
}

template <typename NumberT, typename ObjectT>
detail::ChildPosition
detail::FullTreeTraversal<NumberT, ObjectT>::
GetNodeCurrentChild() const
{
    return position_.current_child;
}

template <typename NumberT, typename ObjectT>
void
detail::FullTreeTraversal<NumberT, ObjectT>::
SetNodeCurrentChild(ChildPosition child_position)
{
    position_.current_child = child_position;
}

template <typename NumberT, typename ObjectT>
void
detail::FullTreeTraversal<NumberT, ObjectT>::
GoTopLeft()
{
    assert((size_t)depth_ == position_stack_.size());
    position_.current_child = ChildPosition::kTopLeft;
    position_stack_.emplace_back(position_);
    ForwardTreeTraversal<Number, Object>::GoTopLeft();
    position_.current_child = ChildPosition::kNone;
}

template <typename NumberT, typename ObjectT>
void
detail::FullTreeTraversal<NumberT, ObjectT>::
GoTopRight()
{
    assert((size_t)depth_ == position_stack_.size());
    position_.current_child = ChildPosition::kTopRight;
    position_stack_.emplace_back(position_);
    ForwardTreeTraversal<Number, Object>::GoTopRight();
    position_.current_child = ChildPosition::kNone;
}

template <typename NumberT, typename ObjectT>
void
detail::FullTreeTraversal<NumberT, ObjectT>::
GoBottomRight()
{
    assert((size_t)depth_ == position_stack_.size());
    position_.current_child = ChildPosition::kBottomRight;
    position_stack_.emplace_back(position_);
    ForwardTreeTraversal<Number, Object>::GoBottomRight();
    position_.current_child = ChildPosition::kNone;
}

template <typename NumberT, typename ObjectT>
void
detail::FullTreeTraversal<NumberT, ObjectT>::
GoBottomLeft()
{
    assert((size_t)depth_ == position_stack_.size());
    position_.current_child = ChildPosition::kBottomLeft;
    position_stack_.emplace_back(position_);
    ForwardTreeTraversal<Number, Object>::GoBottomLeft();
    position_.current_child = ChildPosition::kNone;
}

template <typename NumberT, typename ObjectT>
void
detail::FullTreeTraversal<NumberT, ObjectT>::
GoUp()
{
    assert((size_t)depth_ == position_stack_.size() && depth_ > 0);
    position_ = position_stack_.back();
    depth_--;
    position_stack_.pop_back();
}



template <typename NumberT, typename ObjectT, typename BoundingBoxExtractorT>
LooseQuadtree<NumberT, ObjectT, BoundingBoxExtractorT>::Query::Impl::
Impl() : quadtree_(nullptr), query_region_(0, 0, 0, 0),
    query_type_(QueryType::kEndOfQuery),
    free_ride_from_level_(LooseQuadtree<Number, Object, BoundingBoxExtractor>::Impl::kInternalMaxDepth)
{
}

template <typename NumberT, typename ObjectT, typename BoundingBoxExtractorT>
void
LooseQuadtree<NumberT, ObjectT, BoundingBoxExtractorT>::Query::Impl::
Acquire(typename LooseQuadtree<Number, Object, BoundingBoxExtractor>::Impl *quadtree,
        const BoundingBox<Number> *query_region, QueryType query_type)
{
    assert(IsAvailable());
    assert(query_type != QueryType::kEndOfQuery);
    quadtree_ = quadtree;
    query_region_ = *query_region;
    query_type_ = query_type;
    free_ride_from_level_ =
        LooseQuadtree<Number, Object, BoundingBoxExtractor>::Impl::kInternalMaxDepth;
    if(quadtree->root_ == nullptr)
        query_type_ = QueryType::kEndOfQuery;
    else
    {
        quadtree_->running_queries_++;
        traversal_.StartAt(quadtree->root_, quadtree->bounding_box_);
        object_iterator_ = traversal_.GetNode()->objects.before_begin();
        Next();
    }
}

template <typename NumberT, typename ObjectT, typename BoundingBoxExtractorT>
void
LooseQuadtree<NumberT, ObjectT, BoundingBoxExtractorT>::Query::Impl::
Release()
{
    assert(!IsAvailable());
    quadtree_ = nullptr;
}

template <typename NumberT, typename ObjectT, typename BoundingBoxExtractorT>
bool
LooseQuadtree<NumberT, ObjectT, BoundingBoxExtractorT>::Query::Impl::
IsAvailable() const
{
    return quadtree_ == nullptr;
}

template <typename NumberT, typename ObjectT, typename BoundingBoxExtractorT>
bool
LooseQuadtree<NumberT, ObjectT, BoundingBoxExtractorT>::Query::Impl::
EndOfQuery() const
{
    assert(!IsAvailable());
    return query_type_ == QueryType::kEndOfQuery;
}


template <typename NumberT, typename ObjectT, typename BoundingBoxExtractorT>
ObjectT *
LooseQuadtree<NumberT, ObjectT, BoundingBoxExtractorT>::Query::Impl::
GetCurrent() const
{
    assert(!IsAvailable());
    assert(!EndOfQuery());
    return *object_iterator_;
}

template <typename NumberT, typename ObjectT, typename BoundingBoxExtractorT>
void
LooseQuadtree<NumberT, ObjectT, BoundingBoxExtractorT>::Query::Impl::
Next()
{
    assert(!IsAvailable());
    assert(!EndOfQuery());
    do
    {
        object_iterator_before_ = object_iterator_;
        object_iterator_++;
        if(object_iterator_ == traversal_.GetNode()->objects.end())
        {
            do
            {
                switch(traversal_.GetNodeCurrentChild())
                {
                case detail::ChildPosition::kNone:
                    if(traversal_.GetNode()->top_left == nullptr)
                    {
                        traversal_.SetNodeCurrentChild(detail::ChildPosition::kTopLeft);
                        continue;
                    }
                    else
                        traversal_.GoTopLeft();
                    break;
                case detail::ChildPosition::kTopLeft:
                    if(traversal_.GetNode()->top_right == nullptr)
                    {
                        traversal_.SetNodeCurrentChild(detail::ChildPosition::kTopRight);
                        continue;
                    }
                    else
                        traversal_.GoTopRight();
                    break;
                case detail::ChildPosition::kTopRight:
                    if(traversal_.GetNode()->bottom_right == nullptr)
                    {
                        traversal_.SetNodeCurrentChild(detail::ChildPosition::kBottomRight);
                        continue;
                    }
                    else
                        traversal_.GoBottomRight();
                    break;
                case detail::ChildPosition::kBottomRight:
                    if(traversal_.GetNode()->bottom_left == nullptr)
                    {
                        traversal_.SetNodeCurrentChild(detail::ChildPosition::kBottomLeft);
                        continue;
                    }
                    else
                        traversal_.GoBottomLeft();
                    break;
                case detail::ChildPosition::kBottomLeft:

#ifndef NDEBUG
                    int running_queries = 0;
                    for(auto &query : quadtree_->query_pool_)
                    {
                        if(!query.IsAvailable() && !query.EndOfQuery()) running_queries++;
                    }
                    assert(running_queries == quadtree_->running_queries_);
#endif

                    //only run this if no parallel queries are running
                    if(traversal_.GetDepth() > quadtree_->maximal_depth_ &&
                       quadtree_->running_queries_ == 1)
                    {
                        typename detail::TreeNode<Object>::ObjectContainer &objects =
                            traversal_.GetNode()->objects;
                        auto iterator = objects.begin();
                        while(iterator != objects.end())
                        {
                            if(*iterator != nullptr)
                            {
                                quadtree_->Update(*iterator);
                                assert(*iterator == nullptr);
                            }
                            iterator++;
                        }
                        objects.clear();
                    }

                    if(traversal_.GetDepth() > 0)
                    {
                        bool remove_node = (traversal_.GetNode()->objects.empty() &&
                                            traversal_.GetNode()->top_left == nullptr &&
                                            traversal_.GetNode()->top_right == nullptr &&
                                            traversal_.GetNode()->bottom_right == nullptr &&
                                            traversal_.GetNode()->bottom_left == nullptr);
                        detail::TreeNode<Object> *node = traversal_.GetNode();
                        traversal_.GoUp();

                        // if the node is empty no other queries can be invalidated by deleting
                        if(remove_node)
                        {
                            switch(traversal_.GetNodeCurrentChild())
                            {
                            case detail::ChildPosition::kTopLeft:
                                traversal_.GetNode()->top_left = nullptr;
                                break;
                            case detail::ChildPosition::kTopRight:
                                traversal_.GetNode()->top_right = nullptr;
                                break;
                            case detail::ChildPosition::kBottomRight:
                                traversal_.GetNode()->bottom_right = nullptr;
                                break;
                            case detail::ChildPosition::kBottomLeft:
                                traversal_.GetNode()->bottom_left = nullptr;
                                break;
                            case detail::ChildPosition::kNone:
                                assert(false);
                            }
                            quadtree_->allocator_.Delete(node);
                        }

                        if(free_ride_from_level_ == traversal_.GetDepth() + 1)
                        {
                            free_ride_from_level_ =
                                LooseQuadtree<Number, Object,
                                BoundingBoxExtractor>::Impl::kInternalMaxDepth;
                        }
                        continue;
                    }
                    else
                    {
                        // if the root is empty no other queries can be invalidated by deleting
                        if(traversal_.GetNode()->objects.empty() &&
                           traversal_.GetNode()->top_left == nullptr &&
                           traversal_.GetNode()->top_right == nullptr &&
                           traversal_.GetNode()->bottom_right == nullptr &&
                           traversal_.GetNode()->bottom_left == nullptr)
                        {
                            assert(traversal_.GetNode() == quadtree_->root_);
                            assert(quadtree_->GetSize() == 0);
                            assert(quadtree_->object_pointers_.size() == 0);
                            quadtree_->allocator_.Delete(quadtree_->root_);
                            quadtree_->root_ = nullptr;
                            quadtree_->bounding_box_ = BoundingBox<Number>(0, 0, 0, 0);
                        }

                        quadtree_->running_queries_--;
                        query_type_ = QueryType::kEndOfQuery;
                        return;
                    }
                    break;
                }
                assert(traversal_.GetNodeCurrentChild() == detail::ChildPosition::kNone);
                {
                    FitType fit = FitType::kPartialFit;
                    if(traversal_.GetDepth() >= free_ride_from_level_ ||
                       (fit = CurrentNodeFits()) != FitType::kNoFit)
                    {
                        if(fit == FitType::kFreeRide)
                            free_ride_from_level_ = traversal_.GetDepth();
                    }
                    else
                    {
                        traversal_.SetNodeCurrentChild(detail::ChildPosition::kBottomLeft);
                        continue;
                    }
                }
                object_iterator_ = traversal_.GetNode()->objects.before_begin();
                break;
            }
            while(true);
        }
        else if(*object_iterator_ == nullptr)
        {
            // other queries also cannot stop on an empty object,
            // so we are not invalidating any iterators/queries with this
            traversal_.GetNode()->objects.erase_after(object_iterator_before_);
            object_iterator_ = object_iterator_before_;
        }
        else
        {
            if(traversal_.GetDepth() >= free_ride_from_level_ ||
               CurrentObjectFits())
                break;
        }
    }
    while(true);
}

template <typename NumberT, typename ObjectT, typename BoundingBoxExtractorT>
bool
LooseQuadtree<NumberT, ObjectT, BoundingBoxExtractorT>::Query::Impl::
CurrentObjectFits() const
{
    BoundingBox<Number> object_bounds(0, 0, 0, 0);
    BoundingBoxExtractor::ExtractBoundingBox(GetCurrent(), &object_bounds);
    switch(query_type_)
    {
    case QueryType::kIntersects:
        return query_region_.Intersects(object_bounds);
    case QueryType::kInside:
        return query_region_.Contains(object_bounds);
    case QueryType::kContains:
        return object_bounds.Contains(query_region_);
    case QueryType::kEndOfQuery:
        assert(false);
    }
    return false;
}

template <typename NumberT, typename ObjectT, typename BoundingBoxExtractorT>
auto
LooseQuadtree<NumberT, ObjectT, BoundingBoxExtractorT>::Query::Impl::
CurrentNodeFits() const -> FitType
{
    const BoundingBox<Number> &node_bounds = traversal_.GetNodeBoundingBox();
    BoundingBox<Number> extended_bounds = node_bounds;
    Number half_width =
        (Number)((typename detail::MakeDistance<Number>::Type)node_bounds.width / 2);
    Number half_height =
        (Number)((typename detail::MakeDistance<Number>::Type)node_bounds.height / 2);
    extended_bounds.width = (Number)(extended_bounds.width * 2);
    extended_bounds.height = (Number)(extended_bounds.height * 2);
    extended_bounds.left = (Number)(extended_bounds.left - half_width);
    extended_bounds.top = (Number)(extended_bounds.top - half_height);
    switch(query_type_)
    {
    case QueryType::kIntersects:
        if(!query_region_.Intersects(extended_bounds))
            return FitType::kNoFit;
        else if(query_region_.Contains(node_bounds))
            return FitType::kFreeRide;
        return FitType::kPartialFit;
    case QueryType::kInside:
        if(!query_region_.Intersects(node_bounds))
            return FitType::kNoFit;
        else if(query_region_.Contains(extended_bounds))
            return FitType::kFreeRide;
        return FitType::kPartialFit;
    case QueryType::kContains:
        if(!extended_bounds.Contains(query_region_))
            return FitType::kNoFit;
        return FitType::kPartialFit;
    case QueryType::kEndOfQuery:
        assert(false);
    }
    return FitType::kNoFit;
}



template <typename NumberT, typename ObjectT, typename BoundingBoxExtractorT>
LooseQuadtree<NumberT, ObjectT, BoundingBoxExtractorT>::Impl::
Impl() :
    root_(nullptr), bounding_box_(0, 0, 0, 0),
    object_pointers_(64, std::hash<Object*>(), std::equal_to<Object*>(),
                     detail::BlocksAllocatorAdaptor<std::pair<const Object *, Object * *>>(allocator_)),
    number_of_objects_(0), maximal_depth_(kInternalMinDepth),
    running_queries_(0)
{
    assert(maximal_depth_ < kInternalMaxDepth);
    //object_pointers_.reserve(64);
}

template <typename NumberT, typename ObjectT, typename BoundingBoxExtractorT>
LooseQuadtree<NumberT, ObjectT, BoundingBoxExtractorT>::Impl::
~Impl()
{
    DeleteTree();
}

template <typename NumberT, typename ObjectT, typename BoundingBoxExtractorT>
bool
LooseQuadtree<NumberT, ObjectT, BoundingBoxExtractorT>::Impl::
Insert(Object *object)
{
    bool was_removed = Remove(object);
    Object **place = InsertIntoTree(object);
    object_pointers_.emplace(object, place);
    number_of_objects_++;
    RecalculateMaximalDepth();
    return !was_removed;
}

template <typename NumberT, typename ObjectT, typename BoundingBoxExtractorT>
bool
LooseQuadtree<NumberT, ObjectT, BoundingBoxExtractorT>::Impl::
Update(Object *object)
{
    return !Insert(object);
}

template <typename NumberT, typename ObjectT, typename BoundingBoxExtractorT>
bool
LooseQuadtree<NumberT, ObjectT, BoundingBoxExtractorT>::Impl::
Remove(Object *object)
{
    auto it = object_pointers_.find(object);
    if(it != object_pointers_.end())
    {
        assert(*(it->second) == it->first);
        *(it->second) = nullptr;
        object_pointers_.erase(it);
        number_of_objects_--;
        RecalculateMaximalDepth();
        return true;
    }
    return false;
}

template <typename NumberT, typename ObjectT, typename BoundingBoxExtractorT>
bool
LooseQuadtree<NumberT, ObjectT, BoundingBoxExtractorT>::Impl::
Contains(Object *object) const
{
    return object_pointers_.find(object) != object_pointers_.end();
}

template <typename NumberT, typename ObjectT, typename BoundingBoxExtractorT>
auto
LooseQuadtree<NumberT, ObjectT, BoundingBoxExtractorT>::Impl::
QueryIntersectsRegion(const BoundingBox<Number> &region) -> Query
{
    typename Query::Impl *query_impl = GetAvailableQueryFromPool();
    query_impl->Acquire(this, &region, Query::Impl::QueryType::kIntersects);
    return Query(query_impl);
}

template <typename NumberT, typename ObjectT, typename BoundingBoxExtractorT>
auto
LooseQuadtree<NumberT, ObjectT, BoundingBoxExtractorT>::Impl::
QueryInsideRegion(const BoundingBox<Number> &region) -> Query
{
    typename Query::Impl *query_impl = GetAvailableQueryFromPool();
    query_impl->Acquire(this, &region, Query::Impl::QueryType::kInside);
    return Query(query_impl);
}

template <typename NumberT, typename ObjectT, typename BoundingBoxExtractorT>
auto
LooseQuadtree<NumberT, ObjectT, BoundingBoxExtractorT>::Impl::
QueryContainsRegion(const BoundingBox<Number> &region) -> Query
{
    typename Query::Impl *query_impl = GetAvailableQueryFromPool();
    query_impl->Acquire(this, &region, Query::Impl::QueryType::kContains);
    return Query(query_impl);
}

template <typename NumberT, typename ObjectT, typename BoundingBoxExtractorT>
const BoundingBox<NumberT> &
LooseQuadtree<NumberT, ObjectT, BoundingBoxExtractorT>::Impl::
GetBoundingBox() const
{
    return bounding_box_;
}

template <typename NumberT, typename ObjectT, typename BoundingBoxExtractorT>
void
LooseQuadtree<NumberT, ObjectT, BoundingBoxExtractorT>::Impl::
ForceCleanup()
{
    Query query = QueryIntersectsRegion(bounding_box_);
    while(!query.EndOfQuery())
        query.Next();
    allocator_.ReleaseFreeBlocks();
}

template <typename NumberT, typename ObjectT, typename BoundingBoxExtractorT>
int
LooseQuadtree<NumberT, ObjectT, BoundingBoxExtractorT>::Impl::
GetSize() const
{
    return number_of_objects_;
}

template <typename NumberT, typename ObjectT, typename BoundingBoxExtractorT>
void
LooseQuadtree<NumberT, ObjectT, BoundingBoxExtractorT>::Impl::
Clear()
{
    DeleteTree();
}



template <typename NumberT, typename ObjectT, typename BoundingBoxExtractorT>
void
LooseQuadtree<NumberT, ObjectT, BoundingBoxExtractorT>::Impl::
RecalculateMaximalDepth()
{
    do
    {
        if(maximal_depth_ < kInternalMaxDepth &&
           number_of_objects_ > 1ll << (maximal_depth_ << 1))
            maximal_depth_++;
        else if(maximal_depth_ > kInternalMinDepth &&
                number_of_objects_ <= 1ll << ((maximal_depth_ - 1) << 1))
            maximal_depth_--;
        else
            break;
    }
    while(true);
}

template <typename NumberT, typename ObjectT, typename BoundingBoxExtractorT>
void
LooseQuadtree<NumberT, ObjectT, BoundingBoxExtractorT>::Impl::
DeleteTree()
{
    object_pointers_.clear();
    detail::FullTreeTraversal<Number, Object> &trav = internal_traversal_;
    trav.StartAt(root_, bounding_box_);
    while(root_ != nullptr)
    {
        assert(trav.GetDepth() >= 0 && trav.GetDepth() <= kInternalMaxDepth);
        detail::TreeNode<Object> *node = trav.GetNode();
        if(node->top_left != nullptr)
            trav.GoTopLeft();
        else if(node->top_right != nullptr)
            trav.GoTopRight();
        else if(node->bottom_right != nullptr)
            trav.GoBottomRight();
        else if(node->bottom_left != nullptr)
            trav.GoBottomLeft();
        else
        {
            if(trav.GetDepth() > 0)
            {
                trav.GoUp();
                switch(trav.GetNodeCurrentChild())
                {
                case detail::ChildPosition::kNone:
                    assert(false);
                    break;
                case detail::ChildPosition::kTopLeft:
                    assert(node == trav.GetNode()->top_left);
                    trav.GetNode()->top_left = nullptr;
                    break;
                case detail::ChildPosition::kTopRight:
                    assert(node == trav.GetNode()->top_right);
                    trav.GetNode()->top_right = nullptr;
                    break;
                case detail::ChildPosition::kBottomRight:
                    assert(node == trav.GetNode()->bottom_right);
                    trav.GetNode()->bottom_right = nullptr;
                    break;
                case detail::ChildPosition::kBottomLeft:
                    assert(node == trav.GetNode()->bottom_left);
                    trav.GetNode()->bottom_left = nullptr;
                    break;
                }
                allocator_.Delete(node);
            }
            else
            {
                assert(node == root_);
                allocator_.Delete(root_);
                root_ = nullptr;
            }
        }
    }

    bounding_box_ = BoundingBox<Number>(0, 0, 0, 0);
    number_of_objects_ = 0;
    maximal_depth_ = kInternalMinDepth;
}

template <typename NumberT, typename ObjectT, typename BoundingBoxExtractorT>
ObjectT **
LooseQuadtree<NumberT, ObjectT, BoundingBoxExtractorT>::Impl::
InsertIntoTree(Object *object)
{
    BoundingBox<Number> object_bounds(0, 0, 0, 0);
    BoundingBoxExtractor::ExtractBoundingBox(object, &object_bounds);
    assert(object_bounds.width >= 0);
    assert(object_bounds.height >= 0);
    assert(object_bounds.left <= object_bounds.left + object_bounds.width);
    assert(object_bounds.top <= object_bounds.top + object_bounds.height);
    Number maximal_object_extent = object_bounds.width >= object_bounds.height ?
                                   object_bounds.width : object_bounds.height;
    if(maximal_object_extent < kMinimalObjectExtent)
        maximal_object_extent = kMinimalObjectExtent;
    Number object_center_x = (Number)(object_bounds.left +
                                      (Number)((typename detail::MakeDistance<Number>::Type)object_bounds.width / 2));
    Number object_center_y = (Number)(object_bounds.top +
                                      (Number)((typename detail::MakeDistance<Number>::Type)object_bounds.height / 2));

    if(root_ != nullptr)
    {
        assert(number_of_objects_ >= 0);
        assert(bounding_box_.width > 0);
        assert(bounding_box_.width == bounding_box_.height);

        int depth_increase = 0;
        while(!bounding_box_.Contains(object_center_x, object_center_y) ||
              maximal_object_extent > bounding_box_.width)
        {
            Number previous_size = bounding_box_.width;
            bounding_box_.width = (Number)(bounding_box_.width * 2);
            bounding_box_.height = bounding_box_.width;
            Number previous_half =
                (Number)((typename detail::MakeDistance<Number>::Type)previous_size / 2);
            Number bb_center_x = (Number)(bounding_box_.left + previous_half);
            Number bb_center_y = (Number)(bounding_box_.top + previous_half);
            detail::TreeNode<Object> *old_root = root_;
            root_ = allocator_.New<detail::TreeNode<Object>>(allocator_);
            if(object_center_x <= bb_center_x)
            {
                bounding_box_.left = (Number)(bounding_box_.left - previous_size);
                if(object_center_y <= bb_center_y)
                {
                    bounding_box_.top = (Number)(bounding_box_.top - previous_size);
                    root_->bottom_right = old_root;
                }
                else
                    root_->top_right = old_root;
            }
            else
            {
                if(object_center_y <= bb_center_y)
                {
                    bounding_box_.top = (Number)(bounding_box_.top - previous_size);
                    root_->bottom_left = old_root;
                }
                else
                    root_->top_left = old_root;
            }
            depth_increase++;
            assert(depth_increase < kInternalMaxDepth);
            (void)depth_increase;
            assert(bounding_box_.left < bounding_box_.left + bounding_box_.width);
            assert(bounding_box_.top < bounding_box_.top + bounding_box_.height);
            // If this happens with integral types you are close to get out of bounds
            // The bounding box of things should be at least 1/8 of the total interval spanned
            assert(!std::is_integral<Number>::value ||
                   bounding_box_.width < std::numeric_limits<Number>::max() / 8 * 7);
            assert(!std::is_integral<Number>::value ||
                   bounding_box_.height < std::numeric_limits<Number>::max() / 8 * 7);
        }

        detail::ForwardTreeTraversal<Number, Object> trav;
        trav.StartAt(root_, bounding_box_);
        do
        {
            const BoundingBox<Number> &node_bounds = trav.GetNodeBoundingBox();
            assert(node_bounds.Contains(object_center_x, object_center_y));
            Number maximal_bb_extent =
                node_bounds.width >= node_bounds.height ?
                node_bounds.width : node_bounds.height;
            Number half_bb_extent =
                (Number)((typename detail::MakeDistance<Number>::Type)maximal_bb_extent / 2);
            assert(maximal_object_extent <= maximal_bb_extent);

            if(maximal_object_extent > half_bb_extent ||
               trav.GetDepth() >= maximal_depth_)
                break;

            Number node_center_x = (Number)(node_bounds.left +
                                            (Number)((typename detail::MakeDistance<Number>::Type)node_bounds.width / 2));
            Number node_center_y = (Number)(node_bounds.top +
                                            (Number)((typename detail::MakeDistance<Number>::Type)node_bounds.height / 2));

            detail::TreeNode<Object> **direction;
            if(object_center_x < node_center_x)
            {
                if(object_center_y < node_center_y)
                    direction = &trav.GetNode()->top_left;
                else
                    direction = &trav.GetNode()->bottom_left;
            }
            else
            {
                if(object_center_y < node_center_y)
                    direction = &trav.GetNode()->top_right;
                else
                    direction = &trav.GetNode()->bottom_right;
            }

            if(*direction == nullptr)
                *direction = allocator_.New<detail::TreeNode<Object>>(allocator_);

            if(*direction == trav.GetNode()->top_left)
                trav.GoTopLeft();
            else if(*direction == trav.GetNode()->top_right)
                trav.GoTopRight();
            else if(*direction == trav.GetNode()->bottom_right)
                trav.GoBottomRight();
            else
            {
                assert(*direction == trav.GetNode()->bottom_left);
                trav.GoBottomLeft();
            }
        }
        while(true);

#ifndef NDEBUG
        BoundingBox<Number> effective_bounds = trav.GetNodeBoundingBox();
        Number half_width =
            (Number)((typename detail::MakeDistance<Number>::Type)effective_bounds.width / 2);
        Number half_height =
            (Number)((typename detail::MakeDistance<Number>::Type)effective_bounds.height / 2);
        effective_bounds.width = (Number)(effective_bounds.width * 2);
        effective_bounds.height = (Number)(effective_bounds.height * 2);
        effective_bounds.left = (Number)(effective_bounds.left - half_width);
        effective_bounds.top = (Number)(effective_bounds.top - half_height);
        assert(effective_bounds.Contains(object_bounds));
#endif

        typename detail::TreeNode<Object>::ObjectContainer &objects =
            trav.GetNode()->objects;
        if(!objects.empty() && objects.front() == nullptr)
            objects.front() = object;
        else
            objects.emplace_front(object);
        return &objects.front();
    }
    else
    {
        assert(number_of_objects_ == 0);
        {
            bounding_box_.width =
                (Number)((typename detail::MakeDistance<Number>::Type)maximal_object_extent * 2 * 7 / 8);
            bounding_box_.height = bounding_box_.width;
            Number extent_half =
                (Number)((typename detail::MakeDistance<Number>::Type)bounding_box_.width / 2);
            bounding_box_.left = (Number)(object_center_x - extent_half);
            bounding_box_.top = (Number)(object_center_y - extent_half);
            assert(bounding_box_.left < bounding_box_.left + bounding_box_.width);
            assert(bounding_box_.top < bounding_box_.top + bounding_box_.height);
        }
        root_ = allocator_.New<detail::TreeNode<Object>>(allocator_);
        root_->objects.emplace_front(object);
        return &root_->objects.front();
    }
}

template <typename NumberT, typename ObjectT, typename BoundingBoxExtractorT>
auto
LooseQuadtree<NumberT, ObjectT, BoundingBoxExtractorT>::Impl::
GetAvailableQueryFromPool() -> typename Query::Impl *
{
    for(auto it = query_pool_.begin(); it != query_pool_.end(); it++)
    {
        if(it->IsAvailable())
            return &*it;
    }
    query_pool_.emplace_back();
    assert(query_pool_.back().IsAvailable());
    return &query_pool_.back();
}



template <typename NumberT, typename ObjectT, typename BoundingBoxExtractorT>
bool
LooseQuadtree<NumberT, ObjectT, BoundingBoxExtractorT>::
Insert(Object *object)
{
    return impl_.Insert(object);
}

template <typename NumberT, typename ObjectT, typename BoundingBoxExtractorT>
bool
LooseQuadtree<NumberT, ObjectT, BoundingBoxExtractorT>::
Update(Object *object)
{
    return impl_.Update(object);
}

template <typename NumberT, typename ObjectT, typename BoundingBoxExtractorT>
bool
LooseQuadtree<NumberT, ObjectT, BoundingBoxExtractorT>::
Remove(Object *object)
{
    return impl_.Remove(object);
}

template <typename NumberT, typename ObjectT, typename BoundingBoxExtractorT>
bool
LooseQuadtree<NumberT, ObjectT, BoundingBoxExtractorT>::
Contains(Object *object) const
{
    return impl_.Contains(object);
}

template <typename NumberT, typename ObjectT, typename BoundingBoxExtractorT>
auto
LooseQuadtree<NumberT, ObjectT, BoundingBoxExtractorT>::
QueryIntersectsRegion(const BoundingBox<Number> &region) -> Query
{
    return impl_.QueryIntersectsRegion(region);
}

template <typename NumberT, typename ObjectT, typename BoundingBoxExtractorT>
auto
LooseQuadtree<NumberT, ObjectT, BoundingBoxExtractorT>::
QueryInsideRegion(const BoundingBox<Number> &region) -> Query
{
    return impl_.QueryInsideRegion(region);
}

template <typename NumberT, typename ObjectT, typename BoundingBoxExtractorT>
auto
LooseQuadtree<NumberT, ObjectT, BoundingBoxExtractorT>::
QueryContainsRegion(const BoundingBox<Number> &region) -> Query
{
    return impl_.QueryContainsRegion(region);
}

template <typename NumberT, typename ObjectT, typename BoundingBoxExtractorT>
const BoundingBox<NumberT> &
LooseQuadtree<NumberT, ObjectT, BoundingBoxExtractorT>::
GetLooseBoundingBox() const
{
    return impl_.GetBoundingBox();
}

template <typename NumberT, typename ObjectT, typename BoundingBoxExtractorT>
void
LooseQuadtree<NumberT, ObjectT, BoundingBoxExtractorT>::
ForceCleanup()
{
    impl_.ForceCleanup();
}

template <typename NumberT, typename ObjectT, typename BoundingBoxExtractorT>
int
LooseQuadtree<NumberT, ObjectT, BoundingBoxExtractorT>::
GetSize() const
{
    return impl_.GetSize();
}

template <typename NumberT, typename ObjectT, typename BoundingBoxExtractorT>
bool
LooseQuadtree<NumberT, ObjectT, BoundingBoxExtractorT>::
IsEmpty() const
{
    return impl_.GetSize() == 0;
}

template <typename NumberT, typename ObjectT, typename BoundingBoxExtractorT>
void
LooseQuadtree<NumberT, ObjectT, BoundingBoxExtractorT>::
Clear()
{
    impl_.Clear();
}



template <typename NumberT, typename ObjectT, typename BoundingBoxExtractorT>
LooseQuadtree<NumberT, ObjectT, BoundingBoxExtractorT>::Query::
Query(Impl *pimpl) : pimpl_(pimpl)
{
    assert(pimpl_ != nullptr);
    assert(!pimpl_->IsAvailable());
}

template <typename NumberT, typename ObjectT, typename BoundingBoxExtractorT>
LooseQuadtree<NumberT, ObjectT, BoundingBoxExtractorT>::Query::
~Query()
{
    if(pimpl_ != nullptr)
    {
        pimpl_->Release();
        assert(pimpl_->IsAvailable());
        pimpl_ = nullptr;
    }
}

template <typename NumberT, typename ObjectT, typename BoundingBoxExtractorT>
LooseQuadtree<NumberT, ObjectT, BoundingBoxExtractorT>::Query::
Query(Query &&other) : pimpl_(other.pimpl_)
{
    other.pimpl_ = nullptr;
}

template <typename NumberT, typename ObjectT, typename BoundingBoxExtractorT>
auto
LooseQuadtree<NumberT, ObjectT, BoundingBoxExtractorT>::Query::
operator=(Query &&other) -> Query &
{
    this->~Query();
    pimpl_ = other.pimpl_;
    other.pimpl_ = nullptr;
    return *this;
}

template <typename NumberT, typename ObjectT, typename BoundingBoxExtractorT>
bool
LooseQuadtree<NumberT, ObjectT, BoundingBoxExtractorT>::Query::
EndOfQuery() const
{
    return pimpl_->EndOfQuery();
}


template <typename NumberT, typename ObjectT, typename BoundingBoxExtractorT>
ObjectT *
LooseQuadtree<NumberT, ObjectT, BoundingBoxExtractorT>::Query::
GetCurrent() const
{
    return pimpl_->GetCurrent();
}

template <typename NumberT, typename ObjectT, typename BoundingBoxExtractorT>
void
LooseQuadtree<NumberT, ObjectT, BoundingBoxExtractorT>::Query::
Next()
{
    pimpl_->Next();
}



} //loose_quadtree

#endif //LOOSEQUADTREE_LOOSEQUADTREE_IMPL_H
