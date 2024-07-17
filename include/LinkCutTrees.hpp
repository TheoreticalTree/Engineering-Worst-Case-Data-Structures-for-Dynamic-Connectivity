#ifndef GKKT_LINKCUTTREES_HPP
#define GKKT_LINKCUTTREES_HPP

#include <vector>

#include "base.hpp"
#include "BiasedBinaryForest.hpp"
#include "BiasedBinaryTree.hpp"
#include "BiasedBinaryHeap.hpp"

using Path = BBTNode*;

/**
 * The Link-Cut-Trees data structure by Sleator and Tarjan
 * This implementation is the one for O(log(n)) runtime guaranteed (no amortisation necessary)
 */
class LinkCutTrees {
public:
    explicit LinkCutTrees(count pn);
    ~LinkCutTrees();

    /**
     * Inserts an edge (v,w) with cost x therefore attaching the tree of v to the tree of w
     * Declares v the root of its tree before the join
     * @param v node v
     * @param w node w
     * @param x the cost of the new edge
     */
    void link(node v, node w, cost x);

    /**
     * Cuts the edge between v and its parent
     * @param v node v, which must not be a root
     * @return the cost of the deleted edge
     */
    cost cut(node v);

    /**
     * Cuts the specific edge (v,w)
     * @param v node v
     * @param w node w
     * @return the cost of the deleted edge
     */
    cost cutEdge(node v, node w);

    /**
     * Turns @v into the root of its subtree. Called evert in the original paper
     * @param v node v that shall become the new root
     */
    void reroot(node v);

    /**
     * Get the parent in the current version of the rooted tree
     * @param v node v
     * @return the parent of @v, none if @v is a root
     */
    node getParent(node v);

    /**
     * Get the root of node @v
     * @param v node v
     * @return the root of the tree containing @v
     */
    node getRoot(node v);

    /**
     * Get the cost of edge (v,parent(v)).
     * @param v node v
     * @return the cost of edge (v,parent(v)), 0 if no edge exists
     */
    cost getCost(node v);

    /**
     * Get the minimum cost edge of all edges on the path v ~> root(v)
     * If there are multiple returns the one closest to the root
     * @param v node v
     * @return the minimum cost of all edges on the path to the root, ((none, none),0) if v is the root
     */
    std::pair<std::pair<node, node>, cost> getMinEdge(node v);

    /**
     * Get the maximum cost edge of all edges on the path v ~> root(v)
     * If there are multiple returns the one closest to the root
     * @param v node v
     * @return the minimum cost of all edges on the path to the root, ((none, none),0) if v is the root
     */
    std::pair<std::pair<node, node>, cost> getMaxEdge(node v);

    /**
     * Adds @x to the cost of all edges between v and root(v)
     * @param v node v
     * @param x the change to the edge costs
     */
    void update(node v, diff x);

protected:
    //! number of nodes
    count n;
    //! to operate on all the trees
    BiasedBinaryForest forest;
    //! if a node is at the end of its solid sequence this stores the parent node, none if v is root
    std::vector<node> parent;
    //! if a node is at the end of its solid sequence this stores the cost of (v, parent(v))
    std::vector<cost> pCost;
    //! Stores the nodes representation in paths
    std::vector<Path> pathNodes;
    //! Store all the paths connected to v by light edges sorted by weight
    std::vector<BiasedBinaryTree<std::pair<count, node>, Path>> pathSets;

    /**
     * Extends the solid path @path by turning the dashed edge leaving its end solid
     * @param path the path, which must have a dashed edge leaving its end
     * @return the new, longer path
     */
    Path splice(Path path);

    /**
     * Creates a solid path between @v and the root of v
     * @param v the evrtex v
     * @return the solid path
     */
    Path expose(node v);

    /**
     * Finds the edge closest to the end of @path that should be dashed and turns it dashed
     * @param path the path
     * @return the path left of the newly dashed edge
     */
    Path slice(Path path);

    /**
     * Reestablishes the proper dashed/solid-order on @path
     * @param path the path
     */
    void conceal(Path path);

    void refreshPathEntry(Path p);

    void checkForestValidity();
};

#endif //GKKT_LINKCUTTREES_HPP
