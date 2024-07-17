#include "gtest/gtest.h"

#include "BiasedBinaryForest.hpp"

constexpr bool LEFT = true;
constexpr bool RIGHT = false;

TEST(BBTNodes, getChildrenTest){
    BBTNode root, left, right;

    root.setChild(LEFT, &left);
    left.parent = &root;
    root.setChild(RIGHT, &right);
    right.parent = &root;

    EXPECT_EQ(root.getChild(LEFT), &left);
    EXPECT_EQ(root.getChild(RIGHT), &right);

    EXPECT_EQ(left.isLeftChild(), true);
    EXPECT_EQ(right.isLeftChild(), false);
}

TEST(BBTNodes, reverseChildrenTest){
    BBTNode root, left, right;

    root.setChild(LEFT, &left);
    left.parent = &root;
    root.setChild(RIGHT, &right);
    right.parent = &root;

    EXPECT_EQ(root.getChild(LEFT), &left);
    EXPECT_EQ(root.getChild(RIGHT), &right);

    EXPECT_EQ(left.isLeftChild(), true);
    EXPECT_EQ(right.isLeftChild(), false);

    root.reversalState = true;

    EXPECT_EQ(root.getChild(LEFT), &right);
    EXPECT_EQ(root.getChild(RIGHT), &left);

    EXPECT_EQ(left.isLeftChild(), false);
    EXPECT_EQ(right.isLeftChild(), true);
}

TEST(BBTNodes, setReversalState){
    BBTNode root, left, right;
    BiasedBinaryForest forest;

    root.setChild(LEFT, &left);
    left.parent = &root;
    root.setChild(RIGHT, &right);
    right.parent = &root;

    forest.testSetReversalStates(&root);

    EXPECT_EQ(root.reversalState, false);
    EXPECT_EQ(left.reversalState, false);
    EXPECT_EQ(right.reversalState, false);

    root.reversed = true;
    forest.testSetReversalStates(&root);

    EXPECT_EQ(root.reversalState, true);
    EXPECT_EQ(left.reversalState, true);
    EXPECT_EQ(right.reversalState, true);

    left.reversed = true;
    forest.testSetReversalStates(&root);

    EXPECT_EQ(root.reversalState, true);
    EXPECT_EQ(left.reversalState, false);
    EXPECT_EQ(right.reversalState, true);

    root.reversed = false;
    forest.testSetReversalStates(&root);

    EXPECT_EQ(root.reversalState, false);
    EXPECT_EQ(left.reversalState, true);
    EXPECT_EQ(right.reversalState, false);
}

TEST(BBTNodes, separateChildren){
    BBTNode root, left, right;
    BiasedBinaryForest forest;

    root.setChild(LEFT, &left);
    left.parent = &root;
    root.setChild(RIGHT, &right);
    right.parent = &root;

    //Setting some tracking data to nonboring values
    left.weight = 7;
    right.weight = 3;
    root.weight = 10;

    left.reversed = true;

    root.trackData.netCost = 5;
    left.trackData.netCost = -2;
    right.trackData.netCost = 1;

    root.trackData.netMin = 3;
    left.trackData.netMin = 0;
    right.trackData.netMin = 3;

    root.trackData.netLeftMin = -1;
    left.trackData.netLeftMin = 3;
    right.trackData.netLeftMin = 0;

    root.trackData.netRightMin = -2;
    left.trackData.netRightMin = 7;
    right.trackData.netRightMin = 5;

    std::pair<BBTNode*, BBTNode*> result = forest.testSeparateChildren(&root);

    EXPECT_EQ(result.first, &left);
    EXPECT_EQ(result.second, &right);

    EXPECT_EQ(left.reversed, true);
    EXPECT_EQ(right.reversed, false);

    EXPECT_EQ(left.trackData.netCost, 3);
    EXPECT_EQ(right.trackData.netCost, 6);

    EXPECT_EQ(left.trackData.netMin, 3);
    EXPECT_EQ(right.trackData.netMin, 6);

    EXPECT_EQ(left.getNetDirMin(LEFT), 6);
    EXPECT_EQ(right.getNetDirMin(LEFT), -8);

    EXPECT_EQ(left.getNetDirMin(RIGHT), -2);
    EXPECT_EQ(right.getNetDirMin(RIGHT), 3);
}

