#include "gtest/gtest.h"

#include <deque>
#include <random>
#include <iostream>

#include "ETForestCutSet.hpp"
//To check connected components
#include "PrimitiveStructures/RootedTree.hpp"

constexpr uint64_t bigNum = 1000000;

class ETForestTest : public ETForestCutSet {
public:
    ETForestTest() : ETForestCutSet(0, 0, true){}

    FRIEND_TEST(ETForestCutSet, basicJoinTest);
    FRIEND_TEST(ETForestCutSet, basicSplitTest);
    FRIEND_TEST(ETForestCutSet, largeJoinTest);
    FRIEND_TEST(ETForestCutSet, largeSplitTest);

    class NodeTest : public Node {
    public:
        FRIEND_TEST(ETForestCutSet, basicJoinTest);
        FRIEND_TEST(ETForestCutSet, basicSplitTest);
        FRIEND_TEST(ETForestCutSet, largeJoinTest);
        FRIEND_TEST(ETForestCutSet, largeSplitTest);

        NodeTest(node pv, node pw) : Node(pv, pw, {}){}
    };
};

TEST(ETForestCutSet, basicJoinTest){
    ETForestTest etForest;

    ETForestTest::NodeTest node1(0,1), node2(1,2), node3(2,3), node4(3,4), node5(4,5), node6(5,6), node7(6,7);

    ETForestTest::Node* join1 = etForest.join(&node1, &node3, &node2);
    ETForestTest::Node* join2 = etForest.join(join1, &node5, &node4);
    ETForestTest::Node* join3 = etForest.join(join2, &node7, &node6);

    std::vector<std::pair<node, node>> sequence;
    etForest.writeTour(join3, &sequence);

    EXPECT_EQ(sequence.size(), 7);

    for(count i = 0; i < 7; i++){
        EXPECT_EQ(i, sequence[i].first);
        EXPECT_EQ(i+1, sequence[i].second);
    }
}

TEST(ETForestCutSet, basicSplitTest){
    ETForestTest etForest;

    ETForestTest::NodeTest node1(0,1), node2(1,2), node3(2,3), node4(3,4), node5(4,5), node6(5,6), node7(6,7);

    ETForestTest::Node* join1 = etForest.join(&node1, &node3, &node2);
    ETForestTest::Node* join2 = etForest.join(join1, &node5, &node4);
    ETForestTest::Node* join3 = etForest.join(join2, &node7, &node6);

    std::pair<address, address> splitResults = etForest.split(&node3);

    std::vector<std::pair<node, node>> sequence1, sequence2;
    etForest.writeTour(splitResults.first, &sequence1);
    etForest.writeTour(splitResults.second, &sequence2);

    EXPECT_EQ(sequence1.size(), 2);
    EXPECT_EQ(sequence2.size(), 5);

    for(count i = 0; i < 2; i++){
        EXPECT_EQ(i, sequence1[i].first);
        EXPECT_EQ(i+1, sequence1[i].second);
    }

    for(count i = 0; i < 5; i++){
        EXPECT_EQ(i + 2, sequence2[i].first);
        EXPECT_EQ(i + 3, sequence2[i].second);
    }
}

void checkSequenceEquality(std::vector<std::vector<std::pair<node, node>>> &sequencesExpected, std::vector<std::vector<ETForestTest::NodeTest*>> &sequencesActual){
    EXPECT_EQ(sequencesActual.size(), sequencesExpected.size());

    for(count i = 0; i < sequencesExpected.size(); i++){
        EXPECT_EQ(sequencesExpected[i].size(), sequencesActual[i].size());

        for(count j = 0; j < sequencesExpected[i].size(); j++){
            EXPECT_EQ(sequencesExpected[i][j].first, sequencesActual[i][j]->getV());
            EXPECT_EQ(sequencesExpected[i][j].second, sequencesActual[i][j]->getW());

            if(j != 0){
                EXPECT_EQ(sequencesExpected[i][j-1].second, sequencesExpected[i][j].first);
            }
        }
    }
}

