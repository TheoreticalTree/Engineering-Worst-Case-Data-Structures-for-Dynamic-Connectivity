#ifndef GKKT_DTREE_HPP
#define GKKT_DTREE_HPP

#include <vector>
#include <deque>

#include "base.hpp"
#include "DynConnectivityAlgorithm.hpp"

class DTree final : public DynConnectivity {
public:
    /**
     * Sets up initial D-Tree data structure for Graph @Gp
     *
     * @param Gp The graph.
     */
    DTree(const Graph &Gp);

    /**
     * Sets up D-Tree data structure with @n nodes
     */
    DTree(count n);

    /**
     * Checks if u and v are connected
     *
     * @param u The node u
     * @param v The node v
     * @return true iff u and v are connected and both are existing nodes
     */
    bool query(node u, node v) override;

    /**
     * Adds the edge {u,v}
     * @param u
     * @param v
     */
    void addEdge(node u, node v) override;

    /**
     * Removes the edge {u,v}
     * @param u
     * @param v
     */
    void deleteEdge(node u, node v) override;

    /**
     * Get the number of components
     * @return The number of connected components
     */
    count numberOfComponents() override;

    /**
     * Get an ID in [0, numberOfComponents -1] for the component of vertex v.
     * @param v the node v
     * @return The ID of the connected component of @v
     */
    count componentOfNode(node v) override;

    std::vector<count> getComponentSizes() override;

    /**
     * get a vector of every node in the connected component of v
     * @param v any representative of the desired connected component
     * @return
     */
    std::vector<node> getComponentOf(node v) override;

    std::vector<std::vector<node>> getComponents() override;

    /**
     * Get a path between two connected vertices
     * @param u node u
     * @param v node v
     * @return a vector of nodes in the order in which they form a u-v path
     *         runtime error if @u and @v are not connected
     */
    std::vector<node> getPath(node u, node v);

protected:
    //! resizes datastructures if a new vertex is added to graph
    void Init(node u, node v);

    //! handles the reinsertion of a node
    void restoreNode(node v);

    //! handles the deletion of a node
    void deleteNode(node v);

    //! check for tree edge. @return true iff there is a edge between v and w
    bool IsTreeEdge(node v, node w);

    //! adds child. @c node to be child @p node to be parent
    void AddChild(node c, node p);

    //! removes child. @c from @children_ of @p and removes backedge and adjusts indexing of backedges
    void CutChild(node c, node p);

    //! removes a non-tree edge from the datastructure
    void CutNTEdge(std::pair<node, size_t> e);

    //! add edge between @u and @v, assumes @u and @v are already connected
    void AddNTEdge(node u, node v);

    //! return root of component containing @u, if allow_reroot is set it compares the size of the roots child with the root and reroots the tree to optimize the depth
    node Find(node u, bool allow_reroot = true);

    //! make @v root of its component. (simply swaps parent/child relationship of the two neighboring nodes and updates sizes)
    //! @trueRoot=false means this doesn't change the true root of any component
    void Reroot(node v, bool trueRoot=true);

    //! connect component of @v and @w, where @rv is the root of the component with @v, performs @reroot if necessary
    void Link(node v, node rv, node w);

    //! removes tree edge between @v and @parent_[u], leaves two D-trees not connected via tree edges
    //! @return root of old D-tree the other root for the new D-trees is v
    node Unlink(node v);

    //! links the smaller of the two D-Trees rooted at @rv and @rw to the other, via the new edge between @v and ł
    void InsertTreeEdge(node v, node rv, node w, node rw);

    //! tries to find replacement edge, leaves two separate d_trees if unsuccessful
    void DeleteTreeEdge(node u, node v);

    //! adds non tree edge, rebalances if height difference between subtrees starting at @u and @v is greater than 2
    void InsertNTEdge(node u, node v);

    //! deletes non tree edge (simply removes nodes from neighboring non_tree_neighbors_ entries)
    void DeleteNTEdge(node u, node v);

    //! returns true if v and w are connected, and restores centroid property
    bool Conn(node v, node w);

    void SanityCheck();

    //! Cleans up a component so all vertices in it have the componentID as their compID
    void cleanComponent(count comp);
    //! Checks if the dirty list is getting too long to avoid using too much space
    void checkDirtyListOvergrow();

    //! Relevant infos for the component queries
    //! Notes the compID of a vertex guaranteed if and only if that vertex is currently the root of its tree or dirtyPartition == false
    std::vector<node> compID;
    //! Counts the number of connected components
    count compNum = 0;
    //! Saves for every existing component the root of said components spanning tree
    std::vector<node> compRep;
    //! Store if the current partition isn't up to date and if yes, which components have changed
    std::vector<bool> dirtyComp;
    std::deque<count> dirtyList;
    bool dirtyPartition = false;
    //! Stores if a too large amount of things are in the dirty list to avoid memory leaks over long usage
    bool dirtyListBlocked = false;

    //! Used to store if vertices have been marked during a path query
    std::vector<bool> marked;

    //! to check if completely new nodes got added to the graph
    count num_nodes_ = 0;
    //! each child node @c saves its parent node @p
    std::vector<node> parent_;
    //! @child_idx_ stores the index, where @c is found in @children_ of @p
    std::vector<std::size_t> child_idx_;
    //! number of vertexes in the subtree rooted at v
    std::vector<std::size_t> size_;
    //! children of a node
    std::vector<std::vector<node>> children_;
    //! all nodes @v is connected to via edges not included in any spanning tree used to find a replacement edge if a tree edge is deleted
    std::vector<std::vector<std::pair<node, std::size_t>>>
    non_tree_neighbors_; // node is neighbour, size_t gives index of backedge
};

#endif //GKKT_DTREE_HPP
