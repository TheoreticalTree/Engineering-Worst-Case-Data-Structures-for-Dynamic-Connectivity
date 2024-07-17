#ifndef GKKT_CUTSET_HPP
#define GKKT_CUTSET_HPP

#include <vector>

#include "base.hpp"
#include "ETForestCutSet.hpp"
#include "AVLTree.hpp"
#include "TabularHash.hpp"

constexpr uint blockSize = 2;

class CutSet{
public:
    /**
     * Creates a cutset data structure
     * @param pn the number of nodes
     * @param pBoostLevel boost level, guarantees find has chance at least 1 - (7/8)^boostLevel to succeed
     * @param pNumLevels number of levels
     */
    CutSet(count pn, count pBoostLevel, count pNumLevels, count seed, const std::vector<AVLTree<node, bool>> &pAdjacencyTrees);

    ~CutSet();

    /**
     * Adds the edge @e = {v,w} to the sets of v and w on all levels
     * @param e the edge
     */
    void addEdgeToSet(edge e);

    /**
     * Inserts the edge @e on all levels above @level
     * @param e the edge, which must connect two unconnected components on all levels
     * @param level the lowest level on which @e is supposed to be inserted
     */
    void makeTreeEdge(edge e, count level);

    /**
     * Turns @e into a nontree edge on all levels on which it is a tree edge
     * @param e the edge
     */
    void makeNonTreeEdge(edge e);

    /**
     * Removes the edge @e = {v,w} from the sets of v and w on all levels
     * If @e is a tree edge it is removed from all levels on which it is a tree edge
     * @param e the edge
     */
    void deleteEdge(edge e);

    /**
     * Looks for any edge leaving the tree of @v on level @level
     * If any such edge exists one is returned with probability >= 1/8
     * @param v the node
     * @param level the level on which we are looking
     * @return an edge leaving the tree of @v on level @level with probability 1/8 (if any exists), (0,0) otherwise
     */
    edge search(node v, count level);

    /**
     * Get a representative of the connected component of @v on level @level
     * The representatives is the same for all nodes in the component and only changes when a tree edge is inserted or deleted at this level
     * @param v the node
     * @param level the level on which we seek the representative
     * @return a representative of the connected component of @v on level @level
     */
    node compRepresentative(node v, count level);

    /**
     * Get the size of the component of @v on level @level
     * @param v the node v
     * @param level the level
     * @return the size
     */
    count compSize(node v, count level);

protected:
    count numLevels = 0;
    count boostLevel = 0;
    count n = 0;
    count lognsqr = 0;

    //! Stores the adjacent edges of a node in AVLTrees
    const std::vector<AVLTree<node, bool>> &adjacencyTrees;

    //! Stores the tree edges and the active edge of every node for every level [level][vertex]
    std::vector<std::vector<AVLTree<node, address>>> treeEdges;
    std::vector<std::vector<address>> activeEdges;

    //! The levels of ETTrees
    std::vector<ETForestCutSet> forests;

    //! For every node this stores the xored edge sets with index [level][vertex][boostLine][inBoostLine]
    std::vector<std::vector<std::vector<std::vector<edge>>>> xOrEdgeVectors;

    //! For every level this stores the necessary hash functions with index [level][boostLine]
    std::vector<std::vector<TabularHash>> hashes;

    //! Gets a new active instance for @v
    void refreshActiveInstance(count level, node v);
};

#endif //GKKT_CUTSET_HPP
