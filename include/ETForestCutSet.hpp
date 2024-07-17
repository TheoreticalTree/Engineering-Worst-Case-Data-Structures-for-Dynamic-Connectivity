#ifndef GKKT_ETFORESTCUTSET_HPP
#define GKKT_ETFORESTCUTSET_HPP

#include <memory>
#include <vector>

#include "base.hpp"

/**
 * This class provides functionality to use EulerTourTrees as described by Henzinger and King 1993
 * It requires that some algorithm further up keeps track of references to the edges to do proper calls
 * The specific tracking is currently set to the edge tracking for the cutset data structure used by Kapron, King and Mountjoy
 */
class ETForestCutSet {
public:
    class Node {
    public:
        struct TrackingData{
            std::vector<std::vector<edge>> accumulatedData;
            std::vector<std::vector<edge>> *ownData = nullptr;
        };

        node getV() {
            return v;
        }

        node getW() {
            return w;
        }

        count getSize() {
            return size;
        }

    protected:
        friend class ETForestCutSet;

        Node() = default;
        Node(node pv, node pw, TrackingData pTrackingData);

        node v = 0, w = 0;

        Node* parent = nullptr;
        Node* leftChild = nullptr;
        Node* rightChild = nullptr;

        count height = 1;
        count size = 1;

        TrackingData trackingData;
    };

    ETForestCutSet(count pBoostLevel, count n, bool pNoTracking=false);

    /**
     * Get the root of the ETTree that @v is part of
     * @param v a node
     * @return the root of the tree
     */
    Node* getRoot(Node* v) const;

    /**
     * Get the tracking data of @edge (use root to get accumulated data of the entire tree)
     * @param edge the edge
     * @return a const reference to the tracking data of the edge
     */
    const Node::TrackingData &getTrackingData(Node* edge);

    /**
     * Set new tracking data for @edge
     * @param e the edge
     * @param newTrackingData the new tracking data
     */
    void setTrackingData(Node* e, std::vector<std::vector<edge>> *trackingDataRef);

    /**
     * Adds @newEdge to the accumulated edgesets of @e and every ancestor in the ETTree
     * @param e reference to the active edge
     * @param newEdge the new edge being inserted
     * @param startingLevels vector of the levels from which upwards the edge is supposed to be inserted
     */
    void addEdgeToData(Node* e, edge newEdge, const std::vector<uint8_t> &startingLevels);

    /**
     * Inserts an edge between node @v and @w
     * @param v the name of node v
     * @param w the name of node w
     * @param vEdge a reference to any edge (v,x)
     * @param wEdge a reference to any edge (w,y)
     * @param vwEdgeTrack tracking data that should be attached to the edge (v,w)
     * @param wvEdgeTrack tracking data that should be attached to the edge (w,v)
     * @return references to the two new edges
     */
    std::pair<Node*, Node*> insertETEdge(node v, node w, Node* vEdge, Node* wEdge);

    /**
     * Deletes an ETEdge and its backedge
     * @param edge a reference to the edge
     * @param backEdge a reference to the backedge
     */
    void deleteETEdge(Node* edge, Node*backEdge);

    /**
     * Get the euler tour of some edge
     * @param v any edge in the euler tour
     * @return the euler tour
     */
    std::vector<std::pair<node, node>> getTour(Node* e);

protected:
    //! Stores if complex tracking is done at all
    bool noTracking = true;
    //! stores the boost level of the cutset data structure
    count boostLevel = 0;
    //! stores the logarithm of n
    count lognsqr = 0;

    /**
     * Rotates the Euler tout of @newFront such that it is now the very first edge within it
     * @param newFront the edge that is supposed to be the new first edge
     * @return the root of the rotated tree
     */
    Node* makeFront(Node* newFront);

    /**
     * Splits the tree that @v is part off into everything before @v and @v with everything after
     * @param v the node v
     * @return a pair of trees, the first one with every node left of @v, the second with all other nodes
     */
    std::pair<Node*, Node*> split(Node* v);

    /**
     * Joins two trees with a join node such that the final in-order is leftTree, joinNode, rightTree
     * @param leftTree the left tree
     * @param rightTree the right tree
     * @param joinTNode the node to join them all
     * @return the root of the final joined tree
     */
    Node* join(Node* leftTree, Node* rightTree, Node* joinNode);

    /**
     * Inserts @v into the tree of @root as the first or last element
     * @param v the new node v
     * @param root the root of the tree to insert into
     * @param first denotes if we insert as the first or last element in-order
     * @return
     */
    Node* trivialInsert(Node* newNode, Node* root, bool first);

    /**
     * Removes the first ot last node from the tree of @root
     * @param root the root of the tree
     * @param first indicates if the first or last element gets removed
     * @return a pair of references (deleted node, root of the remaining tree)
     */
    std::pair<Node*, Node*> trivialDelete(Node* root, bool first);

    //! from start going up checks for unbalanced nodes and rotates to rebalance them, returns the root
    Node* rebalance(Node* start);

    //! Simple single rotation//
    Node* rotate(Node* head, bool left);

    //! refresh tracking data for @v and all its ancestors
    void refreshTrackingDataUpwards(Node* v);

    //! Refreshes height and size of @v, as well as subtree tracking
    void refreshSubtreeInfo(Node* v);

    //! refresh the subtree tracking of @v
    void refreshTracking(Node* v);

    //! Cleanly separetes the connection to a child
    Node* separateDirChild(Node* v, bool left);

    //! Returns the height of a subtree or 0 if handed a nullpointer
    count getHeight(Node* v);
    //! Returns the size of a subtree or 0 if handed a nullpointer
    count getSize(Node* v);
    //! turns this node into a valid 1-node AVL-Tree by removing all references and resetting height, tracking etc.
    void cleanNode(Node* v);

    void checkTreeValidity(Node* v);

    //! Test function that writes the sequence of @root into @path
    void writeTour(Node* root, std::vector<std::pair<node, node>>* path);
};

using address = ETForestCutSet::Node*;
using TrackingData = ETForestCutSet::Node::TrackingData;

#endif //GKKT_ETFORESTCUTSET_HPP
