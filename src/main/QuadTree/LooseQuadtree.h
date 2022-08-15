/**
 * LooseQuadtree written by Zozo
 * use freely under MIT license
 *
 * Loose quadtree (unlike normal quadtrees which are for points only) is
 * a region tree designed to store bounding boxes effectively
 * See boost::geometry::index::rtree for a more advanced, general solution!
 *
 * This implementation features:
 * - Fully adaptive behavior, adjusts its bounds, height and memory on demand
 * - Every object will be stored on the level to which its size corresponds to
 * - Gives theoretically optimal search results (see previous)
 * - Uses tree structure instead of hashed (smaller memory footprint, cache friendly)
 * - Uses as much data in-place as it can (by using its own allocator)
 * - Allocates memory in big chunks
 * - Uses axis-aligned bounding boxes for calculations
 * - Uses left-top-width-height bounds for better precision (no right-bottom)
 * - Uses left-top closed right-bottom open interval logic (for integral types)
 * - Uses X-towards-right Y-towards-bottom screen-like coordinate system
 * - It is suitable for both floating- and fixed-point logic
 * - This library is not thread-safe but multiple queries can be run at once
 *
 * Generic parameters are:
 * - NumberT generic number type allows its floating- and fixed-point usage
 * - ObjectT only reference is stored, no object copying is done, not an inclusive container
 * - BoundingBoxExtractorT allows using your own bounding box type/source, needs
 *     BoundingBoxExtractor::ExtractBoundingBox(ObjectT in, BoundingBox<Number>* out) implemented
 */

#ifndef LOOSEQUADTREE_LOOSEQUADTREE_H
#define LOOSEQUADTREE_LOOSEQUADTREE_H

namespace loose_quadtree
{



template <typename NumberT>
struct BoundingBox
{
    using Number = NumberT;

    BoundingBox(Number _left, Number _top, Number _width, Number _height) :
        left(_left), top(_top), width(_width), height(_height) {}
    bool Intersects(const BoundingBox<Number> &other) const ;
    bool Contains(const BoundingBox<Number> &other) const;
    bool Contains(Number x, Number y) const;

    Number left;
    Number top;
    Number width;
    Number height;
};



template <typename NumberT, typename ObjectT, typename BoundingBoxExtractorT>
class LooseQuadtree
{
public:
    using Number = NumberT;
    using Object = ObjectT;
    using BoundingBoxExtractor = BoundingBoxExtractorT;

private:
    class Impl;

public:
    class Query
    {
    public:
        ~Query();
        Query() = delete;
        Query(const Query &) = delete;
        Query &operator=(const Query &) = delete;
        Query(Query &&);
        Query &operator=(Query &&);

        bool EndOfQuery() const;
        Object GetCurrent() const;
        void Next();

    private:
        friend class LooseQuadtree<Number, Object, BoundingBoxExtractor>::Impl;
        class Impl;
        Query(Impl *pimpl);
        Impl *pimpl_;
    };

    LooseQuadtree() {}
    ~LooseQuadtree() {}
    LooseQuadtree(const LooseQuadtree &) = delete;
    LooseQuadtree &operator=(const LooseQuadtree &) = delete;

    bool Insert(Object object); ///< true if it was inserted (else updated)
    bool Update(Object object); ///< true if it was updated (else inserted)
    bool Remove(Object object); ///< true if it was removed
    bool Contains(Object object) const; ///< true if object is in tree
    Query QueryIntersectsRegion(const BoundingBox<Number> &region);
    Query QueryInsideRegion(const BoundingBox<Number> &region);
    Query QueryContainsRegion(const BoundingBox<Number> &region);
    const BoundingBox<Number> &GetLooseBoundingBox() const;
    ///< double its size to get a bounding box including everything contained for sure
    int GetSize() const;
    bool IsEmpty() const;
    void Clear();
    void ForceCleanup(); ///< does a full data structure and memory cleanup
    ///< cleanup is semi-automatic during queries so you needn't call this normally

private:
    Impl impl_;
};



} //loose_quadtree

#include "LooseQuadtree-impl.tcc"

#endif //LOOSEQUADTREE_LOOSEQUADTREE_H
