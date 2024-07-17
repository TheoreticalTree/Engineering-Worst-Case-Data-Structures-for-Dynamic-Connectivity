#include "gtest/gtest.h"

#include "base.hpp"
#include "DTree.hpp"
#include "CutSet.hpp"

constexpr uint64_t bigNum = 1000000;

TEST(CutSet, basicTest){
    std::vector<AVLTree<node, bool>> adjacencyTrees(6);

    CutSet cutset(6, 1, 4, 42, adjacencyTrees);

    adjacencyTrees[0].insert(1, false);
    adjacencyTrees[1].insert(0, false);
    cutset.addEdgeToSet({0, 1});
    adjacencyTrees[0].insert(2, false);
    adjacencyTrees[2].insert(0, false);
    cutset.addEdgeToSet({0, 2});
    adjacencyTrees[1].insert(3, false);
    adjacencyTrees[3].insert(1, false);
    cutset.addEdgeToSet({1, 3});
    adjacencyTrees[2].insert(3, false);
    adjacencyTrees[3].insert(2, false);
    cutset.addEdgeToSet({2, 3});
    adjacencyTrees[2].insert(4, false);
    adjacencyTrees[4].insert(2, false);
    cutset.addEdgeToSet({2, 4});
    adjacencyTrees[2].insert(5, false);
    adjacencyTrees[5].insert(2, false);
    cutset.addEdgeToSet({2, 5});
    adjacencyTrees[4].insert(5, false);
    adjacencyTrees[5].insert(4, false);
    cutset.addEdgeToSet({4, 5});

    cutset.makeTreeEdge({0, 1}, 1);
    cutset.makeTreeEdge({2, 3}, 1);
    cutset.makeTreeEdge({4, 5}, 1);

    cutset.makeTreeEdge({1, 3}, 2);

    cutset.makeTreeEdge({2, 5}, 3);

    //Check if connectivity is correct on various levels
    EXPECT_NE(cutset.compRepresentative(0, 0), cutset.compRepresentative(1, 0));
    EXPECT_NE(cutset.compRepresentative(0, 0), cutset.compRepresentative(2, 0));
    EXPECT_NE(cutset.compRepresentative(2, 0), cutset.compRepresentative(3, 0));
    EXPECT_NE(cutset.compRepresentative(3, 0), cutset.compRepresentative(4, 0));
    EXPECT_NE(cutset.compRepresentative(4, 0), cutset.compRepresentative(5, 0));

    EXPECT_EQ(cutset.compRepresentative(0, 1), cutset.compRepresentative(1, 1));
    EXPECT_NE(cutset.compRepresentative(0, 1), cutset.compRepresentative(2, 1));
    EXPECT_EQ(cutset.compRepresentative(2, 1), cutset.compRepresentative(3, 1));
    EXPECT_NE(cutset.compRepresentative(2, 1), cutset.compRepresentative(4, 1));
    EXPECT_EQ(cutset.compRepresentative(4, 1), cutset.compRepresentative(5, 1));

    EXPECT_EQ(cutset.compRepresentative(0, 2), cutset.compRepresentative(1, 2));
    EXPECT_EQ(cutset.compRepresentative(0, 2), cutset.compRepresentative(2, 2));
    EXPECT_EQ(cutset.compRepresentative(2, 2), cutset.compRepresentative(3, 2));
    EXPECT_NE(cutset.compRepresentative(2, 2), cutset.compRepresentative(4, 2));
    EXPECT_EQ(cutset.compRepresentative(4, 2), cutset.compRepresentative(5, 2));

    EXPECT_EQ(cutset.compRepresentative(0, 3), cutset.compRepresentative(1, 3));
    EXPECT_EQ(cutset.compRepresentative(0, 3), cutset.compRepresentative(2, 3));
    EXPECT_EQ(cutset.compRepresentative(2, 3), cutset.compRepresentative(3, 3));
    EXPECT_EQ(cutset.compRepresentative(2, 3), cutset.compRepresentative(4, 3));
    EXPECT_EQ(cutset.compRepresentative(4, 3), cutset.compRepresentative(5, 3));

    for(count i = 0; i < 4; i++){
        for(node v = 0; v < 6; v++){
            std::cout << "Edge searched by " << v << " is (" << cutset.search(v, i).v << "," << cutset.search(v, i).w << ") on level " << i << "\n";
        }
    }
}

void checkConnectivityEquality(uint n, uint numLevels, std::vector<DTree> &dTrees, CutSet &cutSet){
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dis(0, bigNum);

    for(node i = 0; i < 3*n; i++){
        node u = dis(rng) % n;
        node v = dis(rng) % n;
        for(uint level = 0; level < numLevels; level++){
            EXPECT_EQ(dTrees[level].query(u, v), cutSet.compRepresentative(u, level) == cutSet.compRepresentative(v, level));
        }
    }
}

