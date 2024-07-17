#include "gtest/gtest.h"

#include <random>

#include "BiasedBinaryForest.hpp"

constexpr uint64_t bigNum = 1000000;

constexpr long seed = 42;

TEST(BBTTracking, basicMinMaxTest){
    BBTNode n0(0, 1), n1(1, 8), n2(2, 64), n3(3, 2);
    BiasedBinaryForest forest;

    BBTNode *path1 = forest.globalJoin(&n0, &n1, 11);
    BBTNode *path2 = forest.globalJoin(&n2, &n3, 22);
    BBTNode *path3 = forest.globalJoin(path1, path2, 33);

    auto minResult = forest.getMinEdgeOnPath(path3);
    auto maxResult = forest.getMaxEdgeOnPath(path3);

    EXPECT_EQ(minResult.second, 11);
    EXPECT_EQ(minResult.first.first, 0);
    EXPECT_EQ(minResult.first.second, 1);

    EXPECT_EQ(maxResult.second, 33);
    EXPECT_EQ(maxResult.first.first, 1);
    EXPECT_EQ(maxResult.first.second, 2);
}

void checkPathMinMax(cost maxEdgeCost, std::vector<std::vector<BBTNode *>> & pathsSoFar,
                     std::vector<std::vector<cost>> &pathCostsSoFar, BiasedBinaryForest &forest, count p1){
    //Now check if the new path has min and max correct
    count minPos = 0, maxPos = 0;
    cost minCost = maxEdgeCost + 1, maxCost = 0;
    for(count j = 0; j < pathCostsSoFar[p1].size(); j++){
        if(pathCostsSoFar[p1][j] <= minCost){
            minPos = j;
            minCost = pathCostsSoFar[p1][j];
        }
        if(pathCostsSoFar[p1][j] >= maxCost){
            maxPos = j;
            maxCost = pathCostsSoFar[p1][j];
        }
    }
    auto minRes = forest.getMinEdgeOnPath(forest.getBBTRoot(pathsSoFar[p1][0]));
    EXPECT_EQ(minRes.second, minCost);
    EXPECT_EQ(minRes.first.first, pathsSoFar[p1][minPos]->v);
    EXPECT_EQ(minRes.first.second, pathsSoFar[p1][minPos + 1]->v);
    auto maxRes = forest.getMaxEdgeOnPath(forest.getBBTRoot(pathsSoFar[p1][0]));
    EXPECT_EQ(maxRes.second, maxCost);
    EXPECT_EQ(maxRes.first.first, pathsSoFar[p1][maxPos]->v);
    EXPECT_EQ(maxRes.first.second, pathsSoFar[p1][maxPos + 1]->v);
}

TEST(BBTTracking, largeMinMaxGlobalJoinTest){
    count n = 10000;
    cost maxEdgeCost = n;

    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> dis(0, bigNum);

    std::vector<std::vector<BBTNode*>> pathsSoFar;
    std::vector<std::vector<cost>> pathCostsSoFar;

    for(count i = 0; i < n; i++){
        BBTNode* node = new BBTNode(i, dis(rng) % (10*n) + 1);
        pathsSoFar.push_back({node});
        pathCostsSoFar.push_back({});
    }
    BiasedBinaryForest forest;

    count p1, p2;

    while (pathsSoFar.size() > 1){
        //Get two random paths and join them
        p1 = dis(rng) % pathsSoFar.size();
        do p2 = dis(rng) % pathsSoFar.size(); while (p1 == p2);

        if(p2 < p1) std::swap(p1,p2);

        cost x = dis(rng) % maxEdgeCost;
        forest.globalJoin(forest.getBBTRoot(pathsSoFar[p1][0]), forest.getBBTRoot(pathsSoFar[p2][0]), x);
        //Join their vectors too, so we always know how the paths look
        for(BBTNode* node : pathsSoFar[p2]){
            pathsSoFar[p1].push_back(node);
        }
        pathCostsSoFar[p1].push_back(x);
        for(cost edgeCost : pathCostsSoFar[p2]){
            pathCostsSoFar[p1].push_back(edgeCost);
        }

        //Now move down the path of maximum index and reduce vector size
        pathsSoFar[p2] = pathsSoFar[pathsSoFar.size() - 1];
        pathsSoFar.pop_back();
        pathCostsSoFar[p2] = pathCostsSoFar[pathCostsSoFar.size() - 1];
        pathCostsSoFar.pop_back();

        checkPathMinMax(maxEdgeCost, pathsSoFar, pathCostsSoFar, forest, p1);
    }
}

