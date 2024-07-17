#ifndef GKKT_BIASEDBINARYFOREST_HPP
#define GKKT_BIASEDBINARYFOREST_HPP

#include <utility>
#include <cmath>

#include "base.hpp"

/**
 * This class provides handling for BiasedBinaryTrees used to store paths in Link-Cut-Trees by Sleator and Tarjan
 */
class BiasedBinaryForest {
public:
    /**
     * The BBTNode class represents a singular Node in a biased binary tree
     * All regular node properties are saved as usual, but everything related to left and right operates through it's own function
     */
    class BBTNode {
    public:
        friend class BiasedBinaryForest;

        BBTNode() = default;

        BBTNode(node pNode, count pWeight){
            v = pNode;
            weight = pWeight;
            rank = std::log2(pWeight);
            isLeaf = true;
            trackData.leftMost = this;
            trackData.rightMost = this;
        }

        //! for leaves this stores the node they represent
        node v = none;

        BBTNode* parent = nullptr;
        bool isLeaf = false;
        //! reversed stores if at this node left and right switch for its subtree
        bool reversed = false;
        //! reversalState should be used to tell a node if it's reversed in the grand scheme
        //! It must be calculated by xor-ing every reversed on the path from the root to here
        bool reversalState = false;

        //! Stores the cumulative weight of all leaves in the subtree
        count weight = 1;
        //! Stores the rank
        count rank = 0;

        /**
         * Allows changing the weight of a node ONLY if the node is currently not part of a larger tree
         * @param w a solitary node
         */
        void setWeight(count w);

        /**
         * Returns the child taking reversalState into account, left child iff @left == true
         * @param left true means the left child is returned
         * @return the left child iff left == true, right child otherwise
         */
        BBTNode* getChild(bool left);
        /**
         * Sets one child to a new node
         * @param left true means the left child is set
         * @param newChild the new left child
         */
        void setChild(bool left, BBTNode *newChild);

        /**
         * Checks if the node is a left or right child of its parent
         * @return true iff the node is a left child, whatever otherwise
         */
        bool isLeftChild();

        /**
         * get left/rightMost as appropriate for reversalState
         * @param left true if leftMost is desired, false otherwise
         * @return left/rightMost as appropriate for reversalState
         */
        BBTNode* getDirMost(bool left);

        /**
         * Calculate theleft/righttilt of a root with respect to reversal
         * @param left direction
         * @return
         */
        diff getDirTilt(bool left);

        /**
         * Get the correct minimum tilt with respect to reversalState
         * @param left do you want netLeftMin
         * @return trackerData.netLeftMin if @reversalState == false, trackerData.netRightMin otherwise
         */
        diff getNetDirMin(bool left);

        /**
         * Set the correct minimum tilt with respect to reversalState
         * @param left do you want netLeftMin
         * @param val the new value
         */
        void setNetDirMin(bool left, diff val);

        /**
         * Adjusts all values tracked over the subtree of a node
         */
        void adjustSubtreeTracking();

        /**
         * Adjusts all implicit tracking as the difference to the parent node for children of this node
         */
        void adjustImplicitTracking();

        struct trackingData {
            //! stores the left/rightmost child
            BBTNode *leftMost  = nullptr, *rightMost  = nullptr;
            //! stores cost difference to parent (actual cost if there is no parent)
            costDiff netCost = 0;
            //! stores minCost difference to parent (tracks minimum cost of edge in subtree)
            costDiff netMin = 0;
            //! stores the maxCost difference to parents (tracks maximum cost of edge in subtree)
            costDiff netMax = 0;
            //! store difference in leftMin/rightMin to parent (tracks lowest left-/righttilt in subtree)
            diff netLeftMin = 0, netRightMin = 0;
        };

        trackingData trackData;

    protected:
        BBTNode *leftChild = nullptr, *rightChild = nullptr;

        /**
         * Turns all implicit tracking values in that child from absolute to relative
         * @param child a child node of a root that currently has absolute values for implicit tracking
         */
        void setToDiff(BBTNode* child);
    };

    struct SplitResult {
        BBTNode* leftTree, * rightTree;
        cost lCost, rCost;
    };

    /**
     * Joins two biased binary search trees in such a way that the resulting tree contains all previous leaves
     * and the in-order runthrough of leaves is in-order t1 then in-order t2
     * Maintains all requirements for globally biased binary trees
     * @param t1 the root of the first tree
     * @param t2 the root of the second tree
     * @return the root of the new resulting tree
     */
    BBTNode* globalJoin(BBTNode* t1, BBTNode* t2, cost x);

    /**
     * Splits the tree that @splitNode is part of into one tree of everything before @splitNode and one of everything after
     * @param splitNode the node that will be split at
     * @return references to the two trees that the original has been split into
     */
    SplitResult globalSplit(BBTNode* splitNode);

    /**
     * Returns the root of the BBT that @node is part of
     * @param node the node
     * @return the root of @node's tree
     */
    BBTNode* getBBTRoot(BBTNode* node);

    /**
     * Returns the leaf name of leftmost leaf in the tree of @root
     * @param root the root of the BBT
     * @return the leftmost leaf name, taking reverses into account
     */
    node getStart(BBTNode* root);

