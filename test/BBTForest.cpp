#include "gtest/gtest.h"

#include <random>

#include "BiasedBinaryForest.hpp"

constexpr uint64_t bigNum = 1000000;

TEST(BBTForest, basicGlobalJoinTest){
    BBTNode n0(0, 1), n1(1, 8), n2(2, 64), n3(3, 2);
    BiasedBinaryForest forest;

    BBTNode *path1 = forest.globalJoin(&n0, &n1, 11);
    BBTNode *path2 = forest.globalJoin(&n2, &n3, 22);
    BBTNode *path3 = forest.globalJoin(path1, path2, 33);

    EXPECT_TRUE(n0.isLeaf);

    std::vector<node> path;
    forest.writePath(path3, &path, false);

    for(int i = 0; i < 4; i++){
        EXPECT_EQ(path[i], i);
    }
}

TEST(BBTForest, basicReverseTest){
    BBTNode n0(0, 1), n1(1, 8), n2(2, 64), n3(3, 2);
    BiasedBinaryForest forest;

    BBTNode *path1 = forest.globalJoin(&n0, &n1, 11);
    forest.reverse(path1);
    BBTNode *path2 = forest.globalJoin(&n2, &n3, 22);
    BBTNode *path3 = forest.globalJoin(path1, path2, 33);
    forest.reverse(path3);

    EXPECT_TRUE(n0.isLeaf);

    std::vector<node> expected = {3,2,0,1};
    std::vector<node> path;
    forest.writePath(path3, &path, false);

    for(int i = 0; i < 4; i++){
        EXPECT_EQ(path[i], expected[i]);
    }
}

TEST(BBTForest, largeGlobalJoinTest){
    count n = 10000;

    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dis(0, bigNum);

    std::vector<std::vector<BBTNode*>> pathsSoFar;
    for(count i = 0; i < n; i++){
        BBTNode* node = new BBTNode(i, dis(rng) % (n) + 1);
        pathsSoFar.push_back({node});
    }
    BiasedBinaryForest forest;

    count p1, p2;

    while (pathsSoFar.size() > 1){
        //Get two random paths and join them
        p1 = dis(rng) % pathsSoFar.size();
        do p2 = dis(rng) % pathsSoFar.size(); while (p1 == p2);

        if(p2 < p1) std::swap(p1,p2);

        forest.globalJoin(forest.getBBTRoot(pathsSoFar[p1][0]), forest.getBBTRoot(pathsSoFar[p2][0]), dis(rng) % n);
        //Join their vectors too, so we always know how the paths look
        for(BBTNode* node : pathsSoFar[p2]){
            pathsSoFar[p1].push_back(node);
        }

        //Now move down the path of maximum index and reduce vector size
        pathsSoFar[p2] = pathsSoFar[pathsSoFar.size() - 1];
        pathsSoFar.pop_back();

        //Now check if the new path remains valid
        std::vector<node> path;
        forest.writePath(forest.getBBTRoot(pathsSoFar[p1][0]), &path, false);
        EXPECT_EQ(path.size(), pathsSoFar[p1].size());
        for(count i = 0; i < path.size(); i++){
            EXPECT_EQ(path[i], pathsSoFar[p1][i]->v);
        }
    }
}

TEST(BBTForest, largeGlobalJoinReverseTest){
    count n = 10000;

    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dis(0, bigNum);

    std::vector<std::vector<BBTNode*>> pathsSoFar;
    for(count i = 0; i < n; i++){
        BBTNode* node = new BBTNode(i, dis(rng) % (10*n) + 1);
        pathsSoFar.push_back({node});
    }
    BiasedBinaryForest forest;

    count p1, p2;

    while (pathsSoFar.size() > 1){
        //Get two random paths and join them
        p1 = dis(rng) % pathsSoFar.size();
        do p2 = dis(rng) % pathsSoFar.size(); while (p1 == p2);

        if(p2 < p1) std::swap(p1,p2);

        forest.globalJoin(forest.getBBTRoot(pathsSoFar[p1][0]), forest.getBBTRoot(pathsSoFar[p2][0]), dis(rng) % n);
        //Join their vectors too, so we always know how the paths look
        for(BBTNode* node : pathsSoFar[p2]){
            pathsSoFar[p1].push_back(node);
        }

        //Now move down the path of maximum index and reduce vector size
        pathsSoFar[p2] = pathsSoFar[pathsSoFar.size() - 1];
        pathsSoFar.pop_back();

        //Flip a coin if the resulting path gets reversed
        if(dis(rng) % 2 == 0){
            forest.reverse(forest.getBBTRoot(pathsSoFar[p1][0]));
            for(count i = 0; i < (pathsSoFar[p1].size() / 2); i++){
                std::swap(pathsSoFar[p1][i], pathsSoFar[p1][pathsSoFar[p1].size() - 1 - i]);
            }
        }

        //Now check if the new path remains valid
        std::vector<node> path;
        forest.writePath(forest.getBBTRoot(pathsSoFar[p1][0]), &path, false);
        EXPECT_EQ(path.size(), pathsSoFar[p1].size());
        for(count i = 0; i < path.size(); i++){
            EXPECT_EQ(path[i], pathsSoFar[p1][i]->v);
        }
    }
}