TEST(BBTTracking, largeMinMaxGlobalJoinReverseTest){
    count n = 10000;
    cost maxEdgeCost = n;

    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> dis(0, bigNum);

    std::vector<std::vector<BBTNode*>> pathsSoFar;
    std::vector<std::vector<cost>> pathCostsSoFar;

    for(count i = 0; i < n; i++){
        BBTNode* node = new BBTNode(i, dis(rng) % (10*n) + 1);
        pathsSoFar.push_back({node});
        pathCostsSoFar.push_back({});
    }
    BiasedBinaryForest forest;

    count p1, p2;

    while (pathsSoFar.size() > 1){
        //Get two random paths and join them
        p1 = dis(rng) % pathsSoFar.size();
        do p2 = dis(rng) % pathsSoFar.size(); while (p1 == p2);

        if(p2 < p1) std::swap(p1,p2);

        cost x = dis(rng) % maxEdgeCost;
        forest.globalJoin(forest.getBBTRoot(pathsSoFar[p1][0]), forest.getBBTRoot(pathsSoFar[p2][0]), x);
        //Join their vectors too, so we always know how the paths look
        for(BBTNode* node : pathsSoFar[p2]){
            pathsSoFar[p1].push_back(node);
        }
        pathCostsSoFar[p1].push_back(x);
        for(cost edgeCost : pathCostsSoFar[p2]){
            pathCostsSoFar[p1].push_back(edgeCost);
        }

        //Now move down the path of maximum index and reduce vector size
        pathsSoFar[p2] = pathsSoFar[pathsSoFar.size() - 1];
        pathsSoFar.pop_back();
        pathCostsSoFar[p2] = pathCostsSoFar[pathCostsSoFar.size() - 1];
        pathCostsSoFar.pop_back();

        //Flip a coin if the resulting path gets reversed
        if(dis(rng) % 2 == 0){
            forest.reverse(forest.getBBTRoot(pathsSoFar[p1][0]));
            for(count i = 0; i < (pathsSoFar[p1].size() / 2); i++){
                std::swap(pathsSoFar[p1][i], pathsSoFar[p1][pathsSoFar[p1].size() - 1 - i]);
            }
            for(count j = 0; j < (pathCostsSoFar[p1].size() / 2); j++){
                std::swap(pathCostsSoFar[p1][j], pathCostsSoFar[p1][pathCostsSoFar[p1].size() - 1 - j]);
            }
        }

        //Now check if the new path has min and max correct
        checkPathMinMax(maxEdgeCost, pathsSoFar, pathCostsSoFar, forest, p1);
    }
}

