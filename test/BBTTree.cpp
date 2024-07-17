#include "gtest/gtest.h"

#include <random>

#include "BiasedBinaryTree.hpp"

constexpr uint64_t bigNum = 1000000;

template <class Key, class Val>
class BiasedBinaryTreeTest : public BiasedBinaryTree<Key, Val> {
public:
    BiasedBinaryTreeTest(std::function<bool(Key, Key)> pLess) : BiasedBinaryTree<Key, Val>(pLess){

    }

    FRIEND_TEST(BBTTree, basicJoinTest);
    FRIEND_TEST(BBTTree, basicSplitAtNodeTest);
    FRIEND_TEST(BBTTree, basicSplitMidTest);
    FRIEND_TEST(BBTTree, largeJoinTest);
    FRIEND_TEST(BBTTree, largeSplitTest);
};

TEST(BBTTree, basicJoinTest){
    BiasedBinaryTreeTest<int, int> tree(std::function<bool(int, int)> {[](int a, int b) {
            return a < b;
        }
    });

    BiasedBinaryTreeTest<int, int>::Node leaf1(5,0,1), leaf2(6,1,8), leaf3(7,2,2), leaf4(8,3,16);
    BiasedBinaryTreeTest<int, int>::Node conn1, conn2, conn3;
    BiasedBinaryTreeTest<int, int>::Node* path1 = tree.globalJoin(&leaf1, &leaf2, &conn1);
    BiasedBinaryTreeTest<int, int>::Node* path2 = tree.globalJoin(&leaf3, &leaf4, &conn2);
    BiasedBinaryTreeTest<int, int>::Node* path3 = tree.globalJoin(path1, path2, &conn3);

    tree.root = path3;

    EXPECT_TRUE(leaf1.isLeaf);

    std::vector<std::pair<int, int>> orderedSet = tree.getSortedSet();

    for(count i = 0; i < 4; i++){
        EXPECT_EQ(orderedSet[i].first, 5+i);
        EXPECT_EQ(orderedSet[i].second, i);
    }

    tree.root = nullptr;
}

TEST(BBTTree, largeJoinTest){
    count n = 1000;

    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dis(0, bigNum);

    std::vector<std::vector<BiasedBinaryTreeTest<int, int>::Node*>> pathsSoFar;
    for(count i = 0; i < n; i++){
        BiasedBinaryTreeTest<int, int>::Node* node = new BiasedBinaryTreeTest<int, int>::Node(i, i, dis(rng) % (n) + 1);
        pathsSoFar.push_back({node});
    }
    BiasedBinaryTreeTest<int, int> tree(std::function<bool(int, int)> {[](int a, int b) {
            return a < b;
        }
    });

    count p1, p2;

    while (pathsSoFar.size() > 1){
        //Get two random paths and join them
        p1 = dis(rng) % (pathsSoFar.size() - 1);
        p2 = p1 + 1;

        tree.globalJoin(tree.getRoot(pathsSoFar[p1][0]), tree.getRoot(pathsSoFar[p2][0]), new BiasedBinaryTreeTest<int, int>::Node());

        //Join their vectors too, so we always know how the paths look
        for(BiasedBinaryTreeTest<int, int>::Node* node : pathsSoFar[p2]){
            pathsSoFar[p1].push_back(node);
        }

        //Now move down the path of maximum index and reduce vector size
        for(count i = p2; i < pathsSoFar.size() - 1; i++){
            pathsSoFar[i] = pathsSoFar[i+1];
        }
        pathsSoFar.pop_back();

        //Now check if the new path remains valid
        tree.root = tree.getRoot(pathsSoFar[p1][0]);
        std::vector<std::pair<int, int>> path = tree.getSortedSet();
        EXPECT_EQ(path.size(), pathsSoFar[p1].size());
        for(count i = 0; i < path.size(); i++){
            EXPECT_EQ(path[i].first, pathsSoFar[p1][i]->k);
            EXPECT_EQ(path[i].second, pathsSoFar[p1][i]->v);
        }
    }
}