TEST(BBTForest, largeLocalJoinReverseTest){
    count n = 10000;

    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dis(0, bigNum);

    std::vector<std::vector<BBTNode*>> pathsSoFar;
    for(count i = 0; i < n; i++){
        BBTNode* node = new BBTNode(i, dis(rng) % (10*n) + 1);
        pathsSoFar.push_back({node});
    }
    BiasedBinaryForest forest;

    count p1, p2;

    while (pathsSoFar.size() > 1){
        //Get two random paths and join them
        p1 = dis(rng) % pathsSoFar.size();
        do p2 = dis(rng) % pathsSoFar.size(); while (p1 == p2);

        if(p2 < p1) std::swap(p1,p2);

        BBTNode* conn = new BBTNode;

        forest.testLocalJoin(forest.getBBTRoot(pathsSoFar[p1][0]), forest.getBBTRoot(pathsSoFar[p2][0]), conn);
        //Join their vectors too, so we always know how the paths look
        for(BBTNode* node : pathsSoFar[p2]){
            pathsSoFar[p1].push_back(node);
        }

        //Now move down the path of maximum index and reduce vector size
        pathsSoFar[p2] = pathsSoFar[pathsSoFar.size() - 1];
        pathsSoFar.pop_back();

        //Flip a coin if the resulting path gets reversed
        if(dis(rng) % 2 == 0){
            forest.reverse(forest.getBBTRoot(pathsSoFar[p1][0]));
            for(count i = 0; i < (pathsSoFar[p1].size() / 2); i++){
                std::swap(pathsSoFar[p1][i], pathsSoFar[p1][pathsSoFar[p1].size() - 1 - i]);
            }
        }

        //Now check if the new path remains valid
        std::vector<node> path;
        forest.writePath(forest.getBBTRoot(pathsSoFar[p1][0]), &path, false);
        EXPECT_EQ(path.size(), pathsSoFar[p1].size());
        for(count i = 0; i < path.size(); i++){
            EXPECT_EQ(path[i], pathsSoFar[p1][i]->v);
        }
    }
}

TEST(BBTForest, basicGlobalSplitTest) {
    BBTNode n0(0, 1), n1(1, 8), n2(2, 64), n3(3, 2);
    BiasedBinaryForest forest;

    BBTNode *path1 = forest.globalJoin(&n0, &n1, 11);
    BBTNode *path2 = forest.globalJoin(&n2, &n3, 22);
    BBTNode *path3 = forest.globalJoin(path1, path2, 33);

    BiasedBinaryForest::SplitResult splitResult = forest.globalSplit(&n2);

    EXPECT_TRUE(n2.parent == nullptr);

    std::vector<node> pathA, pathB;
    forest.writePath(splitResult.leftTree, &pathA, false);
    forest.writePath(splitResult.rightTree, &pathB, false);

    EXPECT_EQ(splitResult.lCost, 33);
    EXPECT_EQ(splitResult.rCost, 22);

    EXPECT_EQ(pathA.size(), 2);
    EXPECT_EQ(pathA[0], 0);
    EXPECT_EQ(pathA[1], 1);

    EXPECT_EQ(pathB.size(), 1);
    EXPECT_EQ(pathB[0], 3);
}