TEST(BBTTracking, largeGlobalJoinSplitReverseMinMaxTest){
    count n = 10000;
    cost maxEdgeCost = n;

    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> dis(0, bigNum);

    std::vector<std::vector<BBTNode*>> pathsSoFar;
    std::vector<std::vector<cost>> pathCostsSoFar;

    for(count i = 0; i < n; i++){
        BBTNode* node = new BBTNode(i, dis(rng) % (10*n) + 1);
        pathsSoFar.push_back({node});
        pathCostsSoFar.push_back({});
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

        cost x = dis(rng) % maxEdgeCost;
        forest.globalJoin(forest.getBBTRoot(pathsSoFar[p1][0]), forest.getBBTRoot(pathsSoFar[p2][0]), x);
        //Join their vectors too, so we always know how the paths look
        for(BBTNode* node : pathsSoFar[p2]){
            pathsSoFar[p1].push_back(node);
        }
        pathCostsSoFar[p1].push_back(x);
        for(cost edgeCost : pathCostsSoFar[p2]){
            pathCostsSoFar[p1].push_back(edgeCost);
        }

        //Now move down the path of maximum index and reduce vector size
        pathsSoFar[p2] = pathsSoFar[pathsSoFar.size() - 1];
        pathsSoFar.pop_back();
        pathCostsSoFar[p2] = pathCostsSoFar[pathCostsSoFar.size() - 1];
        pathCostsSoFar.pop_back();
    }

    //Makes sure we always stay around the same number of paths
    int balance = 0;

    for(count i = 0; i < 10*n; i++){
        if(balance >= 0){
            //Get two random paths and join them
            p1 = dis(rng) % pathsSoFar.size();
            do p2 = dis(rng) % pathsSoFar.size(); while (p1 == p2);

            if(p2 < p1) std::swap(p1,p2);

            cost x = dis(rng) % maxEdgeCost;
            forest.globalJoin(forest.getBBTRoot(pathsSoFar[p1][0]), forest.getBBTRoot(pathsSoFar[p2][0]), x);
            //Join their vectors too, so we always know how the paths look
            for(BBTNode* node : pathsSoFar[p2]){
                pathsSoFar[p1].push_back(node);
            }
            pathCostsSoFar[p1].push_back(x);
            for(cost edgeCost : pathCostsSoFar[p2]){
                pathCostsSoFar[p1].push_back(edgeCost);
            }

            //Now move down the path of maximum index and reduce vector size
            pathsSoFar[p2] = pathsSoFar[pathsSoFar.size() - 1];
            pathsSoFar.pop_back();
            pathCostsSoFar[p2] = pathCostsSoFar[pathCostsSoFar.size() - 1];
            pathCostsSoFar.pop_back();

            //Flip a coin if the resulting path gets reversed
            if(dis(rng) % 2 == 0){
                forest.reverse(forest.getBBTRoot(pathsSoFar[p1][0]));
                for(count j = 0; j < (pathsSoFar[p1].size() / 2); j++){
                    std::swap(pathsSoFar[p1][j], pathsSoFar[p1][pathsSoFar[p1].size() - 1 - j]);
                }
                for(count j = 0; j < (pathCostsSoFar[p1].size() / 2); j++){
                    std::swap(pathCostsSoFar[p1][j], pathCostsSoFar[p1][pathCostsSoFar[p1].size() - 1 - j]);
                }
            }

            checkPathMinMax(maxEdgeCost, pathsSoFar, pathCostsSoFar, forest, p1);

            balance --;
        }
        else {
            //Grab a random path and a random vertex in that path
            p1 = dis(rng) % pathsSoFar.size();
            count splitPos = dis(rng) % pathsSoFar[p1].size();

            BiasedBinaryForest::SplitResult splitResult = forest.globalSplit(pathsSoFar[p1][splitPos]);

            if(splitPos > 0){
                EXPECT_EQ(splitResult.lCost, pathCostsSoFar[p1][splitPos - 1]);
            }
            if(splitPos < pathsSoFar[p1].size() - 1){
                EXPECT_EQ(splitResult.rCost, pathCostsSoFar[p1][splitPos]);
            }

            //Add the two new paths to the paths so far
            if(splitPos != 0){
                pathsSoFar.push_back({pathsSoFar[p1][splitPos]});
                pathCostsSoFar.push_back({});

                balance++;
            }
            if(splitPos != pathsSoFar[p1].size() - 1){
                pathsSoFar.push_back({});
                pathCostsSoFar.push_back({});
                for(count j = splitPos + 1; j < pathsSoFar[p1].size(); j++){
                    pathsSoFar[pathsSoFar.size() - 1].push_back(pathsSoFar[p1][j]);
                }
                for(count j = splitPos + 1; j < pathCostsSoFar[p1].size(); j++){
                    pathCostsSoFar[pathCostsSoFar.size() - 1].push_back(pathCostsSoFar[p1][j]);
                }

                balance++;
            }

            if(splitPos != 0){
                pathsSoFar[p1].resize(splitPos);
                pathCostsSoFar[p1].resize(splitPos - 1);
            }
            else{
                pathsSoFar[p1].resize(1);
                pathCostsSoFar[p1].resize(0);
            }

            if(splitResult.leftTree != nullptr){
                if(pathsSoFar[p1].size() > 1) checkPathMinMax(maxEdgeCost, pathsSoFar, pathCostsSoFar, forest, p1);
            }
            if(splitResult.rightTree != nullptr){
                if(pathsSoFar[pathsSoFar.size() - 1].size() > 1) checkPathMinMax(maxEdgeCost, pathsSoFar, pathCostsSoFar, forest, pathsSoFar.size() - 1);
            }
        }
    }
}

