#ifndef GKKT_AVLTREE_HPP
#define GKKT_AVLTREE_HPP

#include <functional>
#include <utility>

#include "base.hpp"


/**
 * This class provides storage of key-value pairs in an ordered set using AVL-Trees
 * @tparam Key the type used as the key
 * @tparam Val the type used as the stored values
 */
template<class Key, class Val>
class AVLTree {
public:
    AVLTree() = default;

    /**
     * Constructor for custom ordering functions (if the passed function doesn't define a total order you are on your own)
     * @param pLess a function (Key, Key) -> bool that defines a total order on the keys
     */
    explicit AVLTree(std::function<bool(Key, Key)> pLess);

    AVLTree(const AVLTree<Key, Val> &t) noexcept : root(t.root), less(t.less) {}

    AVLTree(AVLTree<Key, Val> &&t) noexcept : root(std::exchange(t.root, nullptr)), less(t.less) {}

    constexpr AVLTree<Key, Val>& operator=(const AVLTree<Key, Val> &other) {
        if (this == &other)
            return *this;

        this->less = other.less;
        this->root = other.root;

        return *this;
    }

    ~AVLTree();

    /**
     * Inserts the key-value-pair (k,v) into the AVL-Tree
     * Runs in O(log(n))
     * @param k the key
     * @param v the val
     */
    void insert(Key k, Val v);

    /**
     * Checks if any key-value pair with key @k is stored
     * Runs in O(log(n))
     * @param k the key
     * @return true iff any key-value-pair with key @k is stored
     */
    bool contains(Key k) const;

    /**
     * Find the value associated with key @k
     * @param k the key
     * @return the value associated with key @k
     */
    Val findVal(Key k) const;

    /**
     * Give a reference to the value associated with key @k
     * @param k the key
     * @return reference to the value associated with key @k
     */
    Val &getValReference(Key k) const;

    /**
     * Change the value of the key-value pair with key @k
     * Runs in O(log(n))
     * @param k the key
     * @param v the new value
     * @return the old value (error if key @k isn't stored)
     */
    Val changeVal(Key k, Val v);

    /**
     * Deletes the key-value-pair with key @k
     * Runs in O(log(n))
     * @param k the key
     * @return the value stored in the pair
     */
    Val remove(Key k);

    /**
     * Check if the AVL-Tree is empty
     * Runs in O(1)
     * @return true iff the AVL-Tree is empty
     */
    [[nodiscard]] bool empty() const;

    /**
     * Returns all key-value-pairs in-order
     * Runs in O(n)
     * @return all key-value-pairs in-order
     */
    std::vector<std::pair<Key, Val>> getSortedSet() const;

    /**
     * Returns a key-value pair stored in the AVL-Tree
     * @return a key-value pair
     */
    std::pair<Key, Val> getAnyEntry() const;

protected:
    static constexpr bool LEFT = true;
    static constexpr bool RIGHT = false;

    class Node {
    public:
        Node(Key pk, Val pv);

        Key k;
        Val v;

        count height = 1;
        count size = 1;

        Node* leftChild = nullptr;
        Node* rightChild = nullptr;
        Node* parent = nullptr;
    };

    //! Function to determine what is less then something else
    std::function<bool(Key, Key)> less = [](Key a, Key b) { return a < b; };
    //! Root of the AVLTree
    Node* root = nullptr;

    //! from start going up checks for unbalanced nodes and rotates to rebalance them, returns the root
    Node* rebalance(Node* start);

    //! Simple single rotation
    Node* rotate(Node* head, bool left);

    //! Refreshes height and size of v
    void refreshSubtreeInfo(Node* v);

    //! Cleanly separetes the connection to a child
    Node* separateDirChild(Node* v, bool left);

    //! Returns the height of a subtree or 0 if handed a nullpointer
    count getHeight(Node* v);
    //! Returns the size of a subtree or 0 if handed a nullpointer
    count getSize(Node* v);

    void checkTreeValidity(Node* v);
};

#include "../src/AVLTree.ipp"

#endif //GKKT_AVLTREE_HPP
