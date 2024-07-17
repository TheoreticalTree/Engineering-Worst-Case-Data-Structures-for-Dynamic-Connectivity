#include "gtest/gtest.h"

#include <random>
#include <iostream>

#include "PrimitiveStructures/RootedTree.hpp"
#include "LinkCutTrees.hpp"

constexpr uint64_t bigNum = 1000000;

//This exists so certain tests can access protected member functions and variables
class LinkCutTreesOpen : public LinkCutTrees{
    LinkCutTreesOpen(count pn) : LinkCutTrees(pn){}

    FRIEND_TEST(LinkCutTrees, simpleSpliceTest);
    FRIEND_TEST(LinkCutTrees, simpleExposeTest);
    FRIEND_TEST(LinkCutTrees, simpleConcealTest);
};

TEST(LinkCutTrees, simpleSpliceTest){
    LinkCutTreesOpen LCForest(5);
    //Set weights
    for(node i = 0; i < 5; i++){
        LCForest.pathNodes[i]->setWeight(1<<i);
    }
    //Define starting paths
    Path p1 = LCForest.forest.globalJoin(LCForest.pathNodes[0], LCForest.pathNodes[1], 0);
    Path p2 = LCForest.pathNodes[2];
    Path p3 = LCForest.forest.globalJoin(LCForest.pathNodes[3], LCForest.pathNodes[4], 0);
    LCForest.parent[1] = 3;
    LCForest.parent[2] = 3;
    LCForest.pathSets[3].insert({p1->weight, 1}, p1, p1->weight);
    LCForest.pathSets[3].insert({p2->weight, 2}, p2, p2->weight);

    Path pFinal = LCForest.splice(p1);

    EXPECT_EQ(LCForest.forest.getBBTRoot(LCForest.pathNodes[0]), pFinal);
    EXPECT_EQ(LCForest.forest.getBBTRoot(LCForest.pathNodes[1]), pFinal);
    EXPECT_EQ(LCForest.forest.getBBTRoot(LCForest.pathNodes[3]), pFinal);
    EXPECT_EQ(LCForest.forest.getBBTRoot(LCForest.pathNodes[4]), pFinal);
    EXPECT_NE(LCForest.forest.getBBTRoot(LCForest.pathNodes[2]), pFinal);

    EXPECT_EQ(LCForest.forest.getStart(pFinal), 0);
    EXPECT_EQ(LCForest.forest.getEnd(pFinal), 4);

    EXPECT_TRUE(LCForest.pathSets[3].contains({4, 2}));
    EXPECT_FALSE(LCForest.pathSets[3].contains({3, 1}));

    EXPECT_EQ(LCForest.pathNodes[3]->weight, 5);
}

TEST(LinkCutTrees, simpleExposeTest){
    LinkCutTreesOpen LCForest(7);
    //Set weights
    for(node i = 0; i < 7; i++){
        LCForest.pathNodes[i]->setWeight(1<<i);
    }
    //Define starting paths
    Path p12 = LCForest.forest.globalJoin(LCForest.pathNodes[1], LCForest.pathNodes[2], 0);
    Path p123 = LCForest.forest.globalJoin(p12, LCForest.pathNodes[3], 0);
    Path p45 = LCForest.forest.globalJoin(LCForest.pathNodes[4], LCForest.pathNodes[5], 0);
    Path p456 = LCForest.forest.globalJoin(p45, LCForest.pathNodes[6], 0);
    LCForest.parent[0] = 2;
    LCForest.parent[3] = 5;
    LCForest.pathSets[2].insert({1, 0}, LCForest.pathNodes[0], 1);
    LCForest.pathSets[5].insert({p123->weight, 3}, p123, p123->weight);

    Path pFinal = LCForest.expose(0);

    EXPECT_EQ(LCForest.forest.getBBTRoot(LCForest.pathNodes[0]), pFinal);
    EXPECT_EQ(LCForest.forest.getBBTRoot(LCForest.pathNodes[2]), pFinal);
    EXPECT_EQ(LCForest.forest.getBBTRoot(LCForest.pathNodes[3]), pFinal);
    EXPECT_EQ(LCForest.forest.getBBTRoot(LCForest.pathNodes[5]), pFinal);
    EXPECT_EQ(LCForest.forest.getBBTRoot(LCForest.pathNodes[6]), pFinal);

    EXPECT_NE(LCForest.forest.getBBTRoot(LCForest.pathNodes[1]), pFinal);
    EXPECT_NE(LCForest.forest.getBBTRoot(LCForest.pathNodes[4]), pFinal);

    EXPECT_EQ(LCForest.forest.getStart(pFinal), 0);
    EXPECT_EQ(LCForest.forest.getEnd(pFinal), 6);

    EXPECT_TRUE(LCForest.pathSets[2].contains({2, 1}));
    EXPECT_TRUE(LCForest.pathSets[5].contains({16, 4}));

    EXPECT_FALSE(LCForest.pathSets[2].contains({1, 0}));
    EXPECT_FALSE(LCForest.pathSets[5].contains({15, 3}));

    EXPECT_EQ(LCForest.pathNodes[2]->weight, 5);
    EXPECT_EQ(LCForest.pathNodes[5]->weight, 34);
}