TEST(BBTTracking, largeGlobalJoinSplitReverseMinMaxUpdateTest){
    count n = 10000;
    cost maxEdgeCost = n;

    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> dis(0, bigNum);

    std::vector<std::vector<BBTNode*>> pathsSoFar;
    std::vector<std::vector<cost>> pathCostsSoFar;

    for(count i = 0; i < n; i++){
        BBTNode* node = new BBTNode(i, dis(rng) % (10*n) + 1);
        pathsSoFar.push_back({node});
        pathCostsSoFar.push_back({});
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

        cost x = dis(rng) % maxEdgeCost;
        forest.globalJoin(forest.getBBTRoot(pathsSoFar[p1][0]), forest.getBBTRoot(pathsSoFar[p2][0]), x);
        //Join their vectors too, so we always know how the paths look
        for(BBTNode* node : pathsSoFar[p2]){
            pathsSoFar[p1].push_back(node);
        }
        pathCostsSoFar[p1].push_back(x);
        for(cost edgeCost : pathCostsSoFar[p2]){
            pathCostsSoFar[p1].push_back(edgeCost);
        }

        //Now move down the path of maximum index and reduce vector size
        pathsSoFar[p2] = pathsSoFar[pathsSoFar.size() - 1];
        pathsSoFar.pop_back();
        pathCostsSoFar[p2] = pathCostsSoFar[pathCostsSoFar.size() - 1];
        pathCostsSoFar.pop_back();
    }

    //Makes sure we always stay around the same number of paths
    int balance = 0;

    for(count i = 0; i < 10*n; i++){
        if(balance >= 0){
            //Get two random paths and join them
            p1 = dis(rng) % pathsSoFar.size();
            do p2 = dis(rng) % pathsSoFar.size(); while (p1 == p2);

            if(p2 < p1) std::swap(p1,p2);

            cost x = dis(rng) % maxEdgeCost;
            BBTNode* root = forest.globalJoin(forest.getBBTRoot(pathsSoFar[p1][0]), forest.getBBTRoot(pathsSoFar[p2][0]), x);
            //Join their vectors too, so we always know how the paths look
            for(BBTNode* node : pathsSoFar[p2]){
                pathsSoFar[p1].push_back(node);
            }
            pathCostsSoFar[p1].push_back(x);
            for(cost edgeCost : pathCostsSoFar[p2]){
                pathCostsSoFar[p1].push_back(edgeCost);
            }

            //Now move down the path of maximum index and reduce vector size
            pathsSoFar[p2] = pathsSoFar[pathsSoFar.size() - 1];
            pathsSoFar.pop_back();
            pathCostsSoFar[p2] = pathCostsSoFar[pathCostsSoFar.size() - 1];
            pathCostsSoFar.pop_back();

            //Flip a coin if the resulting path gets reversed
            if(dis(rng) % 2 == 0){
                forest.reverse(forest.getBBTRoot(pathsSoFar[p1][0]));
                for(count j = 0; j < (pathsSoFar[p1].size() / 2); j++){
                    std::swap(pathsSoFar[p1][j], pathsSoFar[p1][pathsSoFar[p1].size() - 1 - j]);
                }
                for(count j = 0; j < (pathCostsSoFar[p1].size() / 2); j++){
                    std::swap(pathCostsSoFar[p1][j], pathCostsSoFar[p1][pathCostsSoFar[p1].size() - 1 - j]);
                }
            }

            //Flip a coin if the cost of the path gets changed a bit
            if(dis(rng) % 2 == 0){
                diff val = (dis(rng) % (maxEdgeCost - forest.getMaxEdgeOnPath(root).second)) - forest.getMinEdgeOnPath(root).second;
                forest.update(root, val);
                for(count j = 0; j < pathCostsSoFar[p1].size(); j++){
                    pathCostsSoFar[p1][j] += val;
                }
            }

            checkPathMinMax(maxEdgeCost, pathsSoFar, pathCostsSoFar, forest, p1);

            balance --;
        }
        else {
            //Grab a random path and a random vertex in that path
            p1 = dis(rng) % pathsSoFar.size();
            count splitPos = dis(rng) % pathsSoFar[p1].size();

            BiasedBinaryForest::SplitResult splitResult = forest.globalSplit(pathsSoFar[p1][splitPos]);

            if(splitPos > 0){
                EXPECT_EQ(splitResult.lCost, pathCostsSoFar[p1][splitPos - 1]);
            }
            if(splitPos < pathsSoFar[p1].size() - 1){
                EXPECT_EQ(splitResult.rCost, pathCostsSoFar[p1][splitPos]);
            }

            //Add the two new paths to the paths so far
            if(splitPos != 0){
                pathsSoFar.push_back({pathsSoFar[p1][splitPos]});
                pathCostsSoFar.push_back({});

                balance++;
            }
            if(splitPos != pathsSoFar[p1].size() - 1){
                pathsSoFar.push_back({});
                pathCostsSoFar.push_back({});
                for(count j = splitPos + 1; j < pathsSoFar[p1].size(); j++){
                    pathsSoFar[pathsSoFar.size() - 1].push_back(pathsSoFar[p1][j]);
                }
                for(count j = splitPos + 1; j < pathCostsSoFar[p1].size(); j++){
                    pathCostsSoFar[pathCostsSoFar.size() - 1].push_back(pathCostsSoFar[p1][j]);
                }

                balance++;
            }

            if(splitPos != 0){
                pathsSoFar[p1].resize(splitPos);
                pathCostsSoFar[p1].resize(splitPos - 1);
            }
            else{
                pathsSoFar[p1].resize(1);
                pathCostsSoFar[p1].resize(0);
            }

            if(splitResult.leftTree != nullptr){
                if(pathsSoFar[p1].size() > 1) checkPathMinMax(maxEdgeCost, pathsSoFar, pathCostsSoFar, forest, p1);
            }
            if(splitResult.rightTree != nullptr){
                if(pathsSoFar[pathsSoFar.size() - 1].size() > 1) checkPathMinMax(maxEdgeCost, pathsSoFar, pathCostsSoFar, forest, pathsSoFar.size() - 1);
            }
        }
    }
}