TEST(CutSet, testConnectivityCorrectness){
    uint n = 100;
    uint numLevels = 10;
    uint boostLevel = 0;

    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dis(0, bigNum);

    std::vector<AVLTree<node, bool>> adjacencyTrees(n);

    CutSet cutSet(n, boostLevel, numLevels, 42, adjacencyTrees);
    std::vector<DTree> dTrees(numLevels, n);

    node u, v;

    std::vector<std::pair<edge, int>> edgeList;

    //Insert a bunch of edges to form a forest
    for(count i = 0; i < n/2; i++){
        //Find an edge that is not yet connected
        u = dis(rng) % n;
        do v = dis(rng) % n; while(dTrees.back().query(u,v));

        int insertionLevel = dis(rng) % numLevels;
        edgeList.push_back({{u, v}, insertionLevel});

        for(int j = insertionLevel; j < numLevels; j++) dTrees[j].addEdge(u, v);
        cutSet.makeTreeEdge({u, v}, insertionLevel);

        checkConnectivityEquality(n, numLevels, dTrees, cutSet);
    }

    for(count i = 0; i < 5*n; i++){
        //Find an edge that is not yet connected
        u = dis(rng) % n;
        do v = dis(rng) % n; while(dTrees.back().query(u,v));

        int insertionLevel = dis(rng) % numLevels;
        edgeList.push_back({{u, v}, insertionLevel});

        for(int j = insertionLevel; j < numLevels; j++) dTrees[j].addEdge(u, v);
        cutSet.makeTreeEdge({u, v}, insertionLevel);

        checkConnectivityEquality(n, numLevels, dTrees, cutSet);

        //Delete one edge
        count killEdge = dis(rng) % edgeList.size();
        std::swap(edgeList[killEdge], edgeList.back());
        std::pair<edge, int> e = edgeList.back();
        edgeList.pop_back();

        for(int j = e.second; j < numLevels; j++){
            dTrees[j].deleteEdge(e.first.v, e.first.w);
        }
        cutSet.deleteEdge(e.first);

        checkConnectivityEquality(n, numLevels, dTrees, cutSet);
    }
}

//Returns number of searches with chance to succeed and number of successes
std::pair<count, count> getNumSuccessfulSearches(count n, count numLevels, std::vector<AVLTree<node, bool>> &adjacencyTrees, CutSet &cutSet, DTree &dTree){
    count searches = 0, successes = 0;

    for(count i = 0; i < numLevels; i++){
        for(node v = 0; v < n; v++){
            if(cutSet.compSize(v, i) < dTree.getComponentSizes()[dTree.componentOfNode(v)]){
                edge res = cutSet.search(v, i);
                if(res != noEdge){
                    EXPECT_TRUE(adjacencyTrees[res.v].contains(res.w));
                    EXPECT_TRUE(adjacencyTrees[res.w].contains(res.v));
                    EXPECT_TRUE((cutSet.compRepresentative(v, i) == cutSet.compRepresentative(res.v, i)) != (cutSet.compRepresentative(v,i) == cutSet.compRepresentative(res.w, i)));
                    successes++;
                }

                searches++;
            }
        }
    }

    return {searches, successes};
}

TEST(CutSet, countSuccessfulSearchesStatic){
    uint n = 1000;
    uint numLevels = 20;
    uint boostLevel = 1;

    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dis(0, bigNum);

    std::vector<AVLTree<node, bool>> adjacencyTrees(n);

    CutSet cutSet(n, boostLevel, numLevels, 42, adjacencyTrees);
    DTree dTree(n);

    //Step 1: Build up a graph in an incremental fashion
    for(count i = 0; i < n; i++){
        for(count j = i+1; j < n; j++){
            if(dis(rng) % n < 5){
                adjacencyTrees[i].insert(j, false);
                adjacencyTrees[j].insert(i, false);
                dTree.addEdge(i, j);
                cutSet.addEdgeToSet({i, j});
                //Add the edge as a tree edge on a random level if it connects previously unconnected components
                if(cutSet.compRepresentative(i, numLevels - 1) != cutSet.compRepresentative(j, numLevels - 1)){
                    count level = dis(rng) % numLevels;
                    cutSet.makeTreeEdge({i, j}, level);
                }
            }
        }
    }

    std::pair<count, count> succRate = getNumSuccessfulSearches(n, numLevels, adjacencyTrees, cutSet, dTree);

    std::cout << "Number of searches:  " << succRate.first << "\n";
    std::cout << "Number of successes: " << succRate.second << "\n";
}

