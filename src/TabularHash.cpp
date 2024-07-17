#include "TabularHash.hpp"

#include <cmath>

TabularHash::TabularHash(count n, count seed, uint pBlockSize) {
    //First we calculate the smallest potency of 2 >= n
    blockSize = pBlockSize;
    nUp2 = 1;
    length = 0;
    count n2 = n * n;
    while (n2 > nUp2){
        nUp2 *= 2;
        length++;
    }

    blockNum = 2 * std::ceil((1.0 * length)/blockSize);

    std::mt19937 rng(seed);
    std::uniform_int_distribution<count> dis(0, std::numeric_limits<count>::max());

    hashBlocks.resize(blockNum);

    for(count i = 0; i < blockSize; i++){
        oneBlock *= 2;
        oneBlock++;
    }

    uint blockInternalSize = std::pow(2, blockSize);

    for(uint i = 0; i < blockNum; i++){
        hashBlocks[i].resize(blockInternalSize);
        for(uint j = 0; j < blockInternalSize; j++){
            hashBlocks[i][j] = dis(rng) % nUp2;
        }
    }
}

count TabularHash::hash(edge e) {
    node v = e.v;

    count res = 0;
    for(count i = 0; i < blockNum / 2; i++){
        //This bitwise xors res with the value for the ith block
        //This uses the bits blockSize * (i - 1) to blockSize * i - 1 and interprets them as the index in a lookup number
        res ^= hashBlocks[i][(v & (oneBlock<<(blockSize * i)))>>(blockSize*i)];
    }

    v = e.w;

    for(count i = 0; i < blockNum / 2; i++){
        //As seen above
        res ^= hashBlocks[i + blockNum/2][(v & (oneBlock<<(blockSize * i)))>>(blockSize*i)];
    }

    return res;
}