TEST(BBTTracking, basicFindTiltedEdge){
    BBTNode n0(0, 16), n1(1, 8), n2(2, 31), n3(3, 51);
    BiasedBinaryForest forest;

    BBTNode *path1 = forest.globalJoin(&n0, &n1, 11);
    BBTNode *path2 = forest.globalJoin(&n2, &n3, 22);
    BBTNode *path3 = forest.globalJoin(path1, path2, 33);

    auto tiltRes = forest.getTiltedEdgeOnPath(path3);

    EXPECT_EQ(tiltRes.second, -7);
    EXPECT_EQ(tiltRes.first.first, 1);
    EXPECT_EQ(tiltRes.first.second, 2);
}

void checkPathTiltedEdge(std::vector<std::vector<BBTNode *>> & pathsSoFar, BiasedBinaryForest &forest, count p1){
    //Find the rightmost tilted edge by hand
    count leftWeight = pathsSoFar[p1][0]->weight;
    count tiltPos = 0;
    diff topTilt = 0;
    bool hasStrongTilt = false;
    for(count j = 0; j + 1 < pathsSoFar[p1].size(); j++){
        //Check edge (v_j,v_{j+1}) for tilt
        diff tilt = leftWeight - pathsSoFar[p1][j+1]->weight;
        if(tilt <= 0){
            hasStrongTilt = true;
            tiltPos = j;
            topTilt = tilt;
        }
        leftWeight += pathsSoFar[p1][j+1]->weight;
    }

    auto tiltRes = forest.getTiltedEdgeOnPath(forest.getBBTRoot(pathsSoFar[p1][0]));

    if(hasStrongTilt == false){
        EXPECT_EQ(tiltRes.first.first, none);
        EXPECT_EQ(tiltRes.first.second, none);
        EXPECT_EQ(tiltRes.second, 0);
    }
    else {
        EXPECT_EQ(tiltRes.first.first, pathsSoFar[p1][tiltPos]->v);
        EXPECT_EQ(tiltRes.first.second, pathsSoFar[p1][tiltPos + 1]->v);
        EXPECT_EQ(tiltRes.second, topTilt);
    }
}