TEST(LinkCutTrees, simpleConcealTest){
    LinkCutTreesOpen LCForest(7);
    //Set weights
    LCForest.pathNodes[0]->setWeight(2);
    LCForest.pathNodes[1]->setWeight(3);
    LCForest.pathNodes[2]->setWeight(1);
    LCForest.pathNodes[3]->setWeight(2);
    LCForest.pathNodes[4]->setWeight(8);
    LCForest.pathNodes[5]->setWeight(11);
    LCForest.pathNodes[6]->setWeight(1);

    //Define starting paths
    Path p12 = LCForest.forest.globalJoin(LCForest.pathNodes[1], LCForest.pathNodes[2], 0);
    Path p125 = LCForest.forest.globalJoin(p12, LCForest.pathNodes[5], 0);
    Path p1256 = LCForest.forest.globalJoin(p125, LCForest.pathNodes[6], 0);
    LCForest.parent[0] = 1;
    LCForest.parent[3] = 5;
    LCForest.parent[4] = 5;
    LCForest.pathSets[1].insert({2, 0}, LCForest.pathNodes[0], 1);
    LCForest.pathSets[5].insert({2, 3}, LCForest.pathNodes[3], 2);
    LCForest.pathSets[5].insert({8, 4}, LCForest.pathNodes[4], 8);

    LCForest.conceal(p1256);

    Path pFinal012 = LCForest.forest.getBBTRoot(LCForest.pathNodes[0]);
    Path pFinal3 = LCForest.forest.getBBTRoot(LCForest.pathNodes[3]);
    Path pFinal456 = LCForest.forest.getBBTRoot(LCForest.pathNodes[4]);

    //Check if all the paths are truly different
    EXPECT_NE(pFinal012, pFinal3);
    EXPECT_NE(pFinal012, pFinal456);
    EXPECT_NE(pFinal3, pFinal456);

    //Check if all vertices are in the right paths
    EXPECT_EQ(LCForest.forest.getBBTRoot(LCForest.pathNodes[0]), pFinal012);
    EXPECT_EQ(LCForest.forest.getBBTRoot(LCForest.pathNodes[1]), pFinal012);
    EXPECT_EQ(LCForest.forest.getBBTRoot(LCForest.pathNodes[2]), pFinal012);
    EXPECT_EQ(LCForest.forest.getBBTRoot(LCForest.pathNodes[3]), pFinal3);
    EXPECT_EQ(LCForest.forest.getBBTRoot(LCForest.pathNodes[4]), pFinal456);
    EXPECT_EQ(LCForest.forest.getBBTRoot(LCForest.pathNodes[5]), pFinal456);
    EXPECT_EQ(LCForest.forest.getBBTRoot(LCForest.pathNodes[6]), pFinal456);

    //Check if all the paths have the correct order
    EXPECT_EQ(LCForest.forest.getStart(pFinal012), 0);
    EXPECT_EQ(LCForest.forest.getEnd(pFinal012), 2);
    EXPECT_EQ(LCForest.forest.getStart(pFinal3), 3);
    EXPECT_EQ(LCForest.forest.getEnd(pFinal3), 3);
    EXPECT_EQ(LCForest.forest.getStart(pFinal456), 4);
    EXPECT_EQ(LCForest.forest.getEnd(pFinal456), 6);

    //Check if all the path sets were properly updated
    EXPECT_TRUE(LCForest.pathSets[1].empty());
    EXPECT_TRUE(LCForest.pathSets[5].contains({2,3}));
    EXPECT_TRUE(LCForest.pathSets[5].contains({4,2}));
    EXPECT_FALSE(LCForest.pathSets[5].contains({7,4}));

    //Check if all the weights are correct
    EXPECT_EQ(LCForest.pathNodes[0]->weight, 2);
    EXPECT_EQ(LCForest.pathNodes[1]->weight, 1);
    EXPECT_EQ(LCForest.pathNodes[2]->weight, 1);
    EXPECT_EQ(LCForest.pathNodes[3]->weight, 2);
    EXPECT_EQ(LCForest.pathNodes[4]->weight, 8);
    EXPECT_EQ(LCForest.pathNodes[5]->weight, 7);
    EXPECT_EQ(LCForest.pathNodes[6]->weight, 1);
}