TEST(BBTForest, largeGlobalSplitTest) {
    count n = 1000;

    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dis(0, bigNum);

    std::vector<std::vector<BBTNode*>> pathsSoFar;
    for(count i = 0; i < n; i++){
        BBTNode* node = new BBTNode(i, dis(rng) % (10*n) + 1);
        pathsSoFar.push_back({node});
    }
    BiasedBinaryForest forest;

    count p1, p2;

    //Step 1: Do a bunch of joins
    for(count i = 0; i < (n / 2); i++){
        //Get two random paths and join them
        p1 = dis(rng) % pathsSoFar.size();
        do p2 = dis(rng) % pathsSoFar.size(); while (p1 == p2);

        if(p2 < p1) std::swap(p1,p2);

        forest.globalJoin(forest.getBBTRoot(pathsSoFar[p1][0]), forest.getBBTRoot(pathsSoFar[p2][0]), dis(rng) % n);
        //Join their vectors too, so we always know how the paths look
        for(BBTNode* node : pathsSoFar[p2]){
            pathsSoFar[p1].push_back(node);
        }

        //Now move down the path of maximum index and reduce vector size
        pathsSoFar[p2] = pathsSoFar[pathsSoFar.size() - 1];
        pathsSoFar.pop_back();
    }

    //Makes sure we always stay around the same number of paths
    int balance = 0;

    for(count i = 0; i < 10*n; i++){
        if(balance >= 0) {
            //Get two random paths and join them
            p1 = dis(rng) % pathsSoFar.size();
            do p2 = dis(rng) % pathsSoFar.size(); while (p1 == p2);

            if (p2 < p1) std::swap(p1, p2);

            forest.globalJoin(forest.getBBTRoot(pathsSoFar[p1][0]), forest.getBBTRoot(pathsSoFar[p2][0]), dis(rng) % n);
            //Join their vectors too, so we always know how the paths look
            for (BBTNode *node: pathsSoFar[p2]) {
                pathsSoFar[p1].push_back(node);
            }

            //Now move down the path of maximum index and reduce vector size
            pathsSoFar[p2] = pathsSoFar[pathsSoFar.size() - 1];
            pathsSoFar.pop_back();

            //Now check if the new path remains valid
            std::vector<node> path;
            forest.writePath(forest.getBBTRoot(pathsSoFar[p1][0]), &path, false);
            EXPECT_EQ(path.size(), pathsSoFar[p1].size());
            for (count i = 0; i < path.size(); i++) {
                EXPECT_EQ(path[i], pathsSoFar[p1][i]->v);
            }

            balance--;
        }
        else {
            //Grab a random path and a random vertex in that path
            p1 = dis(rng) % pathsSoFar.size();
            count splitPos = dis(rng) % pathsSoFar[p1].size();

            BiasedBinaryForest::SplitResult splitResult = forest.globalSplit(pathsSoFar[p1][splitPos]);

            EXPECT_EQ(pathsSoFar[p1][splitPos]->parent, nullptr);

            //Add the two new paths to the paths so far
            if(splitPos != 0){
                pathsSoFar.push_back({pathsSoFar[p1][splitPos]});

                EXPECT_FALSE(splitResult.leftTree == nullptr);

                balance++;
            }
            if(splitPos != pathsSoFar[p1].size() - 1){
                EXPECT_FALSE(splitResult.rightTree == nullptr);

                pathsSoFar.push_back({});
                for(count j = splitPos + 1; j < pathsSoFar[p1].size(); j++){
                    pathsSoFar[pathsSoFar.size() - 1].push_back(pathsSoFar[p1][j]);
                }

                balance++;
            }

            if(splitPos != 0) pathsSoFar[p1].resize(splitPos);
            else pathsSoFar[p1].resize(splitPos + 1);

            if(splitResult.leftTree != nullptr){
                //Now check if the new paths remains valid
                std::vector<node> path;
                forest.writePath(forest.getBBTRoot(pathsSoFar[p1][0]), &path, false);
                EXPECT_EQ(path.size(), pathsSoFar[p1].size());
                for (count i = 0; i < path.size(); i++) {
                    EXPECT_EQ(path[i], pathsSoFar[p1][i]->v);
                }
            }

            if(splitResult.rightTree != nullptr){
                //Now check if the new paths remains valid
                std::vector<node> path;
                forest.writePath(forest.getBBTRoot(pathsSoFar[pathsSoFar.size() - 1][0]), &path, false);
                EXPECT_EQ(path.size(), pathsSoFar[pathsSoFar.size() - 1].size());
                for (count i = 0; i < path.size(); i++) {
                    EXPECT_EQ(path[i], pathsSoFar[pathsSoFar.size() - 1][i]->v);
                }
            }
        }
    }
}