TEST(BBTTracking, largeFindTiltedEdgeJoin){
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

        checkPathTiltedEdge(pathsSoFar, forest, p1);
    }
}

TEST(BBTTracking, largeFindTiltedEdgeJoinReverse){
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

        BBTNode *t1 = forest.getBBTRoot(pathsSoFar[p1][0]), *t2 = forest.getBBTRoot(pathsSoFar[p2][0]);
        if(p1 == 613 && p2 == 1531){
            std::cout << "sus\n";
        }
        forest.globalJoin(t1, t2, dis(rng) % n);
        if(p1 == 613 && p2 == 1531){
            std::cout << "sus\n";
        }
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
            for(count j = 0; j < (pathsSoFar[p1].size() / 2); j++){
                std::swap(pathsSoFar[p1][j], pathsSoFar[p1][pathsSoFar[p1].size() - 1 - j]);
            }
        }

        checkPathTiltedEdge(pathsSoFar, forest, p1);
    }
}

TEST(BBTTracking, largeFindTiltedEdgeJoinSplitTest){
    count n = 10000;

    std::mt19937 rng(seed);
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

        forest.globalJoin(forest.getBBTRoot(pathsSoFar[p1][0]), forest.getBBTRoot(pathsSoFar[p2][0]), dis(rng) % n + 1);
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
        if(balance >= 0){
            //Get two random paths and join them
            p1 = dis(rng) % pathsSoFar.size();
            do p2 = dis(rng) % pathsSoFar.size(); while (p1 == p2);

            if(p2 < p1) std::swap(p1,p2);

            forest.globalJoin(forest.getBBTRoot(pathsSoFar[p1][0]), forest.getBBTRoot(pathsSoFar[p2][0]), dis(rng) % n + 1);
            //Join their vectors too, so we always know how the paths look
            for(BBTNode* node : pathsSoFar[p2]){
                pathsSoFar[p1].push_back(node);
            }

            //Now move down the path of maximum index and reduce vector size
            pathsSoFar[p2] = pathsSoFar[pathsSoFar.size() - 1];
            pathsSoFar.pop_back();

            /*
            //Flip a coin if the resulting path gets reversed
            if(dis(rng) % 2 == 0){
                forest.reverse(forest.getRoot(pathsSoFar[p1][0]));
                for(count j = 0; j < (pathsSoFar[p1].size() / 2); j++){
                    std::swap(pathsSoFar[p1][j], pathsSoFar[p1][pathsSoFar[p1].size() - 1 - j]);
                }
            }
             */

            checkPathTiltedEdge(pathsSoFar, forest, p1);

            balance --;
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
                pathsSoFar.push_back({});
                for(count j = splitPos + 1; j < pathsSoFar[p1].size(); j++){
                    pathsSoFar[pathsSoFar.size() - 1].push_back(pathsSoFar[p1][j]);
                }

                balance++;
            }

            if(splitPos != 0){
                pathsSoFar[p1].resize(splitPos);
            }
            else{
                pathsSoFar[p1].resize(1);
            }

            if(splitResult.leftTree != nullptr){
                if(pathsSoFar[p1].size() > 1) checkPathTiltedEdge(pathsSoFar, forest, p1);
            }
            if(splitResult.rightTree != nullptr){
                if(pathsSoFar[pathsSoFar.size() - 1].size() > 1) checkPathTiltedEdge(pathsSoFar, forest, pathsSoFar.size() - 1);
            }
        }
    }
}