void checkTrackingEquality(LinkCutTrees &actual, RootedForest &expected, count n){
    for(count i = 0; i < n; i++){
        //First test for structural properties
        EXPECT_EQ(actual.getParent(i), expected.getParent(i));
        EXPECT_EQ(actual.getRoot(i), expected.getRoot(i));
        //Now check for correct cost tracking
        if(actual.getMinEdge(i) != expected.getMinEdge(i)){
            std::cout << "sus\n";
        }
        EXPECT_EQ(actual.getCost(i), expected.getCost(i));
        EXPECT_EQ(actual.getMinEdge(i), expected.getMinEdge(i));
        EXPECT_EQ(actual.getMaxEdge(i), expected.getMaxEdge(i));
    }
}

TEST(LinkCutTrees, simpleLinkTest){
    count n = 6;
    LinkCutTrees actual(n);
    RootedForest expected(n);

    actual.link(0,1,1);
    expected.link(0,1,1);
    checkTrackingEquality(actual, expected, n);

    actual.link(2,3,2);
    expected.link(2,3,2);
    checkTrackingEquality(actual, expected, n);

    actual.link(1,3,3);
    expected.link(1,3,3);
    checkTrackingEquality(actual, expected, n);

    actual.link(4,5,4);
    expected.link(4,5,4);
    checkTrackingEquality(actual, expected, n);

    actual.link(3,5,5);
    expected.link(3,5,5);
    checkTrackingEquality(actual, expected, n);
}

TEST(LinkCutTrees, largeLinkTest){
#ifdef NDEBUG //This test should only be run in release for n > 100
    count n = 1000;
    LinkCutTrees actual(n);
    RootedForest expected(n);

    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dis(0, bigNum);

    node u, v;

    for(count i = 0; i < (n/2); i++){
        u = dis(rng) % n;
        do v = dis(rng) % n; while(expected.getRoot(u) == expected.getRoot(v));

        u = expected.getRoot(u);

        cost x = dis(rng) % n;

        actual.link(u, v, x);
        expected.link(u,v, x);

        checkTrackingEquality(actual,expected,n);
    }
#endif //NDEBUG
}

TEST(LinkCutTrees, simpleCutTest){
    count n = 6;
    LinkCutTrees actual(n);
    RootedForest expected(n);

    actual.link(0,1,1);
    expected.link(0,1,1);
    actual.link(2,3,2);
    expected.link(2,3,2);
    actual.link(1,3,3);
    expected.link(1,3,3);
    actual.link(4,5,4);
    expected.link(4,5,4);
    actual.link(3,5,5);
    expected.link(3,5,5);

    EXPECT_EQ(actual.cut(2), expected.cut(2));
    checkTrackingEquality(actual,expected,n);
    EXPECT_EQ(actual.cut(4), expected.cut(4));
    checkTrackingEquality(actual,expected,n);
    EXPECT_EQ(actual.cut(3), expected.cut(3));
    checkTrackingEquality(actual,expected,n);
}