TEST(ETForestCutSet, largeJoinTest){
    count n = 500;

    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dis(0, bigNum);

    ETForestTest etForest;

    std::vector<std::vector<std::pair<node, node>>> sequencesExpected;
    std::vector<ETForestTest::Node*> sequencesActualRoots;

    //Generate the base sequences
    for(count i = 0; i < n; i++){
        sequencesExpected.push_back({{i, i+1}});
        ETForestTest::Node* node = new ETForestTest::NodeTest(i,i+1);
        sequencesActualRoots.push_back(node);
    }

    count p1 = 0, p2 = 0;

    while (sequencesExpected.size() > 1){
        //Get two random sequences to join
        p1 = dis(rng) % sequencesExpected.size();
        do p2 = dis(rng) % sequencesExpected.size(); while(p1 == p2);

        if(p1 > p2) std::swap(p1, p2);

        //Modify the expected sequences
        std::pair<node, node> connectEdge = {sequencesExpected[p1].back().second, sequencesExpected[p2][0].first};
        sequencesExpected[p1].emplace_back(connectEdge);
        for(std::pair<node, node> e : sequencesExpected[p2]) sequencesExpected[p1].push_back(e);
        sequencesExpected[p2] = sequencesExpected.back();
        sequencesExpected.pop_back();

        //Modify the actual sequences
        ETForestTest::Node* newNode = new ETForestTest::NodeTest(connectEdge.first, connectEdge.second);
        sequencesActualRoots[p1] = etForest.join(sequencesActualRoots[p1], sequencesActualRoots[p2], newNode);
        sequencesActualRoots[p2] = sequencesActualRoots.back();
        sequencesActualRoots.pop_back();

        //Check if all tours remain valid
        for(count i = 0; i < sequencesExpected.size(); i++){
            std::vector<std::pair<node, node>> actualSequence;
            etForest.writeTour(sequencesActualRoots[i], &actualSequence);

            EXPECT_EQ(sequencesExpected[i].size(), actualSequence.size());

            for(count j = 0; j < sequencesExpected[i].size(); j++){
                EXPECT_EQ(sequencesExpected[i][j], actualSequence[j]);
            }
        }
    }
}

