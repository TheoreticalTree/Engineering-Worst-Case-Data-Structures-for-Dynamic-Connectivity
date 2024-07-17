#ifndef BIASEDBINARYTREES_BIASEDBINARYHEAP_HPP
#define BIASEDBINARYTREES_BIASEDBINARYHEAP_HPP

#include <vector>
#include <functional>

#include "base.hpp"

//TODO put in a proper biased binary tree implementation here

template<class Key, class Val>
class BiasedBinaryHeap{
public:
    /**
     * Creates a biased binary heap using @pCompare to define the total order
     * @param pCompare a funktion Key, Key -> bool that returns true iff the first Key is less then the second
     *                 Must define a total order of undefined behaviour may occur
     */
    explicit BiasedBinaryHeap(std::function<bool(Key, Key)> pLess);

    ~BiasedBinaryHeap() = default;

    /**
     * Insert the Key-value-pair (k,v) with weight w
     * @param k Key
     * @param v value
     * @param w weight
     */
    void insert(Key k, Val v, count w);

    /**
     * Delete the entry with Key k
     * @param k Key
     * @return the value that used to be stored with the Key
     */
    Val remove(Key k);

    /**
     * Changes the value of the entry with Key @k to @v
     * @param k the Key
     * @param v the new value
     * @return the old value
     */
    Val changeVal(Key k, Val v);

    /**
     * Get the value with maximum Key
     * @return (k,v) for the Key-value-pair with maximum Key
     */
    std::pair<Key, Val> getMax();

    /**
     * Check if the structure is empty
     * @return true iff the BBH currently contains no elements
     */
    bool isEmpty();

    /**
     * Checks if an element with Key k is currently stored
     * @param k the Key to search for
     * @return true iff there exists an element with Key k
     */
    bool contains(Key k);

    /**
     * Get the sum of all weights in the Heap
     * @return the sum of all weights
     */
    count getWeightSum();

protected:
    class BBHNode{
        friend class BiasedBinaryHeap;
    protected:

    };

    count weightSum = 0;

    std::function<bool(Key, Key)> less = [](Key a, Key b) { return a < b; };

    std::vector<std::pair<Key, Val>> entries;
};

template<class Key, class Val> using BBHNode = BiasedBinaryHeap<Key, Val>::BBHNode;

#include "../src/BiasedBinaryHeap.ipp"

#endif //BIASEDBINARYTREES_BIASEDBINARYHEAP_HPP