    /**
     * Returns the leaf name of rightmost leaf in the tree of @root
     * @param root the root of the BBT
     * @return the leftmost leaf name, taking reverses into account
     */
    node getEnd(BBTNode* root);

    /**
     * Return the name of the leaf left of @leaf and the cost of the edge connecting them
     * @param leaf the leaf
     * @return the name of the leaf right before leaf in-order and the cost of the connecting edge, (none,0) if none exists
     */
    std::pair<node, cost> getBefore(BBTNode* leaf);

    /**
     * Return the name of the leaf right of @leaf and the cost of the edge connecting them
     * @param leaf the leaf
     * @return the name of the leaf right after leaf in-order and the cost of the connecting edge, (none,0) if none exists
     */
    std::pair<node, cost> getAfter(BBTNode* leaf);

    /**
     * Returns the minimum weight edge in the tree with cost. If there are multiple returns the one furthest to the right
     * @param root the root of the tree
     * @return a pair of the edge as two nodes from left to right and the cost, ((none, none),0) if the path has no edges
     */
    std::pair<std::pair<node, node>, cost> getMinEdgeOnPath(BBTNode* root);

    /**
     * Returns the maximum weight edge in the tree with cost. If there are multiple returns the one furthest to the right
     * @param root the root of the tree
     * @return a pair of the edge as two nodes from left to right and the cost, ((none, none),0) if the path has no edges
     */
    std::pair<std::pair<node, node>, cost> getMaxEdgeOnPath(BBTNode* root);

    /**
     * Add @x to the cost of every edge in the entire tree
     * @param root the root of the BBT
     * @param x the cost change
     */
    void update(BBTNode* root, diff x);

    /**
     * Returns the rightmost edge (v,w) such that w->weight >= sum u->weight for all leaves u left of w
     * @param root the root of the tree, which must not be trivial
     * @return a pair of the edge (v,w) and the tilt. ((none, none), 0) if no such edge exists
     */
    std::pair<std::pair<node, node>, diff> getTiltedEdgeOnPath(BBTNode* root);

    /**
     * Reverses the direction of the entire tree, left and right swap meaning
     * @param root the root of the tree to be swapped
     */
    void reverse(BBTNode* root);

    /**
     * Writes down the nodes of this tree in-order
     * @param root the root
     * @param path the vector the final path should be stored in
     * @param reversed should just be false by default
     */
    void writePath(BBTNode* root, std::vector<node>* path, bool reversed=false);

    /**
     * Deletes the entire tree that @node is in
     * @param node any node in the tree to be deleted
     */
    void deleteTree(BBTNode* node);

protected:
    //! parameter to signal universal left
    static constexpr bool LEFT = true;
    //! parameter to signal universal right
    static constexpr bool RIGHT = false;


    /**
     * Performs case 1 of global globalJoin
     * @param t1 the left tree
     * @param t2 the right tree
     * @param x the cost of the joining edge
     * @return the root of the joined trees
     */
    BBTNode* globalJoinCase1(BBTNode* t1, BBTNode* t2, cost x);

    /**
    * Performs case 2/3 of global globalJoin
    * @param t1 the left tree
    * @param t2 the right tree
    * @param x the cost of the joining edge
    * @return the root of the joined trees
    */
    BBTNode* globalJoinCase23(BBTNode* t1, BBTNode* t2, cost x);

    /**
    * Performs case 4 of global globalJoin
    * @param t1 the left tree
    * @param t2 the right tree
    * @param x the cost of the joining edge
    * @return the root of the joined trees
    */
    BBTNode* globalJoinCase4(BBTNode* t1, BBTNode* t2, cost x);

    //! Case 4a of biased binary global globalJoin
    BBTNode* globalJoinCase4a(BBTNode *t1, BBTNode *t2, BBTNode *u, BBTNode *v, std::pair<BBTNode *, BBTNode *> &t1kids,
                              std::pair<BBTNode *, BBTNode *> &t2kids, std::pair<BBTNode *, BBTNode *> &ukids,
                              std::pair<BBTNode *, BBTNode *> &vkids, BBTNode *z);

    //! Case 4b i of biased binary global globalJoin
    BBTNode* globalJoinCase4bi(BBTNode *t1, BBTNode *t2, BBTNode *u, BBTNode *v, std::pair<BBTNode *, BBTNode *> &t1kids,
                               std::pair<BBTNode *, BBTNode *> &t2kids, std::pair<BBTNode *, BBTNode *> &ukids,
                               std::pair<BBTNode *, BBTNode *> &vkids, BBTNode *z);

    //! Case 4b ii of biased binary global globalJoin
    BBTNode *globalJoinCase4bii(BBTNode *t1, BBTNode *t2, BBTNode *u, BBTNode *v, std::pair<BBTNode *, BBTNode *> &t1kids,
                                std::pair<BBTNode *, BBTNode *> &t2kids, std::pair<BBTNode *, BBTNode *> &ukids,
                                std::pair<BBTNode *, BBTNode *> &vkids, BBTNode *z);