TEST(BBTTree, basicSplitAtNodeTest){
    BiasedBinaryTreeTest<int, int> tree(std::function<bool(int, int)> {[](int a, int b) {
        return a < b;
    }
    });

    BiasedBinaryTreeTest<int, int>::Node leaf1(5,0,1), leaf2(6,1,8), leaf3(7,2,2), leaf4(8,3,16);
    BiasedBinaryTreeTest<int, int>::Node conn1, conn2, conn3;
    BiasedBinaryTreeTest<int, int>::Node* path1 = tree.globalJoin(&leaf1, &leaf2, &conn1);
    BiasedBinaryTreeTest<int, int>::Node* path2 = tree.globalJoin(&leaf3, &leaf4, &conn2);
    BiasedBinaryTreeTest<int, int>::Node* path3 = tree.globalJoin(path1, path2, &conn3);

    BiasedBinaryTreeTest<int, int>::SplitResult splitResult = tree.split(path3, 6);

    tree.root = splitResult.leftTree;
    std::vector<std::pair<int, int>> pathA = tree.getSortedSet();
    tree.root = splitResult.rightTree;
    std::vector<std::pair<int, int>> pathB = tree.getSortedSet();

    EXPECT_EQ(leaf2.parent, nullptr);
    EXPECT_EQ(splitResult.splitNode, &leaf2);

    EXPECT_NE(splitResult.leftSplitNode, splitResult.rightSplitNode);
    EXPECT_NE(splitResult.leftSplitNode, nullptr);
    EXPECT_NE(splitResult.rightSplitNode, nullptr);

    EXPECT_EQ(pathA.size(), 1);
    EXPECT_EQ(pathB.size(), 2);

    EXPECT_EQ(pathA[0].first, 5);
    EXPECT_EQ(pathA[0].second, 0);
    EXPECT_EQ(pathB[0].first, 7);
    EXPECT_EQ(pathB[0].second, 2);
    EXPECT_EQ(pathB[1].first, 8);
    EXPECT_EQ(pathB[1].second, 3);

    tree.root = nullptr;
}

TEST(BBTTree, basicSplitMidTest){
    BiasedBinaryTreeTest<int, int> tree(std::function<bool(int, int)> {[](int a, int b) {
        return a < b;
    }
    });

    BiasedBinaryTreeTest<int, int>::Node leaf1(3,0,1), leaf2(4,1,8), leaf3(7,2,2), leaf4(8,3,16);
    BiasedBinaryTreeTest<int, int>::Node conn1, conn2, conn3;
    BiasedBinaryTreeTest<int, int>::Node* path1 = tree.globalJoin(&leaf1, &leaf2, &conn1);
    BiasedBinaryTreeTest<int, int>::Node* path2 = tree.globalJoin(&leaf3, &leaf4, &conn2);
    BiasedBinaryTreeTest<int, int>::Node* path3 = tree.globalJoin(path1, path2, &conn3);

    BiasedBinaryTreeTest<int, int>::SplitResult splitResult = tree.split(path3, 5);

    tree.root = splitResult.leftTree;
    std::vector<std::pair<int, int>> pathA = tree.getSortedSet();
    tree.root = splitResult.rightTree;
    std::vector<std::pair<int, int>> pathB = tree.getSortedSet();

    EXPECT_EQ(splitResult.splitNode, nullptr);

    EXPECT_NE(splitResult.leftSplitNode, splitResult.rightSplitNode);
    EXPECT_TRUE(splitResult.leftSplitNode == nullptr || splitResult.rightSplitNode == nullptr);

    EXPECT_EQ(pathA.size(), 2);
    EXPECT_EQ(pathB.size(), 2);

    EXPECT_EQ(pathA[0].first, 3);
    EXPECT_EQ(pathA[0].second, 0);
    EXPECT_EQ(pathA[1].first, 4);
    EXPECT_EQ(pathA[1].second, 1);
    EXPECT_EQ(pathB[0].first, 7);
    EXPECT_EQ(pathB[0].second, 2);
    EXPECT_EQ(pathB[1].first, 8);
    EXPECT_EQ(pathB[1].second, 3);

    tree.root = nullptr;
}