TEST(BBTForest, largeGlobalSplitReverseTest){
    count n = 10000;

    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dis(0, bigNum);

    std::vector<std::vector<BBTNode*>> pathsSoFar;
    for(count i = 0; i < n; i++){
        BBTNode* node = new BBTNode(i, dis(rng) % (10*n) + 1);
        pathsSoFar.push_back({node});
    }
    BiasedBinaryForest forest;

    count p1, p2;

    //Step 1: Do a bunch of joins
    for(count i = 0; i < (n / 2); i++){
        //Get two random paths and join them
        p1 = dis(rng) % pathsSoFar.size();
        do p2 = dis(rng) % pathsSoFar.size(); while (p1 == p2);

        if(p2 < p1) std::swap(p1,p2);

        forest.globalJoin(forest.getBBTRoot(pathsSoFar[p1][0]), forest.getBBTRoot(pathsSoFar[p2][0]), dis(rng) % n);
        //Join their vectors too, so we always know how the paths look
        for(BBTNode* node : pathsSoFar[p2]){
            pathsSoFar[p1].push_back(node);
        }

        //Now move down the path of maximum index and reduce vector size
        pathsSoFar[p2] = pathsSoFar[pathsSoFar.size() - 1];
        pathsSoFar.pop_back();
    }

    //Makes sure we always stay around the same number of paths
    int balance = 0;

    for(count i = 0; i < 10*n; i++){
        if(balance >= 0) {
            //Get two random paths and join them
            p1 = dis(rng) % pathsSoFar.size();
            do p2 = dis(rng) % pathsSoFar.size(); while (p1 == p2);

            if (p2 < p1) std::swap(p1, p2);

            forest.globalJoin(forest.getBBTRoot(pathsSoFar[p1][0]), forest.getBBTRoot(pathsSoFar[p2][0]), dis(rng) % n);
            //Join their vectors too, so we always know how the paths look
            for (BBTNode *node: pathsSoFar[p2]) {
                pathsSoFar[p1].push_back(node);
            }

            //Now move down the path of maximum index and reduce vector size
            pathsSoFar[p2] = pathsSoFar[pathsSoFar.size() - 1];
            pathsSoFar.pop_back();

            //Flip a coin if the resulting path gets reversed
            if (dis(rng) % 2 == 0) {
                forest.reverse(forest.getBBTRoot(pathsSoFar[p1][0]));
                for (count i = 0; i < (pathsSoFar[p1].size() / 2); i++) {
                    std::swap(pathsSoFar[p1][i], pathsSoFar[p1][pathsSoFar[p1].size() - 1 - i]);
                }
            }

            //Now check if the new path remains valid
            std::vector<node> path;
            forest.writePath(forest.getBBTRoot(pathsSoFar[p1][0]), &path, false);
            EXPECT_EQ(path.size(), pathsSoFar[p1].size());
            for (count i = 0; i < path.size(); i++) {
                EXPECT_EQ(path[i], pathsSoFar[p1][i]->v);
            }

            balance--;
        }
        else {
            //Grab a random path and a random vertex in that path
            p1 = dis(rng) % pathsSoFar.size();
            count splitPos = dis(rng) % pathsSoFar[p1].size();

            BiasedBinaryForest::SplitResult splitResult = forest.globalSplit(pathsSoFar[p1][splitPos]);

            EXPECT_EQ(pathsSoFar[p1][splitPos]->parent, nullptr);

            //Add the two new paths to the paths so far
            if(splitPos != 0){
                pathsSoFar.push_back({pathsSoFar[p1][splitPos]});

                EXPECT_FALSE(splitResult.leftTree == nullptr);

                balance++;
            }
            if(splitPos != pathsSoFar[p1].size() - 1){
                EXPECT_FALSE(splitResult.rightTree == nullptr);

                pathsSoFar.push_back({});
                for(count j = splitPos + 1; j < pathsSoFar[p1].size(); j++){
                    pathsSoFar[pathsSoFar.size() - 1].push_back(pathsSoFar[p1][j]);
                }

                balance++;
            }

            if(splitPos != 0) pathsSoFar[p1].resize(splitPos);
            else pathsSoFar[p1].resize(splitPos + 1);

            if(splitResult.leftTree != nullptr){
                //Now check if the new paths remains valid
                std::vector<node> path;
                forest.writePath(forest.getBBTRoot(pathsSoFar[p1][0]), &path, false);
                EXPECT_EQ(path.size(), pathsSoFar[p1].size());
                for (count i = 0; i < path.size(); i++) {
                    EXPECT_EQ(path[i], pathsSoFar[p1][i]->v);
                }
            }

            if(splitResult.rightTree != nullptr){
                //Now check if the new paths remains valid
                std::vector<node> path;
                forest.writePath(forest.getBBTRoot(pathsSoFar[pathsSoFar.size() - 1][0]), &path, false);
                EXPECT_EQ(path.size(), pathsSoFar[pathsSoFar.size() - 1].size());
                for (count i = 0; i < path.size(); i++) {
                    EXPECT_EQ(path[i], pathsSoFar[pathsSoFar.size() - 1][i]->v);
                }
            }
        }
    }
}