    //! Case 4b iii of biased binary global globalJoin
    BBTNode *globalJoinCase4biii(BBTNode *t1, BBTNode *t2, BBTNode *u, BBTNode *v, std::pair<BBTNode *, BBTNode *> &t1kids,
                                 std::pair<BBTNode *, BBTNode *> &t2kids, std::pair<BBTNode *, BBTNode *> &ukids,
                                 std::pair<BBTNode *, BBTNode *> &vkids, BBTNode *z);

    //! Case 4b iv of biased binary global globalJoin
    BBTNode *globalJoinCase4biv(BBTNode *t1, BBTNode *t2, BBTNode *u, BBTNode *v, std::pair<BBTNode *, BBTNode *> &t1kids,
                                std::pair<BBTNode *, BBTNode *> &t2kids, std::pair<BBTNode *, BBTNode *> &ukids,
                                std::pair<BBTNode *, BBTNode *> &vkids, BBTNode *z);

    //! Case 4b v of biased binary global globalJoin
    BBTNode *globalJoinCase4bv(BBTNode *t1, BBTNode *t2, BBTNode *u, BBTNode *v, std::pair<BBTNode *, BBTNode *> &t1kids,
                               std::pair<BBTNode *, BBTNode *> &t2kids, std::pair<BBTNode *, BBTNode *> &ukids,
                               std::pair<BBTNode *, BBTNode *> &vkids, BBTNode *z);

    /**
     * Local join for biased binary search trees
     * Used as a subroutine of globalSplit
     * @param t1 the first tree
     * @param t2 the second tree
     * @param conn the "new" internal node for connection, usually recycled old node from split
     * @return root of the joined tree, globally biased if called from globalSplit
     */
    BBTNode* localJoin(BBTNode* t1, BBTNode* t2, BBTNode* conn);

    /**
     * Calcs the total values for global(local) Split
     * @param splitNode the node we are splitting at
     * @param v
     * @param leftWeight
     * @param rightWeight
     * @param totalValuesHere
     * @param reversalState
     */
    void getTotalValues(BBTNode *splitNode, BBTNode *v, count &leftWeight, count &rightWeight,
                        BBTNode::trackingData &totalValuesHere, bool &reversalState) const;

    /**
     * Tilts @node's subtree in such a way that the root and its left/right child have different ranks
     * The rank of the root may increase by 1
     * @param node the root of the subtree to be tilted
     * @param left true iff the right child of the subtree root is supposed to have lower rank, false for right
     * @return the root of the tilted subtree
     */
    BBTNode* tilt(BBTNode* node, bool left);

    /**
     * Classic binary tree rotation. @left == true means left rotation, otherwise right rotation
     * @param node The topmost node to be rotated
     * @param left controls if left- or right rotation
     * @return the new topmost node
     */
    BBTNode* rotate(BBTNode* node, bool left);

    /**
     * Cuts off the children of root @v and turns them into internally consistent biased binary trees
     * Turns @v into a solitary node and sets all tracking to default except netcost and netmin, which are set to the correct values
     * @param v the root of a biased binary search tree
     * @return a pair of left and right subtree (according to v's reversal) that are internally consistant
     */
    std::pair<BBTNode*, BBTNode*> separateChildren(BBTNode *v);

    /**
     * Attaches the two trees to @v and adjusts all implicit tracking to form a new biased binary tree
     * @param leftChild the root of the left tree
     * @param v the root of the resulting tree
     * @param rightChild the root of the right tree
     */
    void attachChildren(BBTNode* leftChild, BBTNode* v,  BBTNode* rightChild);

    /**
     * Sets the reversal state of root @v and its children to the correct values
     * @param v root of a biased binary tree
     */
    void setReversalStates(BBTNode* v);

    /**
     * Checks if the tree below is a globally biased binary tree
     * Uses asserts and only contains code in debug mode
     * @param root the root of the tree
     */
    void checkTreeValidity(BBTNode* root);

    /**
     * returns the rightmost leaf left of the subtree of @v or symmetric if left == false
     * @param node the node v
     * @param left true iff you want the left neighbouring leaf
     * @return the leaf and the cost of the connecting edge, (nullptr, 0) if no @left neighbouring leaf exists
     */
    std::pair<BBTNode*, cost> getNeighbouringLeaf(BBTNode* node, bool left);

public:
    //! these functions are purely for testing
    void testSetReversalStates(BBTNode* v){
        setReversalStates(v);
    }

    std::pair<BBTNode*, BBTNode*> testSeparateChildren(BBTNode *v){
        return separateChildren(v);
    }

    void testAttachChildren(BBTNode *leftChild, BBTNode *v,  BBTNode *rightChild){
        attachChildren(leftChild, v, rightChild);
    }

    BBTNode* testRotate(BBTNode* root, bool left){
        return rotate(root, left);
    }

    BBTNode* testLocalJoin(BBTNode* t1, BBTNode* t2, BBTNode* conn){
        return localJoin(t1,t2,conn);
    }
};

using BBTNode = BiasedBinaryForest::BBTNode;
using SplitResult = BiasedBinaryForest::SplitResult;

#endif //GKKT_BIASEDBINARYFOREST_HPP