TEST(ETForestCutSet, largeSplitTest){
    count n = 200;

    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dis(0, bigNum);

    ETForestTest etForest;

    std::vector<std::vector<std::pair<node, node>>> sequencesExpected;
    std::vector<ETForestTest::Node*> sequencesActualRoots;
    std::vector<std::vector<ETForestTest::Node*>> sequenceActualNodes;

    //Generate the base sequences
    for(count i = 0; i < n; i++){
        sequencesExpected.push_back({{i, i+1}});
        ETForestTest::Node* node = new ETForestTest::NodeTest(i,i+1);
        sequencesActualRoots.push_back(node);
        sequenceActualNodes.push_back({node});
    }

    count p1 = 0, p2 = 0;

    //Build up some sequences
    for(count c = 0; c < n/2; c++){
        //Get two random sequences to join
        p1 = dis(rng) % sequencesExpected.size();
        do p2 = dis(rng) % sequencesExpected.size(); while(p1 == p2);

        if(p1 > p2) std::swap(p1, p2);

        //Modify the expected sequences
        std::pair<node, node> connectEdge = {sequencesExpected[p1].back().second, sequencesExpected[p2][0].first};
        sequencesExpected[p1].emplace_back(connectEdge);
        for(std::pair<node, node> e : sequencesExpected[p2]) sequencesExpected[p1].push_back(e);
        sequencesExpected[p2] = sequencesExpected.back();
        sequencesExpected.pop_back();

        //Modify the actual sequences
        ETForestTest::Node* newNode = new ETForestTest::NodeTest(connectEdge.first, connectEdge.second);
        sequencesActualRoots[p1] = etForest.join(sequencesActualRoots[p1], sequencesActualRoots[p2], newNode);
        sequencesActualRoots[p2] = sequencesActualRoots.back();
        sequencesActualRoots.pop_back();
        sequenceActualNodes[p1].push_back(newNode);
        for(ETForestTest::Node* e : sequenceActualNodes[p2]) sequenceActualNodes[p1].push_back(e);
        sequenceActualNodes[p2] = sequenceActualNodes.back();
        sequenceActualNodes.pop_back();
    }

    //Now always perform a split and then a join
    for(count c = 0; c < n; c++){
        //Perform a split
        p1 = dis(rng) % sequencesExpected.size();
        p2 = dis(rng) % sequencesExpected[p1].size();

        if(p2 != 0){
            sequencesExpected.emplace_back();
            for(count i = p2; i < sequencesExpected[p1].size(); i++) sequencesExpected.back().push_back(sequencesExpected[p1][i]);
            sequencesExpected[p1].resize(p2);
        }

        auto splitResults = etForest.split(sequenceActualNodes[p1][p2]);
        if(splitResults.first != nullptr){
            sequencesActualRoots[p1] = splitResults.first;
            sequencesActualRoots.push_back(splitResults.second);
            sequenceActualNodes.emplace_back();
            for(count i = p2; i < sequenceActualNodes[p1].size(); i++) sequenceActualNodes.back().push_back(sequenceActualNodes[p1][i]);
            sequenceActualNodes[p1].resize(p2);
        }
        else {
            sequencesActualRoots[p1] = splitResults.second;
        }

        //Check if all tours remain valid
        for(count i = 0; i < sequencesExpected.size(); i++){
            std::vector<std::pair<node, node>> actualSequence;
            etForest.writeTour(sequencesActualRoots[i], &actualSequence);

            if(sequencesExpected[i].size() != sequenceActualNodes[i].size()){
                std::cout << "sus\n";
            }

            EXPECT_EQ(sequencesExpected[i].size(), sequenceActualNodes[i].size());
            EXPECT_EQ(sequencesExpected[i].size(), actualSequence.size());

            for(count j = 0; j < sequencesExpected[i].size(); j++){
                EXPECT_EQ(sequencesExpected[i][j], actualSequence[j]);
            }
        }

        //Now join two random sequences
        p1 = dis(rng) % sequencesExpected.size();
        do p2 = dis(rng) % sequencesExpected.size(); while(p1 == p2);

        if(p1 > p2) std::swap(p1, p2);

        //Modify the expected sequences
        std::pair<node, node> connectEdge = {sequencesExpected[p1].back().second, sequencesExpected[p2][0].first};
        sequencesExpected[p1].emplace_back(connectEdge);
        for(std::pair<node, node> e : sequencesExpected[p2]) sequencesExpected[p1].push_back(e);
        sequencesExpected[p2] = sequencesExpected.back();
        sequencesExpected.pop_back();

        //Modify the actual sequences
        ETForestTest::Node* newNode = new ETForestTest::NodeTest(connectEdge.first, connectEdge.second);
        sequencesActualRoots[p1] = etForest.join(sequencesActualRoots[p1], sequencesActualRoots[p2], newNode);
        sequencesActualRoots[p2] = sequencesActualRoots.back();
        sequencesActualRoots.pop_back();
        sequenceActualNodes[p1].emplace_back(newNode);
        for(ETForestTest::Node* e : sequenceActualNodes[p2]) sequenceActualNodes[p1].emplace_back(e);
        sequenceActualNodes[p2] = sequenceActualNodes.back();
        sequenceActualNodes.pop_back();

        //Check if all tours remain valid
        for(count i = 0; i < sequencesExpected.size(); i++){
            std::vector<std::pair<node, node>> actualSequence;
            etForest.writeTour(sequencesActualRoots[i], &actualSequence);

            if(sequencesExpected[i].size() != sequenceActualNodes[i].size()){
                std::cout << "sus\n";
            }

            EXPECT_EQ(sequencesExpected[i].size(), sequenceActualNodes[i].size());
            EXPECT_EQ(sequencesExpected[i].size(), actualSequence.size());

            for(count j = 0; j < sequencesExpected[i].size(); j++){
                EXPECT_EQ(sequencesExpected[i][j], actualSequence[j]);
            }
        }
    }
}

struct TestEdge {
    node v1, v2;
    address ETEdge;
};

std::vector<std::vector<std::pair<node, node>>> getTours(ETForestCutSet &etForest, std::vector<TestEdge> &testEdges){
    std::vector<std::vector<std::pair<node, node>>> ret;

    for(TestEdge e : testEdges){
        if(e.ETEdge == etForest.getRoot(e.ETEdge)){
            ret.emplace_back(etForest.getTour(e.ETEdge));
        }
    }

    return ret;
}