TEST(CutSet, countSuccessfulSearchesIncremental){
    uint n = 1000;
    uint numLevels = 2;
    uint boostLevel = 1;

    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dis(0, bigNum);

    std::vector<AVLTree<node, bool>> adjacencyTrees(n);

    CutSet cutSet(n, boostLevel, numLevels, 42, adjacencyTrees);
    DTree dTree(n);

    count searches = 0, successes = 0;

    std::vector<edge> edgeList;

    //Step 1: generate random edges
    for(count i = 0; i < n; i++){
        for(count j = i+1; j < n; j++){
            if(dis(rng) % n < 5){
                edgeList.emplace_back(i, j);
            }
        }
    }

    //Randomize the edgelist
    for(count i = 0; i < edgeList.size(); i++){
        count swapPos = dis(rng) % (edgeList.size() - i) + i;
        if(swapPos != i) std::swap(edgeList[i], edgeList[swapPos]);
    }

    //Insert all the edges
    count i = 0;
    for(edge e : edgeList){
        adjacencyTrees[e.v].insert(e.w, false);
        adjacencyTrees[e.w].insert(e.v, false);
        dTree.addEdge(e.v, e.w);
        cutSet.addEdgeToSet(e);
        if(cutSet.compRepresentative(e.v, numLevels - 1) != cutSet.compRepresentative(e.w, numLevels - 1)){
            count level = dis(rng) % numLevels;
            cutSet.makeTreeEdge({e.v, e.w}, level);
        }

        if(i % (n/10) == 0){
            std::pair<count, count> searchResults = getNumSuccessfulSearches(n, numLevels, adjacencyTrees, cutSet, dTree);
            searches += searchResults.first;
            successes += searchResults.second;
        }
        i++;
    }

    std::cout << "Number of searches:  " << searches << "\n";
    std::cout << "Number of successes: " << successes << "\n";
}

TEST(CutSet, countSuccessfulSearchesFullyDynamic){
    uint n = 1000;
    uint numLevels = 10;
    uint boostLevel = 1;

    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dis(0, bigNum);

    std::vector<AVLTree<node, bool>> adjacencyTrees(n);

    CutSet cutSet(n, boostLevel, numLevels, 42, adjacencyTrees);
    DTree dTree(n);

    count searches = 0, successes = 0;

    std::vector<edge> edgeList;

    //Step 1: generate random edges
    for(count i = 0; i < 3*n; i++){
        for(count j = i+1; j < n; j++){
            if(dis(rng) % n < 5){
                edgeList.emplace_back(i, j);
            }
        }
    }

    //Randomize the edgelist
    for(count i = 0; i < edgeList.size(); i++){
        count swapPos = dis(rng) % (edgeList.size() - i) + i;
        if(swapPos != i) std::swap(edgeList[i], edgeList[swapPos]);
    }

    //Insert all the edges
    count i = 0;
    for(edge e : edgeList){
        adjacencyTrees[e.v].insert(e.w, false);
        adjacencyTrees[e.w].insert(e.v, false);
        dTree.addEdge(e.v, e.w);
        cutSet.addEdgeToSet(e);
        if(cutSet.compRepresentative(e.v, numLevels - 1) != cutSet.compRepresentative(e.w, numLevels - 1)){
            count level = dis(rng) % numLevels;
            cutSet.makeTreeEdge({e.v, e.w}, level);
        }

        if(i % (n/10) == 0){
            std::pair<count, count> searchResults = getNumSuccessfulSearches(n, numLevels, adjacencyTrees, cutSet, dTree);
            searches += searchResults.first;
            successes += searchResults.second;
        }
        i++;
    }

    //Randomize the edgelist again
    for(count i = 0; i < edgeList.size(); i++){
        count swapPos = dis(rng) % (edgeList.size() - i) + i;
        if(swapPos != i) std::swap(edgeList[i], edgeList[swapPos]);
    }

    //Delete all the edges
    i = 0;
    for(edge e : edgeList){
        adjacencyTrees[e.v].remove(e.w);
        adjacencyTrees[e.w].remove(e.v);
        dTree.deleteEdge(e.v, e.w);
        cutSet.deleteEdge(e);

        if(i % (n/10) == 0){
            std::pair<count, count> searchResults = getNumSuccessfulSearches(n, numLevels, adjacencyTrees, cutSet, dTree);
            searches += searchResults.first;
            successes += searchResults.second;
        }
        i++;
    }

    std::cout << "Number of searches:  " << searches << "\n";
    std::cout << "Number of successes: " << successes << "\n";
}