TEST(BBTModes, attachChildren){
    BBTNode root, left, right;
    BiasedBinaryForest forest;

    root.setChild(LEFT, &left);
    left.parent = &root;
    root.setChild(RIGHT, &right);
    right.parent = &root;

    //Setting some tracking data to nonboring values
    left.weight = 7;
    right.weight = 3;
    root.weight = 10;

    left.reversed = true;

    root.trackData.leftMost = &left;
    root.trackData.rightMost = &right;
    left.trackData.leftMost = &left;
    left.trackData.rightMost = &left;
    right.trackData.leftMost = &right;
    right.trackData.rightMost = &right;

    root.trackData.netCost = 5;
    left.trackData.netCost = -2;
    right.trackData.netCost = 1;

    root.trackData.netMin = 3;
    left.trackData.netMin = 0;
    right.trackData.netMin = 3;

    root.trackData.netLeftMin = -1;
    left.trackData.netLeftMin = 7;
    right.trackData.netLeftMin = 3;

    root.trackData.netRightMin = -4;
    left.trackData.netRightMin = 0;
    right.trackData.netRightMin = 9;

    std::pair<BBTNode*, BBTNode*> result = forest.testSeparateChildren(&root);

    forest.testAttachChildren(result.first, &root, result.second);

    EXPECT_EQ(root.weight, 10);

    EXPECT_EQ(root.getChild(LEFT), &left);
    EXPECT_EQ(root.getChild(RIGHT), &right);

    EXPECT_EQ(left.reversed, true);
    EXPECT_EQ(right.reversed, false);

    EXPECT_EQ(root.trackData.leftMost, &left);
    EXPECT_EQ(root.trackData.rightMost, &right);

    EXPECT_EQ(root.trackData.netCost, 5);
    EXPECT_EQ(left.trackData.netCost, -2);
    EXPECT_EQ(right.trackData.netCost, 1);

    EXPECT_EQ(root.trackData.netMin, 3);
    EXPECT_EQ(left.trackData.netMin, 0);
    EXPECT_EQ(right.trackData.netMin, 3);

    EXPECT_EQ(root.getNetDirMin(LEFT), -1);
    EXPECT_EQ(left.getNetDirMin(LEFT), 0);
    EXPECT_EQ(right.getNetDirMin(LEFT), 3);

    EXPECT_EQ(root.getNetDirMin(RIGHT), -4);
    EXPECT_EQ(left.getNetDirMin(RIGHT), 7);
    EXPECT_EQ(right.getNetDirMin(RIGHT), 9);
}

TEST(BBTNodes, rotateLeftTest){
    BBTNode root, leftKid, rightKid, rlGrandkid, rrGrandkid;
    BiasedBinaryForest forest;

    root.setChild(LEFT, &leftKid);
    root.setChild(RIGHT, &rightKid);
    leftKid.parent = &root;
    rightKid.parent = &root;
    rightKid.setChild(LEFT, &rlGrandkid);
    rightKid.setChild(RIGHT, &rrGrandkid);
    rlGrandkid.parent = &rightKid;
    rrGrandkid.parent = &rightKid;

    //Setting some tracking data to nonboring values
    root.weight = 15;
    leftKid.weight = 3;
    rightKid.weight = 12;
    rlGrandkid.weight = 5;
    rrGrandkid.weight = 7;

    root.trackData.leftMost = &leftKid;
    root.trackData.rightMost = &rrGrandkid;
    leftKid.trackData.leftMost = &leftKid;
    leftKid.trackData.rightMost = &leftKid;
    rightKid.trackData.leftMost = &rlGrandkid;
    rightKid.trackData.rightMost = &rrGrandkid;
    rlGrandkid.trackData.leftMost = &rlGrandkid;
    rlGrandkid.trackData.rightMost = &rlGrandkid;
    rrGrandkid.trackData.leftMost = &rrGrandkid;
    rrGrandkid.trackData.rightMost = &rrGrandkid;

    root.trackData.netCost = 17;
    leftKid.trackData.netCost = -5;
    rightKid.trackData.netCost = -3;
    rlGrandkid.trackData.netCost = -6;
    rrGrandkid.trackData.netCost = -11;

    root.trackData.netMin = 3;
    leftKid.trackData.netMin = 9;
    rightKid.trackData.netMin = 0;
    rlGrandkid.trackData.netMin = 5;
    rrGrandkid.trackData.netMin = 0;

    root.trackData.netLeftMin = -2;
    leftKid.trackData.netLeftMin = 6;
    rightKid.trackData.netLeftMin = 3;
    rlGrandkid.trackData.netLeftMin = 6;
    rrGrandkid.trackData.netLeftMin = 7;

    root.trackData.netRightMin = -9;
    leftKid.trackData.netRightMin = 18;
    rightKid.trackData.netRightMin = 8;
    rlGrandkid.trackData.netRightMin = 13;
    rrGrandkid.trackData.netRightMin = 14;

    BBTNode* newRoot = forest.testRotate(&root, LEFT);

    EXPECT_EQ(rightKid.weight, 15);
    EXPECT_EQ(root.weight, 8);

    EXPECT_EQ(rightKid.getChild(LEFT), &root);
    EXPECT_EQ(rightKid.getChild(RIGHT), &rrGrandkid);
    EXPECT_EQ(root.getChild(LEFT), &leftKid);
    EXPECT_EQ(root.getChild(RIGHT), &rlGrandkid);

    EXPECT_EQ(rightKid.trackData.leftMost, &leftKid);
    EXPECT_EQ(rightKid.trackData.rightMost, &rrGrandkid);
    EXPECT_EQ(root.trackData.leftMost, &leftKid);
    EXPECT_EQ(root.trackData.rightMost, &rlGrandkid);
}