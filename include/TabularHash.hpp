#ifndef GKKT_TABULARHASH_HPP
#define GKKT_TABULARHASH_HPP

#include <random>
#include <vector>

#include "base.hpp"

/**
 * Tabular hashing from edges into [1:(^n^)²] where ^n^ is the smallest 2^k >= n
 * This provides uniformly random 3-wax-independant hashing
 */
class TabularHash {
public:
    TabularHash(count n, count seed, uint pBlockSize);

    /**
     * Hashes e into [1:(^n^)²] where ^n^ is the smallest 2^k >= n
     * Runs in O(log(n))
     * @param e the edge
     * @return a uniformly random hash value for @e
     */
    node hash(edge e);

protected:
    uint blockSize;
    uint blockNum;
    uint length;
    count nUp2;
    count oneBlock = 0;
    std::vector<std::vector<count>> hashBlocks;
};

#endif //GKKT_TABULARHASH_HPP
