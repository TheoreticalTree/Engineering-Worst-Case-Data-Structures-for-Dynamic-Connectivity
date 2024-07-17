#ifndef GKKT_QUERYFORESTAVL_HPP
#define GKKT_QUERYFORESTAVL_HPP

#include <vector>

#include "base.hpp"
#include "QueryForest.hpp"
#include "AVLTree.hpp"
#include "ETForestCutSet.hpp"

class QueryForestAVL : public QueryForest {
public:
    QueryForestAVL(count n);

    /**
     * Queries if @u and @v are connected in the forest in O(log(n))
     * @param u node u
     * @param v node v
     * @return true iff u and v are connected in the forest
     */
    bool query(node u, node v) const override;

    /**
     * Find the size of the tree containing @v
     * @param v the node
     * @return the size of the tree containing @v
     */
    count compSize(node v) const override;

    /**
     * Find a representative of the tree of @v. So long as no edges change the representative is the same for all nodes in the tree
     * @param v the node
     * @return a node in the tree of @v representing it
     */
    count compRepresentative(node v) const override;

    /**
     * Get the number of connected components
     * @return the number of connected components
     */
    count numberOfComponents() const override;

    /**
     * Check if the edge {u, v} is a tree edge
     * @param u
     * @param v
     * @return true iff {u, v} is a tree edge
     */
    bool isTreeEdge(node u, node v) const override;

    /**
     * Adds edge {@u,@v} to the forest (the two must not have been connected before)
     * @param u node u
     * @param v node v
     */
    void addEdge(node u, node v) override;

    /**
     * Removes the edge {u,v} from the forest
     * @param u node u
     * @param v node v
     */
    void deleteEdge(node u, node v) override;

protected:
    count n;
    count numConnectedComponents;

    std::vector<AVLTree<node, address>> treeEdges;

    ETForestCutSet forest;
};

#endif //GKKT_QUERYFORESTAVL_HPP