TEST(BBTTracking, largeFindTiltedEdgeJoinSplitReverseTest){
    count n = 10000;

    std::mt19937 rng(seed);
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

        forest.globalJoin(forest.getBBTRoot(pathsSoFar[p1][0]), forest.getBBTRoot(pathsSoFar[p2][0]), dis(rng) % n + 1);
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
        if(balance >= 0){
            //Get two random paths and join them
            p1 = dis(rng) % pathsSoFar.size();
            do p2 = dis(rng) % pathsSoFar.size(); while (p1 == p2);

            if(p2 < p1) std::swap(p1,p2);

            forest.globalJoin(forest.getBBTRoot(pathsSoFar[p1][0]), forest.getBBTRoot(pathsSoFar[p2][0]), dis(rng) % n + 1);
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
                for(count j = 0; j < (pathsSoFar[p1].size() / 2); j++){
                    std::swap(pathsSoFar[p1][j], pathsSoFar[p1][pathsSoFar[p1].size() - 1 - j]);
                }
            }

            checkPathTiltedEdge(pathsSoFar, forest, p1);

            balance --;
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
                pathsSoFar.push_back({});
                for(count j = splitPos + 1; j < pathsSoFar[p1].size(); j++){
                    pathsSoFar[pathsSoFar.size() - 1].push_back(pathsSoFar[p1][j]);
                }

                balance++;
            }

            if(splitPos != 0){
                pathsSoFar[p1].resize(splitPos);
            }
            else{
                pathsSoFar[p1].resize(1);
            }

            if(splitResult.leftTree != nullptr){
                if(pathsSoFar[p1].size() > 1) checkPathTiltedEdge(pathsSoFar, forest, p1);
            }
            if(splitResult.rightTree != nullptr){
                if(pathsSoFar[pathsSoFar.size() - 1].size() > 1) checkPathTiltedEdge(pathsSoFar, forest, pathsSoFar.size() - 1);
            }
        }
    }
}

void checkStartEndNeighbours(std::vector<std::vector<BBTNode *>> &pathsSoFar, std::vector<std::vector<cost>> &pathCostsSoFar,
                                BiasedBinaryForest &forest, count p1){
    BBTNode * path = forest.getBBTRoot(pathsSoFar[p1][0]);

    EXPECT_EQ(forest.getStart(path), pathsSoFar[p1][0]->v);
    EXPECT_EQ(forest.getEnd(path), pathsSoFar[p1][pathsSoFar[p1].size() - 1]->v);

    for(count i = 0; i < pathsSoFar[p1].size() - 1; i++){
        std::pair<node, cost> resNext = forest.getAfter(pathsSoFar[p1][i]);
        EXPECT_EQ(resNext.first, pathsSoFar[p1][i+1]->v);
        EXPECT_EQ(resNext.second, pathCostsSoFar[p1][i]);
        std::pair<node, cost> resPrev = forest.getBefore(pathsSoFar[p1][i+1]);
        EXPECT_EQ(resPrev.first, pathsSoFar[p1][i]->v);
        EXPECT_EQ(resPrev.second, pathCostsSoFar[p1][i]);
    }

    EXPECT_EQ(forest.getAfter(pathsSoFar[p1][pathsSoFar[p1].size() - 1]).first, none);
    EXPECT_EQ(forest.getBefore(pathsSoFar[p1][0]).first, none);
}

