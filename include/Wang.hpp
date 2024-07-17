#ifndef GKKT_Wang_HPP
#define GKKT_Wang_HPP

#include "DynConnectivityAlgorithm.hpp"
#include "Graph.hpp"
#include "base.hpp"
#include "LinkCutTrees.hpp"
#include "CutSet.hpp"
#include "QueryForest.hpp"
#include "QueryForestAVL.hpp"
#include "AVLTree.hpp"

class Wang : public DynConnectivity {
public:
    /**
     * Distinguishes between base mode, heuristic using p=0.5 and heuristic setting levels to straight 4log(n)
     */
    enum Mode { base, pHeu, lvlHeu };

    /**
     * Constructs an instance of the algorithm for dynamic connectivity by Gibb, Kapron, King and Thorn with starting grapg @G
     * @param G the starting graph
     * @param c the precision. Queries are wrong with likelihood <= 1/n^c, the runtime is quadratic in c
     * @param seed the seed for the randomized values
     */
    Wang(Graph &G, float c, uint seed, count boostLevel = 1, Mode mode = base);

    /**
     * Constructs an instance of the algorithm for dynamic connectivity by Gibb, Kapron, King and Thorn on an empty graph with @n vertices
     * @param n the number of vertices in
     * @param c the precision. Queries are wrong with likelihood <= 1/n^c, the runtime is quadratic in c
     * @param seed the seed for the randomized values
     */
    Wang(count n, float c, uint seed, count boostLevel = 1, Mode mode = base);

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
    static constexpr double precision = 1.0/8;

    count n = 0;
    float c = 0;
    count numLevels = 0, boostLevel = 1;
    double p = 0;

    std::vector<AVLTree<node, bool>> adjacencyTrees;
    std::unique_ptr<QueryForestAVL> queryForest;
    std::unique_ptr<CutSet> cutSet;
    std::unique_ptr<LinkCutTrees> linkCutTrees;

    //! Moves up all levels with u and v and ensures that if search in cutSet is successfull the tree grows
    void refreshTrees(node u, node v);
    //! Checks if on @level the tree of @v has an edge leaving it and if so inserts the edge for all levels > @level
    void searchAndInsert(node v, count level);


    //! Sanity check if everything is fine
    void sanityCheck();
};

#endif //GKKT_Wang_HPP
