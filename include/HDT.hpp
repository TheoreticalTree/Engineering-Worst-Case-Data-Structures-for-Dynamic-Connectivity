#ifndef GKKT_HDT_HPP
#define GKKT_HDT_HPP

#include <vector>

#include "DynConnectivityAlgorithm.hpp"
#include "Graph.hpp"
#include "base.hpp"
#include "AVLTree.hpp"
#include "ETForestHDT.hpp"

class HDT : public DynConnectivity {
public:
    HDT(const Graph &G);

    HDT(count n);

    /**
     * Queries if node @u and @v are connected in time O(log(n)). Gives false negatives with likelihood at most 1/n^c
     * @param u node u
     * @param v node v
     * @return true if @u and @v are connected with likelihood >= 1 - 1/n^c, false otherwise
     */
    bool query(node u, node v) override;

    /**
     * Add the edge @e to the graph in runtime O(c^2 * log(n)^4)
     * @param e the edge
     */
    void addEdge(node u, node v) override;

    /**
     * Deletes the edge @e from the graph in runtime O(c^2 * log(n)^4)
     * @param e the edge
     */
    void deleteEdge(node u, node v) override;

    /**
     * Get the number of connected components (wrong with chance <= n^{-c})
     * @return
     */
    count numberOfComponents() override;

protected:
    count n = 0;
    count numComponents = 0;

    //! Used to store nonTreeEdges on different levels
    std::vector<std::vector<AVLTree<node, bool>>> nonTreeEdgesOnLevels;
    //! Used to store on which level a nonTreeEdge currently resides
    std::vector<AVLTree<node, uint>> nonTreeEdgesLevelIndex;
    //! Used to store TreeEdges on different levels
    std::vector<std::vector<AVLTree<node, addressHDT>>> treeEdges;
    //! Used to keep track of active edges on all levels
    std::vector<std::vector<addressHDT>> activeEdge;

    ETForestHDT etForest;

    //! Resizes the data structures on insertion of an edge if necessary
    void init(node u, node v);

    //! Adds tree edge to specific level
    void addTreeEdge(node u, node v, count level, bool onLevel);
    //! Deletes tree edge from all levels
    void deleteTreeEdge(node u, node v);

    //! Adds nonTree edge to specific level
    void addNonTreeEdge(node u, node v, count level);
    //! Deletes nonTree edge from specific level
    void deleteNonTreeEdge(node u, node v, count level);

    void refreshActiveEdge(node v, count level);
};

#endif //GKKT_HDT_HPP
