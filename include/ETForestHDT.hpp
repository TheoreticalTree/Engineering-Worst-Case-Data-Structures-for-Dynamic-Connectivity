#ifndef GKKT_ETFORESTHDT_HPP
#define GKKT_ETFORESTHDT_HPP

#include <memory>
#include <vector>

#include "base.hpp"

/**
 * This class provides functionality to use EulerTourTrees as described by Henzinger and King 1993
 * It requires that some algorithm further up keeps track of references to the edges to do proper calls
 * The specific tracking is currently set to track edges and tree edges on level as required by Holm, de Lichtenberg and Thorup (1998)
 */
class ETForestHDT {
public:
    class Node {
    public:
        struct TrackingData{
            bool own_isOnLevel = false;
            count own_numNonTreeEdgesOnLevel = 0;
            count accumulated_numTreeEdgesOnLevel = 0;
            count accumulated_numNonTreeEdgesOnLevel = 0;
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
        friend class ETForestHDT;

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

    ETForestHDT() = default;

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
     * @param onLevel is it supposed to be on level
     */
    void setEdgeOnLevel(Node* e, bool onLevel);


    /**
     * Set how many nonTreeEdges are attached to this edge
     * @param e the edge
     * @param nonTreeEdgesOnLevel the number of nonTreeEdges attached to it
     */
    void setEdgeNontreeEdges(Node* e, count nonTreeEdgesOnLevel);

    /**
     * Get an on-level tree edge in the tree of @e
     * @param e reference to any edge in the tree to be searched
     * @return Returns any one tree edge on this level, (node, none) if none exists
     */
    edge getOnLevelTreeEdge(Node* e);

    /**
     * Get a node in the tree of @e that is marked as having nonTreeEdges on level
     * @param e reference to any edge in the tree to be searched
     * @return Returns any one node with nonTreeEdges on level, none if no such node exists
     */
    node getNodeWithOnLevelNontreeEdge(Node* e);

    /**
     * Inserts an edge between node @v and @w
     * @param v the name of node v
     * @param w the name of node w
     * @param vEdge a reference to any edge (v,x)
     * @param wEdge a reference to any edge (w,y)
     * @param vOnLevel is edge (v,w) on this level
     * @param wOnLevel is edge (w,v) on this level
     * @param vNonTreeEdgesOnLevel number of nonTreeEdges of v attached to this edge
     * @param wNonTreeEdgesOnLevel number of nonTreeEdges of w attached to this edge
     * @return references to the two new edges
     */
    std::pair<Node*, Node*> insertETEdge(node v, node w, Node* vEdge, Node* wEdge, bool vOnLevel, bool wOnLevel,
                                         count vNonTreeEdgesOnLevel, count wNonTreeEdgesOnLevel);

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

using addressHDT = ETForestHDT::Node*;
using TrackingDataHDT = ETForestHDT::Node::TrackingData;

#endif //GKKT_ETFORESTHDT_HPP