TEST(BBTTree, largeSplitTest){
    count n = 1000;

    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dis(0, bigNum);

    std::vector<std::vector<BiasedBinaryTreeTest<int, int>::Node*>> pathsSoFar;
    for(count i = 0; i < n; i++){
        BiasedBinaryTreeTest<int, int>::Node* node = new BiasedBinaryTreeTest<int, int>::Node(2*i, 2*i, dis(rng) % (n) + 1);
        pathsSoFar.push_back({node});
    }
    BiasedBinaryTreeTest<int, int> tree(std::function<bool(int, int)> {[](int a, int b) {
        return a < b;
    }
    });

    count p1, p2;

    for (count i = 0; i < n/2; i++){
        //Get two random paths and join them
        p1 = dis(rng) % (pathsSoFar.size() - 1);
        p2 = p1 + 1;

        tree.globalJoin(tree.getRoot(pathsSoFar[p1][0]), tree.getRoot(pathsSoFar[p2][0]), new BiasedBinaryTreeTest<int, int>::Node());
        //Join their vectors too, so we always know how the paths look
        for(BiasedBinaryTreeTest<int, int>::Node* node : pathsSoFar[p2]){
            pathsSoFar[p1].push_back(node);
        }

        //Now move down the path of maximum index and reduce vector size
        for(count j = p2; j < pathsSoFar.size() - 1; j++){
            pathsSoFar[j] = pathsSoFar[j+1];
        }
        pathsSoFar.pop_back();

        //Now check if the new path remains valid
        tree.root = tree.getRoot(pathsSoFar[p1][0]);
        std::vector<std::pair<int, int>> path = tree.getSortedSet();
        EXPECT_EQ(path.size(), pathsSoFar[p1].size());
        for(count i = 0; i < path.size(); i++){
            EXPECT_EQ(path[i].first, pathsSoFar[p1][i]->k);
            EXPECT_EQ(path[i].second, pathsSoFar[p1][i]->v);
        }
    }

    for(count i = 0; i < n; i++){
        //First do a join, then do a split
        if(pathsSoFar.size() >= n/2) {
            //Get two random paths and join them
            p1 = dis(rng) % (pathsSoFar.size() - 1);
            p2 = p1 + 1;

            tree.globalJoin(tree.getRoot(pathsSoFar[p1][0]), tree.getRoot(pathsSoFar[p2][0]),
                            new BiasedBinaryTreeTest<int, int>::Node());
            //Join their vectors too, so we always know how the paths look
            for (BiasedBinaryTreeTest<int, int>::Node *node: pathsSoFar[p2]) {
                pathsSoFar[p1].push_back(node);
            }

            //Now move down the path of maximum index and reduce vector size
            for (count j = p2; j < pathsSoFar.size() - 1; j++) {
                pathsSoFar[j] = pathsSoFar[j + 1];
            }
            pathsSoFar.pop_back();

            //Now check if the new path remains valid
            tree.root = tree.getRoot(pathsSoFar[p1][0]);
            std::vector<std::pair<int, int>> path = tree.getSortedSet();
            EXPECT_EQ(path.size(), pathsSoFar[p1].size());
            for (count i = 0; i < path.size(); i++) {
                EXPECT_EQ(path[i].first, pathsSoFar[p1][i]->k);
                EXPECT_EQ(path[i].second, pathsSoFar[p1][i]->v);
            }
        }

        //Now split some stuff
        p1 = dis(rng) % (pathsSoFar.size() - 1);
        count splitKey = dis(rng) % 2*n;
        BiasedBinaryTreeTest<int, int>::SplitResult splitResult = tree.split(tree.getRoot(pathsSoFar[p1][0]), splitKey);

        //Create 3 new vectors with the split paths in
        std::vector<BiasedBinaryTreeTest<int, int>::Node*> leftPart, middlePart, rightPart;

        for(BiasedBinaryTreeTest<int, int>::Node* v : pathsSoFar[p1]){
            if(v->k < splitKey) leftPart.push_back(v);
            else if(v->k == splitKey) middlePart.push_back(v);
            else if(v->k > splitKey) rightPart.push_back(v);
        }

        //shift around the segments properly
        count newSegments = 0;
        if(splitResult.leftTree != nullptr) newSegments++;
        if(splitResult.splitNode != nullptr) newSegments++;
        if(splitResult.rightTree != nullptr) newSegments++;

        pathsSoFar.resize(pathsSoFar.size() + newSegments - 1);
        for(count j = pathsSoFar.size() - 1; j > p1 + newSegments - 1; j--){
            pathsSoFar[j] = pathsSoFar[j - newSegments + 1];
        }

        count j = p1;

        if(splitResult.leftTree != nullptr){
            pathsSoFar[j] = leftPart;
            //Now check if the new path remains valid
            tree.root = tree.getRoot(pathsSoFar[j][0]);
            std::vector<std::pair<int, int>> path = tree.getSortedSet();
            EXPECT_EQ(path.size(), pathsSoFar[j].size());
            for(count j2 = 0; j2 < path.size(); j2++){
                EXPECT_EQ(path[j2].first, pathsSoFar[j][j2]->k);
                EXPECT_EQ(path[j2].second, pathsSoFar[j][j2]->v);
            }
            j++;
        }
        if(splitResult.splitNode != nullptr){
            pathsSoFar[j] = middlePart;
            //Now check if the new path remains valid
            tree.root = tree.getRoot(pathsSoFar[j][0]);
            std::vector<std::pair<int, int>> path = tree.getSortedSet();
            EXPECT_EQ(path.size(), pathsSoFar[j].size());
            for(count j2 = 0; j2 < path.size(); j2++){
                EXPECT_EQ(path[j2].first, pathsSoFar[j][j2]->k);
                EXPECT_EQ(path[j2].second, pathsSoFar[j][j2]->v);
            }
            j++;
        }
        if(splitResult.rightTree != nullptr){
            pathsSoFar[j] = rightPart;
            //Now check if the new path remains valid
            tree.root = tree.getRoot(pathsSoFar[j][0]);
            std::vector<std::pair<int, int>> path = tree.getSortedSet();
            EXPECT_EQ(path.size(), pathsSoFar[j].size());
            for(count j2 = 0; j2 < path.size(); j2++){
                EXPECT_EQ(path[j2].first, pathsSoFar[j][j2]->k);
                EXPECT_EQ(path[j2].second, pathsSoFar[j][j2]->v);
            }
            j++;
        }

        if(newSegments == 3) {
            EXPECT_TRUE(splitResult.leftSplitNode != nullptr && splitResult.rightSplitNode != nullptr);
        }
        if(newSegments == 2) {
            EXPECT_NE(splitResult.leftSplitNode != nullptr, splitResult.rightSplitNode != nullptr);
        }
        if(newSegments == 1){
            EXPECT_TRUE( splitResult.leftSplitNode == nullptr && splitResult.rightSplitNode == nullptr);
        }
        delete splitResult.leftSplitNode;
        delete splitResult.rightSplitNode;
    }
}