TEST(LinkCutTrees, cimpleCutEdgeTest){
    count n = 6;
    LinkCutTrees actual(n);
    RootedForest expected(n);

    actual.link(0,1,1);
    expected.link(0,1,1);
    actual.link(2,3,2);
    expected.link(2,3,2);
    actual.link(1,3,3);
    expected.link(1,3,3);
    actual.link(4,5,4);
    expected.link(4,5,4);
    actual.link(3,5,5);
    expected.link(3,5,5);

    EXPECT_EQ(actual.cutEdge(2,3), expected.cutEdge(2,3));
    checkTrackingEquality(actual,expected,n);
    EXPECT_EQ(actual.cutEdge(5,4), expected.cutEdge(5,4));
    checkTrackingEquality(actual,expected,n);
    EXPECT_EQ(actual.cutEdge(3,5), expected.cutEdge(5,3));
    checkTrackingEquality(actual,expected,n);
}

TEST(LinkCutTrees, largeCutCutEdgeTest){
#ifdef NDEBUG //This test should only be run in release for n > 100
    count n = 500;
    LinkCutTrees actual(n);
    RootedForest expected(n);

    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dis(0, bigNum);

    node u, v;

    for(count i = 0; i < (n/2); i++){
        u = dis(rng) % n;
        do v = dis(rng) % n; while(expected.getRoot(u) == expected.getRoot(v));

        u = expected.getRoot(u);

        cost x = dis(rng) % n;

        actual.link(u, v, x);
        expected.link(u,v, x);
    }

    for(count i = 0; i < n; i++){
        //Insert one random edge
        u = dis(rng) % n;
        do v = dis(rng) % n; while(expected.getRoot(u) == expected.getRoot(v));
        u = expected.getRoot(u);
        cost x = dis(rng) % n;
        actual.link(u, v, x);
        expected.link(u,v, x);

        //Now delete one random edge
        do u = dis(rng) % n; while(expected.getParent(u) == none);
        //u now has a parent, so we separate the family
        //Flip a coin if cut or cutEdge is used
        if(dis(rng) % 2 == 0){
            EXPECT_EQ(actual.cut(u), expected.cut(u));
            checkTrackingEquality(actual, expected, n);
        }
        else {
            v = expected.getParent(u);
            if(dis(rng) % 2 == 0) std::swap(u,v);
            EXPECT_EQ(actual.cutEdge(u,v), expected.cutEdge(u,v));
            checkTrackingEquality(actual, expected, n);
        }
    }
#endif
}

//Strictly speaking this test is not needed, as cutEdge
TEST(LinkCutTrees, basicLinkCutReroot){
    count n = 6;
    LinkCutTrees actual(n);
    RootedForest expected(n);

    actual.link(0,1,1);
    expected.link(0,1,1);
    actual.link(2,3,2);
    expected.link(2,3,2);
    actual.link(1,3,3);
    expected.link(1,3,3);
    actual.link(4,5,4);
    expected.link(4,5,4);
    actual.link(3,5,5);
    expected.link(3,5,5);

    actual.reroot(1);
    expected.reroot(1);
    checkTrackingEquality(actual, expected, n);

    actual.cut(5);
    expected.cut(5);
    checkTrackingEquality(actual, expected, n);

    actual.reroot(2);
    expected.reroot(2);
    checkTrackingEquality(actual, expected, n);
}