//! This test should always be run alone with memcheck to look for memory leaks
TEST(BBTForest, deleteTest){
    count n = 10000;

    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dis(0, bigNum);

    std::vector<std::vector<BBTNode*>> pathsSoFar;
    for(count i = 0; i < n; i++){
        BBTNode* node = new BBTNode(i, dis(rng) % (10*n) + 1);
        pathsSoFar.push_back({node});
    }
    BiasedBinaryForest forest;

    count p1, p2;

    //Step 1: Do a bunch of joins
    for(count i = 0; i < (n / 2); i++){
        //Get two random paths and join them
        p1 = dis(rng) % pathsSoFar.size();
        do p2 = dis(rng) % pathsSoFar.size(); while (p1 == p2);

        if(p2 < p1) std::swap(p1,p2);

        BBTNode* conn = new BBTNode;

        forest.globalJoin(forest.getBBTRoot(pathsSoFar[p1][0]), forest.getBBTRoot(pathsSoFar[p2][0]), dis(rng) % n);
        //Join their vectors too, so we always know how the paths look
        for(BBTNode* node : pathsSoFar[p2]){
            pathsSoFar[p1].push_back(node);
        }

        //Now move down the path of maximum index and reduce vector size
        pathsSoFar[p2] = pathsSoFar[pathsSoFar.size() - 1];
        pathsSoFar.pop_back();
    }

    //Makes sure we always stay around the same number of paths
    int balance = 0;

    for(count i = 0; i < 10*n; i++){
        if(balance >= 0) {
            //Get two random paths and join them
            p1 = dis(rng) % pathsSoFar.size();
            do p2 = dis(rng) % pathsSoFar.size(); while (p1 == p2);

            if (p2 < p1) std::swap(p1, p2);

            forest.globalJoin(forest.getBBTRoot(pathsSoFar[p1][0]), forest.getBBTRoot(pathsSoFar[p2][0]), dis(rng) % n);
            //Join their vectors too, so we always know how the paths look
            for (BBTNode *node: pathsSoFar[p2]) {
                pathsSoFar[p1].push_back(node);
            }

            //Now move down the path of maximum index and reduce vector size
            pathsSoFar[p2] = pathsSoFar[pathsSoFar.size() - 1];
            pathsSoFar.pop_back();

            //Flip a coin if the resulting path gets reversed
            if (dis(rng) % 2 == 0) {
                forest.reverse(forest.getBBTRoot(pathsSoFar[p1][0]));
                for (count i = 0; i < (pathsSoFar[p1].size() / 2); i++) {
                    std::swap(pathsSoFar[p1][i], pathsSoFar[p1][pathsSoFar[p1].size() - 1 - i]);
                }
            }

            //Now check if the new path remains valid
            std::vector<node> path;
            forest.writePath(forest.getBBTRoot(pathsSoFar[p1][0]), &path, false);

            balance--;
        }
        else {
            //Grab a random path and a random vertex in that path
            p1 = dis(rng) % pathsSoFar.size();
            count splitPos = dis(rng) % pathsSoFar[p1].size();

            BiasedBinaryForest::SplitResult splitResult = forest.globalSplit(pathsSoFar[p1][splitPos]);

            //Add the two new paths to the paths so far
            if(splitPos != 0){
                pathsSoFar.push_back({pathsSoFar[p1][splitPos]});

                balance++;
            }
            if(splitPos != pathsSoFar[p1].size() - 1){
                pathsSoFar.emplace_back();
                for(count j = splitPos + 1; j < pathsSoFar[p1].size(); j++){
                    pathsSoFar[pathsSoFar.size() - 1].push_back(pathsSoFar[p1][j]);
                }

                balance++;
            }

            if(splitPos != 0) pathsSoFar[p1].resize(splitPos);
            else pathsSoFar[p1].resize(splitPos + 1);
        }
    }

    for(std::vector<BBTNode*> path : pathsSoFar){
        forest.deleteTree(path[0]);
    }
}