TEST(BBTTree, basicInsertContainSTest){
    BiasedBinaryTree<int, int> tree;

    tree.insert(0,0,1);
    tree.insert(5,5,17);
    tree.insert(8,8,3);
    tree.insert(3,3,12);

    EXPECT_TRUE(tree.contains(3));
    EXPECT_TRUE(tree.contains(0));
    EXPECT_FALSE(tree.contains(7));

    EXPECT_EQ(tree.findVal(5), 5);
    EXPECT_EQ(tree.findVal(8), 8);

    EXPECT_ANY_THROW(tree.insert(8,8,8));
}

TEST(BBTTree, basicDeleteContainsTest){
    BiasedBinaryTree<int, int> tree;

    tree.insert(0,0,1);
    tree.insert(5,5,17);
    tree.insert(8,8,3);
    tree.insert(3,3,12);

    EXPECT_TRUE(tree.contains(5));

    tree.remove(5);

    EXPECT_FALSE(tree.contains(5));

    EXPECT_TRUE(tree.contains(0));

    tree.remove(0);

    EXPECT_FALSE(tree.contains(0));
}

TEST(BBTTree, largeInsertTest){
    count n = 1000;

    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dis(0, bigNum);

    std::set<int> expected;
    BiasedBinaryTree<int, int> actual;

    int num = 0;

    for(count i = 0; i < n/2; i++){
        //Add a new number
        do num = dis(rng) % n; while(expected.contains(num));

        expected.insert(num);
        actual.insert(num, num, (dis(rng) % n) + 1);

        //Now check if the two sets are equal
        for(int j = 0; j < n; j++){
            EXPECT_EQ(expected.contains(j), actual.contains(j));
            if(actual.contains(j)) EXPECT_EQ(j, actual.findVal(j));
        }
    }
}

TEST(BBTTree, largeInsertDeleteTest){
    count n = 500;

    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dis(0, bigNum);

    std::set<int> expected;
    BiasedBinaryTree<int, int> actual;

    int num = 0;

    for(count i = 0; i < n/2; i++){
        //Add a new number
        do num = dis(rng) % n; while(expected.contains(num));

        expected.insert(num);
        actual.insert(num, num, (dis(rng) % n) + 1);
    }

    for(count i = 0; i < 5*n; i++){
        //Add a new number
        do num = dis(rng) % n; while(expected.contains(num));

        expected.insert(num);
        actual.insert(num, num, (dis(rng) % n) + 1);

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