void testEulerTours(ETForestCutSet &etForest, std::vector<TestEdge> &testEdges) {
    std::vector<std::vector<std::pair<node, node>>> tours = getTours(etForest, testEdges);

    int edgeIndex, backEdgeIndex;

    //Check for every edge if the edge and backedge are in the same tour
    for(TestEdge e : testEdges){
        edgeIndex = -1;
        backEdgeIndex = -1;

        for(int i = 0; i < tours.size(); i++){
            for(int j = 0; j < tours[i].size(); j++){
                if(tours[i][j].first == e.v1 && tours[i][j].second == e.v2){
                    if(edgeIndex == -1) edgeIndex = i;
                    else throw std::runtime_error("Edge appears twice");
                }
                if(tours[i][j].second == e.v1 && tours[i][j].first == e.v2){
                    if(backEdgeIndex == -1) backEdgeIndex = i;
                    else throw std::runtime_error("BackEdge appears twice");
                }
            }
        }

        EXPECT_EQ(edgeIndex, backEdgeIndex);
    }

    //Now check if every tour is a valid one
    for(std::vector<std::pair<node, node>> tour : tours){
        EXPECT_EQ(tour[0].first, tour.back().second);

        for(int i = 0; i < tour.size() - 1; i++){
            EXPECT_EQ(tour[i].second, tour[i+1].first);
        }
    }
};

TEST(ETForestCutSet, ETTreesIncremental){
    count n = 100;

    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dis(0, bigNum);

    ETForestTest etForest;
    RootedForest rootedForest(n);

    std::vector<TestEdge> edges;

    for(count i = 0; i < n - 1; i++){
        node v, w;
        v = dis(rng) % n;
        do w = dis(rng) % n; while (rootedForest.getRoot(v) == rootedForest.getRoot(w));

        rootedForest.link(v,w,0);

        address vEdge = nullptr;
        address wEdge = nullptr;

        for(TestEdge e : edges){
            if(e.v1 == v) vEdge = e.ETEdge;
            if(e.v1 == w) wEdge = e.ETEdge;
        }

        std::pair<address, address> newEdgeAddresses = etForest.insertETEdge(v, w, vEdge, wEdge);

        edges.emplace_back(TestEdge {v, w, newEdgeAddresses.first});
        edges.emplace_back(TestEdge {w, v, newEdgeAddresses.second});

        testEulerTours(etForest, edges);
    }
}

TEST(ETForestCutSet, ETForestFullyDynamic){
    count n = 100;

    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dis(0, bigNum);

    ETForestTest etForest;
    RootedForest rootedForest(n);

    std::vector<TestEdge> edges;

    for(count i = 0; i < n/2; i++){
        node v, w;
        v = dis(rng) % n;
        do w = dis(rng) % n; while (rootedForest.getRoot(v) == rootedForest.getRoot(w));

        rootedForest.link(v,w,0);

        address vEdge = nullptr;
        address wEdge = nullptr;

        for(TestEdge e : edges){
            if(e.v1 == v) vEdge = e.ETEdge;
            if(e.v1 == w) wEdge = e.ETEdge;
        }

        std::pair<address, address> newEdgeAddresses = etForest.insertETEdge(v, w, vEdge, wEdge);

        edges.emplace_back(TestEdge {v, w, newEdgeAddresses.first});
        edges.emplace_back(TestEdge {w, v, newEdgeAddresses.second});

        testEulerTours(etForest, edges);
    }

    //Now we do stuff fully dynamic
    for(int i = 0; i < 5*n; i++){
        //Add a random edge
        node v, w;
        v = dis(rng) % n;
        do w = dis(rng) % n; while (rootedForest.getRoot(v) == rootedForest.getRoot(w));

        rootedForest.link(v,w,0);

        address vEdge = nullptr;
        address wEdge = nullptr;

        for(TestEdge e : edges){
            if(e.v1 == v) vEdge = e.ETEdge;
            if(e.v1 == w) wEdge = e.ETEdge;
        }

        std::pair<address, address> newEdgeAddresses = etForest.insertETEdge(v, w, vEdge, wEdge);

        edges.emplace_back(TestEdge {v, w, newEdgeAddresses.first});
        edges.emplace_back(TestEdge {w, v, newEdgeAddresses.second});

        testEulerTours(etForest, edges);

        //Now we delete a random edge
        int eNum = dis(rng) % edges.size();

        int eBackNum = 0;
        for(int j = 0; j < edges.size(); j++){
            if((edges[j].v1 == edges[eNum].v2) && (edges[j].v2 == edges[eNum].v1)) eBackNum = j;
        }

        rootedForest.cutEdge(edges[eNum].v1, edges[eNum].v2);

        etForest.deleteETEdge(edges[eNum].ETEdge, edges[eBackNum].ETEdge);

        if(eNum > eBackNum) std::swap(eNum, eBackNum);

        edges[eNum] = edges.back();
        edges.pop_back();
        edges[eBackNum] = edges.back();
        edges.pop_back();

        testEulerTours(etForest, edges);
    }
}