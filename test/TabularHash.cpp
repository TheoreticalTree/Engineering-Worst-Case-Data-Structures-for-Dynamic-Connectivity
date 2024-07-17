#include <gtest/gtest.h>

#include "TabularHash.hpp"

TEST(TabularHash, DebugTest){
    TabularHash hash(2000, 12345, 4);

    std::cout << hash.hash({1500,377}) << "\n";
}

//This test always passes (unless there are crashes) and should be used to check if the number of conflicts is roughly right
TEST(TabularHash, conflictCounter){
    count n = 1<<12;

    std::cout << "n: " << n << "\n";

    TabularHash hash(n, 12345, 4);

    std::vector<int> hits(1<<24, 0);

    for(node v = 0; v < n; v++){
        for(node w = 0; w < n; w++){
            hits[hash.hash({v,w})]++;
        }
    }

    count conflicts = 0;

    for(count i = 0; i < 1<<24; i++){
        if(hits[i] > 1) conflicts += (hits[i] - 1)*hits[i];
    }

    std::cout << "Conflicts: " << conflicts << "\n";
    std::cout << "Expected Number of Conflicts: " << n * n << "\n";
}