TEST(LinkCutTrees, largeLinkCutRerootTest){
#ifdef NDEBUG //This test should only be run in release for n > 100
    count n = 500;
    LinkCutTrees actual(n);
    RootedForest expected(n);

    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dis(0, bigNum);

    node u, v;

    for(count i = 0; i < (n/2); i++){
        u = dis(rng) % n;
        do v = dis(rng) % n; while(expected.getRoot(u) == expected.getRoot(v));

        u = expected.getRoot(u);

        cost x = dis(rng) % n;

        actual.link(u, v, x);
        expected.link(u,v, x);
    }

    for(count i = 0; i < n; i++){
        //Insert one random edge
        u = dis(rng) % n;
        do v = dis(rng) % n; while(expected.getRoot(u) == expected.getRoot(v));
        u = expected.getRoot(u);
        cost x = dis(rng) % n;
        actual.link(u, v, x);
        expected.link(u,v, x);

        //Now delete one random edge
        do u = dis(rng) % n; while(expected.getParent(u) == none);
        //u now has a parent, so we separate the family
        //Flip a coin if cut or cutEdge is used
        if(dis(rng) % 2 == 0){
            EXPECT_EQ(actual.cut(u), expected.cut(u));
        }
        else {
            v = expected.getParent(u);
            if(dis(rng) % 2 == 0) std::swap(u,v);
            EXPECT_EQ(actual.cutEdge(u,v), expected.cutEdge(u,v));
        }

        //Now do a random reroot
        u = dis(rng) % n;
        actual.reroot(u);
        expected.reroot(u);
        checkTrackingEquality(actual, expected, n);
    }
#endif
}

TEST(LinkCutTrees, largeMemorySafetyTestTest){
    count n = 1000;
    LinkCutTrees actual(n);
    RootedForest expected(n);

    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dis(0, bigNum);

    node u, v;

    for(count i = 0; i < (n/2); i++){
        u = dis(rng) % n;
        do v = dis(rng) % n; while(expected.getRoot(u) == expected.getRoot(v));

        u = expected.getRoot(u);

        cost x = dis(rng) % n;

        actual.link(u, v, x);
        expected.link(u,v, x);
    }

    for(count i = 0; i < n; i++){
        //Insert one random edge
        u = dis(rng) % n;
        do v = dis(rng) % n; while(expected.getRoot(u) == expected.getRoot(v));
        u = expected.getRoot(u);
        cost x = dis(rng) % n;
        actual.link(u, v, x);
        expected.link(u,v, x);

        //Now delete one random edge
        do u = dis(rng) % n; while(expected.getParent(u) == none);
        //u now has a parent, so we separate the family
        //Flip a coin if cut or cutEdge is used
        if(dis(rng) % 2 == 0){
            EXPECT_EQ(actual.cut(u), expected.cut(u));
        }
        else {
            v = expected.getParent(u);
            if(dis(rng) % 2 == 0) std::swap(u,v);
            EXPECT_EQ(actual.cutEdge(u,v), expected.cutEdge(u,v));
        }

        //Now do a random reroot
        u = dis(rng) % n;
        actual.reroot(u);
        expected.reroot(u);
    }
}

TEST(LinkCutTrees, profilingTest){
#ifdef NDEBUG //This test only runs in debug mode
    count n = 10000;
    LinkCutTrees actual(n);

    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dis(0, bigNum);

    node u, v;

    for(count i = 0; i < (n/2); i++){
        u = dis(rng) % n;
        do v = dis(rng) % n; while(actual.getRoot(u) == actual.getRoot(v));

        cost x = dis(rng) % n;

        actual.link(u, v, x);
    }

    for(count i = 0; i < 10*n; i++){
        //Insert one random edge
        u = dis(rng) % n;
        do v = dis(rng) % n; while(actual.getRoot(u) == actual.getRoot(v));
        cost x = dis(rng) % n;
        actual.link(u, v, x);

        //Now delete one random edge
        do u = dis(rng) % n; while(actual.getParent(u) == none);
        //u now has a parent, so we separate the family
        //Flip a coin if cut or cutEdge is used
        if(dis(rng) % 2 == 0){
            actual.cut(u);
        }
        else {
            v = actual.getParent(u);
            if(dis(rng) % 2 == 0) std::swap(u,v);
            actual.cutEdge(u,v);
        }

        //Now do a random reroot
        u = dis(rng) % n;
        actual.reroot(u);
    }
#endif //NDEBUG
}