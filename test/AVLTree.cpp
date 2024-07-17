#include "gtest/gtest.h"

#include <random>

#include "AVLTree.hpp"

constexpr uint64_t bigNum = 1000000;

TEST(AVLTree, basicInsertContainSTest){
    AVLTree<int, int> tree;

    tree.insert(0,0);
    tree.insert(5,5);
    tree.insert(8,8);
    tree.insert(3,3);

    EXPECT_TRUE(tree.contains(3));
    EXPECT_TRUE(tree.contains(0));
    EXPECT_FALSE(tree.contains(7));

    EXPECT_EQ(tree.findVal(5), 5);
    EXPECT_EQ(tree.findVal(8), 8);

    EXPECT_ANY_THROW(tree.insert(8,8));
}

TEST(AVLTree, basicDeleteContainsTest){
    AVLTree<int, int> tree;

    tree.insert(0,0);
    tree.insert(5,5);
    tree.insert(8,8);
    tree.insert(3,3);

    EXPECT_TRUE(tree.contains(5));

    tree.remove(5);

    EXPECT_FALSE(tree.contains(5));

    EXPECT_TRUE(tree.contains(0));

    tree.remove(0);

    EXPECT_FALSE(tree.contains(0));
}

TEST(AVLTree, largeInsertTest){
    count n = 500;

    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dis(0, bigNum);

    std::set<int> expected;
    AVLTree<int, int> actual;

    int num = 0;

    for(count i = 0; i < n/2; i++){
        //Add a new number
        do num = dis(rng) % n; while(expected.contains(num));

        expected.insert(num);
        actual.insert(num, num);

        //Now check if the two sets are equal
        for(int j = 0; j < n; j++){
            EXPECT_EQ(expected.contains(j), actual.contains(j));
            if(actual.contains(j)) EXPECT_EQ(j, actual.findVal(j));
        }
    }
}

TEST(AVLTree, largeInsertDeleteTest){
    count n = 50;

    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dis(0, bigNum);

    std::set<int> expected;
    AVLTree<int, int> actual;

    int num = 0;

    for(count i = 0; i < n/2; i++){
        //Add a new number
        do num = dis(rng) % n; while(expected.contains(num));

        expected.insert(num);
        actual.insert(num, num);
    }

    for(count i = 0; i < 5*n; i++){
        //Add a new number
        do num = dis(rng) % n; while(expected.contains(num));

        expected.insert(num);
        actual.insert(num, num);

        //Add a new number
        do num = dis(rng) % n; while(not expected.contains(num));

        expected.erase(num);
        EXPECT_EQ(num, actual.remove(num));

        //Now check if the two sets are equal
        for(int j = 0; j < n; j++){
            EXPECT_EQ(expected.contains(j), actual.contains(j));
            if(actual.contains(j)) EXPECT_EQ(j, actual.findVal(j));
        }
    }
}