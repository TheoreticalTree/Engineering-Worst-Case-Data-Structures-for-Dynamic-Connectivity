#include "gtest/gtest.h"

#include <random>

#include "base.hpp"
#include "DTree.hpp"
#include "HDT.hpp"

constexpr uint64_t bigNum = 1000000;

TEST(HDT, simpleInsertTest){
    Graph graph(6);
    graph.addEdge(0, 1);
    graph.addEdge(1, 2);
    graph.addEdge(0, 2);
    graph.addEdge(2, 3);
    graph.addEdge(4, 5);

    HDT algo(graph);

    EXPECT_TRUE(algo.query(0, 3));
    EXPECT_FALSE(algo.query(0, 4));
}

TEST(HDT, simpleInsertDeleteTest) {
    HDT algo(6);

    algo.addEdge(0, 1);
    algo.addEdge(0, 2);
    algo.addEdge(1, 2);
    algo.addEdge(2, 3);
    algo.addEdge(4, 5);

    EXPECT_TRUE(algo.query(0, 2));
    EXPECT_FALSE(algo.query(0, 4));

    algo.addEdge(3, 5);

    EXPECT_TRUE(algo.query(0, 2));
    EXPECT_TRUE(algo.query(0, 4));

    algo.deleteEdge(0, 2);

    EXPECT_TRUE(algo.query(0, 2));
    EXPECT_TRUE(algo.query(0, 4));

    algo.deleteEdge(0, 1);

    EXPECT_FALSE(algo.query(0, 2));
    EXPECT_FALSE(algo.query(0, 4));
}

TEST(HDT, largeInsertTest) {
    count n = 100;

    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dis(0, bigNum);

    HDT actual(n);
    DTree expected(n);

    std::vector<edge> edgeList;

    count errorStates = 0;
    count errors = 0;

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

    bool falseNow;

    //Insert all the edges
    count i = 0;
    for(edge e : edgeList){
        expected.addEdge(e.v, e.w);
        actual.addEdge(e.v, e.w);

        if (expected.numberOfComponents() != actual.numberOfComponents()) {
            errorStates++;
            falseNow = true;
        }
        else falseNow = false;

        i++;
        std::cout << "Inserted " << i << " edges\n";

        for(node u = 0; u < n; u++)
            for (node v = 0; v < n; v++) {
                if (not falseNow)
                    EXPECT_EQ(expected.query(u, v), actual.query(u, v));
                else {
                    if(expected.query(u, v)) {
                        if (not actual.query(u, v))
                            errors++;
                    }
                    else {
                        EXPECT_FALSE(actual.query(u, v));
                    }
                }
            }
    }

    std::cout << "Number errorStates: " << errorStates << "\n";
    std::cout << "Number errors: " << errors << "\n";
}

TEST(HDT, largeInsertDeleteTest) {
    count n = 100;

    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dis(0, bigNum);

    HDT actual(n);
    DTree expected(n);

    std::vector<edge> edgeList;

    count errorStates = 0;
    count errors = 0;

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

    bool falseNow;

    //Insert all the edges
    count i = 0;
    for(edge e : edgeList){
        expected.addEdge(e.v, e.w);
        actual.addEdge(e.v, e.w);

        if (expected.numberOfComponents() != actual.numberOfComponents()) {
            errorStates++;
            falseNow = true;
        }
        else falseNow = false;

        i++;
        std::cout << "Inserted " << i << " edges\n";

        for(node u = 0; u < n; u++)
            for (node v = 0; v < n; v++) {
                if (not falseNow)
                    EXPECT_EQ(expected.query(u, v), actual.query(u, v));
                else {
                    if(expected.query(u, v)) {
                        if (not actual.query(u, v))
                            errors++;
                    }
                    else {
                        EXPECT_FALSE(actual.query(u, v));
                    }
                }
            }
    }

    //Randomize the edgelist again
    for(count i = 0; i < edgeList.size(); i++){
        count swapPos = dis(rng) % (edgeList.size() - i) + i;
        if(swapPos != i) std::swap(edgeList[i], edgeList[swapPos]);
    }

    //Delete all the edges
    i = 0;
    for(edge e : edgeList){
        actual.deleteEdge(e.v, e.w);
        expected.deleteEdge(e.v, e.w);

        if (expected.numberOfComponents() != actual.numberOfComponents()) {
            errorStates++;
            falseNow = true;
        }
        else falseNow = false;

        i++;
        std::cout << "Deleted " << i << " edges\n";

        for(node u = 0; u < n; u++)
            for (node v = 0; v < n; v++) {
                if (not falseNow)
                    EXPECT_EQ(expected.query(u, v), actual.query(u, v));
                else {
                    if(expected.query(u, v)) {
                        if (not actual.query(u, v))
                            errors++;
                    }
                    else {
                        EXPECT_FALSE(actual.query(u, v));
                    }
                }
            }
    }

    std::cout << "Number errorStates: " << errorStates << "\n";
    std::cout << "Number errors: " << errors << "\n";
}

TEST(HDT, profileTest) {
    count n = 1000;

    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dis(0, bigNum);

    HDT actual(n);

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
    for(edge e : edgeList){
        actual.addEdge(e.v, e.w);
    }

    //Randomize the edgelist again
    for(count i = 0; i < edgeList.size(); i++){
        count swapPos = dis(rng) % (edgeList.size() - i) + i;
        if(swapPos != i) std::swap(edgeList[i], edgeList[swapPos]);
    }

    for(edge e : edgeList){
        actual.deleteEdge(e.v, e.w);
    }
}