TEST(BBTTracking, largeFurtherTestsJoinSplitReverse){
    count n = 10000;
    cost maxEdgeCost = n;

    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> dis(0, bigNum);

    std::vector<std::vector<BBTNode*>> pathsSoFar;
    std::vector<std::vector<cost>> pathCostsSoFar;

    for(count i = 0; i < n; i++){
        BBTNode* node = new BBTNode(i, dis(rng) % (10*n) + 1);
        pathsSoFar.push_back({node});
        pathCostsSoFar.push_back({});
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

        cost x = dis(rng) % maxEdgeCost;
        forest.globalJoin(forest.getBBTRoot(pathsSoFar[p1][0]), forest.getBBTRoot(pathsSoFar[p2][0]), x);
        //Join their vectors too, so we always know how the paths look
        for(BBTNode* node : pathsSoFar[p2]){
            pathsSoFar[p1].push_back(node);
        }
        pathCostsSoFar[p1].push_back(x);
        for(cost edgeCost : pathCostsSoFar[p2]){
            pathCostsSoFar[p1].push_back(edgeCost);
        }

        //Now move down the path of maximum index and reduce vector size
        pathsSoFar[p2] = pathsSoFar[pathsSoFar.size() - 1];
        pathsSoFar.pop_back();
        pathCostsSoFar[p2] = pathCostsSoFar[pathCostsSoFar.size() - 1];
        pathCostsSoFar.pop_back();
    }

    //Makes sure we always stay around the same number of paths
    int balance = 0;

    for(count i = 0; i < 10*n; i++){
        if(balance >= 0){
            //Get two random paths and join them
            p1 = dis(rng) % pathsSoFar.size();
            do p2 = dis(rng) % pathsSoFar.size(); while (p1 == p2);

            if(p2 < p1) std::swap(p1,p2);

            cost x = dis(rng) % maxEdgeCost;
            forest.globalJoin(forest.getBBTRoot(pathsSoFar[p1][0]), forest.getBBTRoot(pathsSoFar[p2][0]), x);
            //Join their vectors too, so we always know how the paths look
            for(BBTNode* node : pathsSoFar[p2]){
                pathsSoFar[p1].push_back(node);
            }
            pathCostsSoFar[p1].push_back(x);
            for(cost edgeCost : pathCostsSoFar[p2]){
                pathCostsSoFar[p1].push_back(edgeCost);
            }

            //Now move down the path of maximum index and reduce vector size
            pathsSoFar[p2] = pathsSoFar[pathsSoFar.size() - 1];
            pathsSoFar.pop_back();
            pathCostsSoFar[p2] = pathCostsSoFar[pathCostsSoFar.size() - 1];
            pathCostsSoFar.pop_back();

            //Flip a coin if the resulting path gets reversed
            if(dis(rng) % 2 == 0){
                forest.reverse(forest.getBBTRoot(pathsSoFar[p1][0]));
                for(count j = 0; j < (pathsSoFar[p1].size() / 2); j++){
                    std::swap(pathsSoFar[p1][j], pathsSoFar[p1][pathsSoFar[p1].size() - 1 - j]);
                }
                for(count j = 0; j < (pathCostsSoFar[p1].size() / 2); j++){
                    std::swap(pathCostsSoFar[p1][j], pathCostsSoFar[p1][pathCostsSoFar[p1].size() - 1 - j]);
                }
            }

            checkStartEndNeighbours(pathsSoFar, pathCostsSoFar, forest, p1);

            balance --;
        }
        else {
            //Grab a random path and a random vertex in that path
            p1 = dis(rng) % pathsSoFar.size();
            count splitPos = dis(rng) % pathsSoFar[p1].size();

            BiasedBinaryForest::SplitResult splitResult = forest.globalSplit(pathsSoFar[p1][splitPos]);

            if(splitPos > 0){
                EXPECT_EQ(splitResult.lCost, pathCostsSoFar[p1][splitPos - 1]);
            }
            if(splitPos < pathsSoFar[p1].size() - 1){
                EXPECT_EQ(splitResult.rCost, pathCostsSoFar[p1][splitPos]);
            }

            //Add the two new paths to the paths so far
            if(splitPos != 0){
                pathsSoFar.push_back({pathsSoFar[p1][splitPos]});
                pathCostsSoFar.push_back({});

                balance++;
            }
            if(splitPos != pathsSoFar[p1].size() - 1){
                pathsSoFar.push_back({});
                pathCostsSoFar.push_back({});
                for(count j = splitPos + 1; j < pathsSoFar[p1].size(); j++){
                    pathsSoFar[pathsSoFar.size() - 1].push_back(pathsSoFar[p1][j]);
                }
                for(count j = splitPos + 1; j < pathCostsSoFar[p1].size(); j++){
                    pathCostsSoFar[pathCostsSoFar.size() - 1].push_back(pathCostsSoFar[p1][j]);
                }

                balance++;
            }

            if(splitPos != 0){
                pathsSoFar[p1].resize(splitPos);
                pathCostsSoFar[p1].resize(splitPos - 1);
            }
            else{
                pathsSoFar[p1].resize(1);
                pathCostsSoFar[p1].resize(0);
            }

            if(splitResult.leftTree != nullptr){
                if(pathsSoFar[p1].size() > 1) checkStartEndNeighbours(pathsSoFar, pathCostsSoFar, forest, p1);
            }
            if(splitResult.rightTree != nullptr){
                if(pathsSoFar[pathsSoFar.size() - 1].size() > 1) checkStartEndNeighbours(pathsSoFar, pathCostsSoFar, forest, pathsSoFar.size() - 1);
            }
        }
    }
}