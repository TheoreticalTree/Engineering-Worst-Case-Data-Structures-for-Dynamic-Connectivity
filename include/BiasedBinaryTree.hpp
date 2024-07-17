#ifndef GKKT_BIASEDBINARYTREE_HPP
#define GKKT_BIASEDBINARYTREE_HPP

#include <utility>
#include <functional>

#include "base.hpp"

/**
 * Stores an ordered set of keys and values as a globally biased binary search tree
 * key-value-pairs are associated with weight. The weight should be proportional to how often this value will be queried
 * @tparam Key the type used as keys
 * @tparam Val the type of stored values
 */
template<class Key, class Val>
class BiasedBinaryTree {
public:
    BiasedBinaryTree() = default;

    /**
     * Constructor for custom ordering functions (if the passed function doesn't define a total order you are on your own)
     * @param pLess a function (Key, Key) -> bool that defines a total order on the keys
     */
    explicit BiasedBinaryTree(std::function<bool(Key, Key)> pLess);

    BiasedBinaryTree(const BiasedBinaryTree<Key, Val> &t) noexcept : less(t.less), root(t.root) {}

    BiasedBinaryTree(BiasedBinaryTree<Key, Val> &&t) noexcept : root(std::exchange(t.root, nullptr)), less(t.less) {}

    constexpr BiasedBinaryTree<Key, Val>& operator=(const BiasedBinaryTree<Key, Val> &other) {
        if (this == &other)
            return *this;

        this->less = other.less;
        this->root = other.root;

        return *this;
    }

    ~BiasedBinaryTree();

    /**
     * Insert a new key-value-pair into the biased binary tree
     * Runs in time O(log(W) - (log(w_k-) + log(w_k+)) + log(W) - log(w_k)) where w_k- and w_k+ are the keys right before/after k in-order
     * @param k the key
     * @param v the value
     * @param weight the weight, of the node vs the total weight should be proportional to the frequency of queries for this key-value-pair out of all pairs
     */
    void insert(Key k, Val v, count weight);

    /**
     * Deletes a the key-value-pair with key @k, returns the related value
     * Runs in time O(log(W) - (log(w_k-) + log(w_k+)) + log(W) - log(w_k)) where w_k- and w_k+ are the keys right before/after k in-order
     * @param k the key k
     * @return the value stored at key @k
     */
    Val remove(Key k);

    /**
     * Checks if any pair with key @k is stored
     * Runs in O(log(W) - log(w_k)) if key @k is stored, O(log(W)) otherwise
     * @param k the key
     * @return true iff a key-value pair with that key exists
     */
    bool contains(Key k) const;

    /**
     * Finds the value stored with key @k, throws an error if k isn't stored
     * Runs in O(log(W) - log(w_k)) if key @k is stored, O(log(W)) otherwise
     * @param k the key
     * @return the value stored with key @k
     */
    Val findVal(Key k) const;

    /**
     * Changes the value associated with key @k
     * Runs in O(log(W) - log(w_k)) if key @k is stored, O(log(W)) otherwise
     * @param k the key
     * @param v the new value
     * @return the old value
     */
    Val changeVal(Key k, Val v);

    /**
     * Checks if the tree is empty
     * Runs in O(1)
     * @return true iff no keys are currently stored
     */
    bool empty() const;

    /**
     * Returns the key-value-pair with minimum key
     * Runs in O(1)
     * @return the key-value-pair with minimum key
     */
    std::pair<Key, Val> getMin() const;

    /**
     * Returns the key-value-pair with maximum key
     * Runs in O(1)
     * @return the key-value-pair with maximum key
     */
    std::pair<Key, Val> getMax() const;

    /**
     * Returns all key-value-pairs with weight in-order
     * Runs in O(n)
     * @return all key-value-pairs with weight in-order
     */
    std::vector<std::pair<Key, Val>> getSortedSet() const;

    /**
     * Returns the sum of all weights
     * Runs in O(1)
     * @return the sum of all weights
     */
    count getWeightSum() const;

protected:
    class Node{
    public:
        Node() = default;
        Node(Key pk, Val pv, count pWeight);

        Key k;
        Val v;

        bool isLeaf = false;

        count weight = 1;
        count rank = 0;

        Node* parent = nullptr;
        Node* leftChild = nullptr;
        Node* rightChild = nullptr;

        Node* leftMost = nullptr;
        Node* rightMost = nullptr;

        Node* getDirChild(bool left);
    };

    //! Stores the function defining how the total order works
    std::function<bool(Key, Key)> less = [](Key a, Key b) { return a < b; };

    //! root of the tree stored here
    Node* root = nullptr;

    //! local join for biased binary trees
    Node* localJoin(Node* leftTree, Node* rightTree, Node* joinNode);

    //! global join for biased binary trees
    Node* globalJoin(Node* leftTree, Node* rightTree, Node* joinNode);
    //! case 1 of global join
    Node* globalJoinCase1(Node* leftTree, Node* rightTree, Node* joinNode);
    //! case 2/3 of global join
    Node* globalJoinCase23(Node* leftTree, Node* rightTree, Node* joinNode);
    //! case 4 of global join
    Node* globalJoinCase4(Node* leftTree, Node* rightTree, Node* joinNode);
    //! case 4a of global join
    Node* globalJoinCase4a(Node* leftTree, Node* rightTree, Node* u, Node* v, Node* z, Node* joinNode);
    //! case 4bi of global join
    Node* globalJoinCase4bi(Node* leftTree, Node* rightTree, Node* u, Node* v, Node* z, Node* joinNode);
    //! case 4bii of global join
    Node* globalJoinCase4bii(Node* leftTree, Node* rightTree, Node* u, Node* v, Node* z, Node* joinNode);
    //! case 4biii of global join
    Node* globalJoinCase4biii(Node* leftTree, Node* rightTree, Node* u, Node* v, Node* z, Node* joinNode);
    //! case 4biv of global join
    Node* globalJoinCase4biv(Node* leftTree, Node* rightTree, Node* u, Node* v, Node* z, Node* joinNode);
    //! case 4bv of global join
    Node* globalJoinCase4bv(Node* leftTree, Node* rightTree, Node* u, Node* v, Node* z, Node* joinNode);

    struct SplitResult {
        Node *leftTree, *splitNode, *rightTree;
        //Returns the places of storage of the nodes that were split at for storage handling
        Node *leftSplitNode, *rightSplitNode;
    };

    //! split for biased binary trees (global if the tree was global before)
    SplitResult split(Node* pRoot, Key k);

    //! ensures that the right(left) child of the root has lower rank then the root
    Node* tilt(Node* pRoot, bool left);

    //! standard single rotate, but only for tree roots
    Node* rotate(Node* pRoot, bool left);

    //! Cuts off the left child clean
    Node* separateLeft(Node* v);
    //! Cuts off the right child clean
    Node* separateRight(Node* v);

    struct CutOffKids {
        Node* first;
        Node* second;
    };
    //! Cuts off both kids clean
    CutOffKids separateKids(Node* v);

    //! Attaches a child left and adjusts tracking
    void attachChildLeft(Node* v, Node* child);
    //! Attaches a child right and adjusts tracking
    void attachChildRight(Node* v, Node* child);
    //! Attaches both children and adjusts tracking
    void attachChildren(Node* v, Node* leftChild, Node* rightChild);

    Node* getRoot(Node* v);

    Node* getDirNeighbouringLeaf(Node* v, bool left);

    void checkTreeValidity(Node* pRoot);
};

#include "../src/BiasedBinaryTree.ipp"

#endif //GKKT_BIASEDBINARYTREE_HPP
