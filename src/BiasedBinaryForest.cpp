#ifndef BIASEDBINARYTREES_BIASEDBINARYFOREST_CPP
#define BIASEDBINARYTREES_BIASEDBINARYFOREST_CPP

#include <cassert>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <deque>

#include "BiasedBinaryForest.hpp"

BBTNode* BiasedBinaryForest::getBBTRoot(BBTNode *node) {
    assert(node != nullptr);
    while(node->parent != nullptr) node = node->parent;
    return node;
}

node BiasedBinaryForest::getStart(BBTNode *root) {
    assert(root->parent == nullptr);
    root->reversalState = root->reversed;
    return root->getDirMost(LEFT)->v;
}

node BiasedBinaryForest::getEnd(BBTNode *root) {
    assert(root->parent == nullptr);
    root->reversalState = root->reversed;
    return root->getDirMost(RIGHT)->v;
}

std::pair<node, cost> BiasedBinaryForest::getBefore(BBTNode *leaf) {
    assert(leaf->isLeaf == true);
    std::pair<BBTNode*, cost> before = getNeighbouringLeaf(leaf, LEFT);
    if(before.first == nullptr) return {none, 0};
    return {before.first->v, before.second};
}

std::pair<node, cost> BiasedBinaryForest::getAfter(BBTNode *leaf) {
    assert(leaf->isLeaf == true);
    std::pair<BBTNode*, cost> after = getNeighbouringLeaf(leaf, RIGHT);
    if(after.first == nullptr) return {none, 0};
    return {after.first->v, after.second};
}

void BiasedBinaryForest::update(BBTNode *root, diff x) {
    assert(root->parent == nullptr);
    if(root->isLeaf == true) return;
    root->trackData.netCost += x;
    root->trackData.netMin += x;
    root->trackData.netMax += x;
}

void BiasedBinaryForest::reverse(BBTNode *root) {
    root->reversed = !root->reversed;
}

BiasedBinaryForest::SplitResult BiasedBinaryForest::globalSplit(BBTNode *splitNode){
    assert(splitNode->isLeaf);
    if(splitNode->parent == nullptr) return {nullptr, nullptr, 0, 0};

    BBTNode *leftTree = nullptr, *rightTree = nullptr;
    cost lCost = 0, rCost = 0;

    //leftWeight is the cummulative weight of everything right of the current node IF
    count leftWeight = 0, rightWeight = 0;
    BBTNode::trackingData totalValuesHere;
    bool reversalState = false;

    //calc direct values at splitNode
    BBTNode* v = splitNode->parent;
    getTotalValues(splitNode, v, leftWeight, rightWeight, totalValuesHere, reversalState);

    //llWeight is the weight of everything left when the node we are looking at is left of splitNode
    //lrWeight is ... of everything right when the node we are looking at is left of splitNode
    //rlWeight and rrWeight are the same for when we are right of splitNode
    count llWeight = leftWeight, lrWeight = rightWeight + splitNode->weight;
    count rlWeight = leftWeight + splitNode->weight, rrWeight = rightWeight;

    v = splitNode->parent;
    BBTNode* parent, *joinTree;
    bool left;
    cost edgeCost;

    //We cut off splitNode by hand
    splitNode->parent = nullptr;
    v->reversalState = reversalState;
    if(v->getChild(LEFT) == splitNode) v->setChild(LEFT, nullptr);
    else v->setChild(RIGHT, nullptr);

    while (v != nullptr){
        //Step 1: cut off v from its parent
        parent = v->parent;
        if(parent != nullptr){
            parent->reversalState = reversalState != parent->reversed;
            if(parent->getChild(LEFT) == v) parent->setChild(LEFT, nullptr);
            else parent->setChild(RIGHT, nullptr);
            v->parent = nullptr;
        }

        //Set up reversal and check directions at v
        v->reversalState = reversalState;
        left = v->getChild(LEFT) != nullptr;

        //separate the join tree from its parent
        joinTree = v->getChild(left);
        v->setChild(left, nullptr);
        joinTree->parent = nullptr;

        //the joinTrees now no longer counts towards its side
        if(left){
            llWeight -= joinTree->weight;
        }
        else {
            rrWeight -= joinTree->weight;
        }

        //Set the join tree to the proper total values
        joinTree->reversed = (reversalState != joinTree->reversed);
        joinTree->reversalState = joinTree->reversed;
        joinTree->trackData.netMax += totalValuesHere.netMax;
        joinTree->trackData.netMin += totalValuesHere.netMin;
        joinTree->trackData.netCost += totalValuesHere.netCost;
        if(left){
            joinTree->setNetDirMin(LEFT, joinTree->getNetDirMin(LEFT) + totalValuesHere.netLeftMin - llWeight);
            joinTree->setNetDirMin(RIGHT, joinTree->getNetDirMin(RIGHT) + totalValuesHere.netRightMin - lrWeight);
        }
        else {
            joinTree->setNetDirMin(LEFT, joinTree->getNetDirMin(LEFT) + totalValuesHere.netLeftMin - rlWeight);
            joinTree->setNetDirMin(RIGHT, joinTree->getNetDirMin(RIGHT) + totalValuesHere.netRightMin - rrWeight);
        }

        //After now every future join tree left has the current join tree to its right and vice versa
        if(left){
            lrWeight += joinTree->weight;
        }
        else {
            rlWeight += joinTree->weight;
        }

        //get the correct total values for the parent of v
        edgeCost = totalValuesHere.netCost;
        totalValuesHere.netMax -= v->trackData.netMax;
        totalValuesHere.netMin -= v->trackData.netMin;
        totalValuesHere.netCost -= v->trackData.netCost;
        totalValuesHere.netLeftMin -= v->getNetDirMin(LEFT);
        totalValuesHere.netRightMin -= v->getNetDirMin(RIGHT);
        reversalState = reversalState != v->reversed;

        v->reversed = false;
        v->reversalState = false;

        //Now the joinTree and v have been separated clean and we can join them in
        if(left){
            if(leftTree == nullptr){
                lCost = edgeCost;
                delete v;
                leftTree = joinTree;
            }
            else {
                v->trackData.netCost = edgeCost;
                leftTree = localJoin(joinTree, leftTree, v);
            }

            checkTreeValidity(leftTree);
        }
        else{
            if(rightTree == nullptr){
                rCost = edgeCost;
                delete v;
                rightTree = joinTree;
            }
            else {
                v->trackData.netCost = edgeCost;
                rightTree = localJoin(rightTree, joinTree, v);
            }

            checkTreeValidity(rightTree);
        }

        v = parent;
    }

    if(leftTree != nullptr) checkTreeValidity(leftTree);
    if(rightTree != nullptr) checkTreeValidity(rightTree);

    return {leftTree, rightTree, lCost, rCost};
}

void BiasedBinaryForest::getTotalValues(BBTNode *splitNode, BBTNode *v, count &leftWeight, count &rightWeight,
                                        BBTNode::trackingData &totalValuesHere, bool &reversalState) const {
    BBTNode* prev = splitNode;
    while (v != nullptr){
        v->reversalState = reversalState;

        //TODO maybe define + and - for trackingData
        totalValuesHere.netMax += v->trackData.netMax;
        totalValuesHere.netMin += v->trackData.netMin;
        totalValuesHere.netCost += v->trackData.netCost;
        totalValuesHere.netLeftMin += v->getNetDirMin(LEFT);
        totalValuesHere.netRightMin += v->getNetDirMin(RIGHT);

        if(v->getChild(LEFT) == prev) rightWeight += v->getChild(RIGHT)->weight;
        else leftWeight += v->getChild(LEFT)->weight;

        reversalState = reversalState != v->reversed;
        prev = v;
        v = v->parent;
    }
    //If reversalState == true then our tracking above has right and left the wrong way around
    if(reversalState == true){
        std::swap(totalValuesHere.netLeftMin, totalValuesHere.netRightMin);
        std::swap(leftWeight, rightWeight);
    }
}

BBTNode* BiasedBinaryForest::globalJoin(BBTNode* t1, BBTNode* t2, cost x) {
    //It is highly recommended to read BIASED SEARCH TREES by Bent, Sleator and Tarjan to understand what is going on here
    assert(t1 != nullptr && t2 != nullptr);
    assert(t1->parent == nullptr && t2->parent == nullptr);

    if((t1->isLeaf && t1->rank >= t2->rank) || (t2->isLeaf && t2->rank >= t1->rank)) return globalJoinCase1(t1, t2, x);
    else if(t1->rank > t2->rank || t2->rank > t1->rank) return globalJoinCase23(t1, t2, x);
    else return globalJoinCase4(t1, t2, x);
}

BBTNode* BiasedBinaryForest::globalJoinCase1(BBTNode *t1, BBTNode *t2, cost x) {
    assert(t1 != nullptr && t2 != nullptr);
    assert(t1->parent == nullptr && t2->parent == nullptr);
    assert((t1->isLeaf && t1->rank >= t2->rank) || (t2->isLeaf && t2->rank >= t1->rank));
    //We have reached the place where the new edge can truly be inserted
    BBTNode* newConn = new BBTNode();

    newConn->trackData.netCost = x;
    newConn->trackData.netMin = x;
    newConn->trackData.netMax = x;
    attachChildren(t1, newConn, t2);

    newConn->rank = std::max(t1->rank, t2->rank) + 1;

    checkTreeValidity(newConn);

    return newConn;
}

BBTNode* BiasedBinaryForest::globalJoinCase23(BBTNode *t1, BBTNode *t2, cost x){
    assert(t1 != nullptr && t2 != nullptr);
    assert(t1->parent == nullptr && t2->parent == nullptr);

    if(t1->rank > t2->rank){
        assert(t1->isLeaf == false);

        t1 = tilt(t1, LEFT);

        std::pair<BBTNode*, BBTNode*> kids = separateChildren(t1);
        BBTNode* z = globalJoin(kids.second, t2, x);
        attachChildren(kids.first, t1, z);

        checkTreeValidity(t1);

        return t1;
    }
    else {
        assert(t2->isLeaf == false);

        t2 = tilt(t2, RIGHT);
        std::pair<BBTNode*, BBTNode*> kids = separateChildren(t2);
        BBTNode* z = globalJoin(t1, kids.first, x);
        attachChildren(z, t2, kids.second);

        checkTreeValidity(t2);

        return t2;
    }
}

BBTNode* BiasedBinaryForest::globalJoinCase4(BBTNode *t1, BBTNode *t2, cost x){
    assert(t1->rank == t2->rank);
    assert((t1->isLeaf == false) && (t2->isLeaf == false));

    BBTNode *u, *v;

    std::pair<BBTNode*, BBTNode*> t1kids = separateChildren(t1);
    std::pair<BBTNode*, BBTNode*> t2kids = separateChildren(t2);
    std::pair<BBTNode*, BBTNode*> ukids, vkids;
    //u is supposed to be the rightmost node in t1 with rank == t1->rank, v symmetric
    if(t1kids.second->rank == t1->rank){
        u = t1kids.second;
        ukids = separateChildren(u);
    }
    else {
        u = t1;
        ukids = t1kids;
    }
    if(t2kids.first->rank == t2->rank){
        v = t2kids.first;
        vkids = separateChildren(v);
    }
    else {
        v = t2;
        vkids = t2kids;
    }

    BBTNode* z = globalJoin(ukids.second, vkids.first, x);

    if(z->rank == t1->rank){
        //Case 4 a)
        return globalJoinCase4a(t1, t2, u, v, t1kids, t2kids, ukids, vkids, z);
    }
    else {
        if(u != t1){
            return globalJoinCase4bi(t1, t2, u, v, t1kids, t2kids, ukids, vkids, z);
        }
        else if(v != t2){
            return globalJoinCase4bii(t1, t2, u, v, t1kids, t2kids, ukids, vkids, z);
        }
        else if(t1kids.first->rank == t1->rank){
            return globalJoinCase4biii(t1, t2, u, v, t1kids, t2kids, ukids, vkids, z);
        }
        else if(t2kids.second->rank == t2->rank){
            return globalJoinCase4biv(t1, t2, u, v, t1kids, t2kids, ukids, vkids, z);
        }
        else {
            return globalJoinCase4bv(t1, t2, u, v, t1kids, t2kids, ukids, vkids, z);
        }
    }
}

BBTNode *BiasedBinaryForest::globalJoinCase4a(BBTNode *t1, BBTNode *t2, BBTNode *u, BBTNode *v,
                                              std::pair<BBTNode *, BBTNode *> &t1kids,
                                              std::pair<BBTNode *, BBTNode *> &t2kids, std::pair<BBTNode *, BBTNode *> &ukids,
                                              std::pair<BBTNode *, BBTNode *> &vkids, BBTNode *z) {
    std::pair<BBTNode*, BBTNode*> zkids = separateChildren(z);
    attachChildren(ukids.first, u, zkids.first);
    attachChildren(zkids.second, v, vkids.second);
    if(u != t1) attachChildren(t1kids.first, t1, u);
    if(v != t2) attachChildren(v, t2, t2kids.second);
    attachChildren(t1, z, t2);

    z->rank = t1->rank + 1;

    checkTreeValidity(z);

    return z;
}

BBTNode *BiasedBinaryForest::globalJoinCase4bi(BBTNode *t1, BBTNode *t2, BBTNode *u, BBTNode *v,
                                               std::pair<BBTNode *, BBTNode *> &t1kids,
                                               std::pair<BBTNode *, BBTNode *> &t2kids,
                                               std::pair<BBTNode *, BBTNode *> &ukids, std::pair<BBTNode *, BBTNode *> &vkids,
                                               BBTNode *z) {
    attachChildren(t1kids.first, t1, ukids.first);
    attachChildren(z, v, vkids.second);
    if(v != t2) attachChildren(v, t2, t2kids.second);
    attachChildren(t1, u, t2);

    u->rank = t1->rank + 1;

    checkTreeValidity(u);

    return u;
}

BBTNode *BiasedBinaryForest::globalJoinCase4bii(BBTNode *t1, BBTNode *t2, BBTNode *u, BBTNode *v,
                                                std::pair<BBTNode *, BBTNode *> &t1kids,
                                                std::pair<BBTNode *, BBTNode *> &t2kids,
                                                std::pair<BBTNode *, BBTNode *> &ukids, std::pair<BBTNode *, BBTNode *> &vkids,
                                                BBTNode *z) {
    attachChildren(vkids.second, t2, t2kids.second);
    attachChildren(ukids.first, u, z);
    if(u != t1) attachChildren(t1kids.first, t1, u);
    attachChildren(t1, v, t2);

    v->rank = t2->rank + 1;

    checkTreeValidity(v);

    return v;
}

BBTNode *BiasedBinaryForest::globalJoinCase4biii(BBTNode *t1, BBTNode *t2, BBTNode *u, BBTNode *v,
                                                 std::pair<BBTNode *, BBTNode *> &t1kids,
                                                 std::pair<BBTNode *, BBTNode *> &t2kids,
                                                 std::pair<BBTNode *, BBTNode *> &ukids, std::pair<BBTNode *, BBTNode *> &vkids,
                                                 BBTNode *z) {
    attachChildren(z, t2, t2kids.second);
    attachChildren(t1kids.first, t1, t2);

    t1->rank++;

    checkTreeValidity(t1);

    return t1;
}

BBTNode *BiasedBinaryForest::globalJoinCase4biv(BBTNode *t1, BBTNode *t2, BBTNode *u, BBTNode *v,
                                                std::pair<BBTNode *, BBTNode *> &t1kids,
                                                std::pair<BBTNode *, BBTNode *> &t2kids,
                                                std::pair<BBTNode *, BBTNode *> &ukids, std::pair<BBTNode *, BBTNode *> &vkids,
                                                BBTNode *z) {
    attachChildren(t1kids.first, t1, z);
    attachChildren(t1, t2, t2kids.second);

    t2->rank++;

    checkTreeValidity(t2);

    return t2;
}

BBTNode *BiasedBinaryForest::globalJoinCase4bv(BBTNode *t1, BBTNode *t2, BBTNode *u, BBTNode *v,
                                               std::pair<BBTNode *, BBTNode *> &t1kids,
                                               std::pair<BBTNode *, BBTNode *> &t2kids,
                                               std::pair<BBTNode *, BBTNode *> &ukids, std::pair<BBTNode *, BBTNode *> &vkids,
                                               BBTNode *z) {
    attachChildren(z, t2, t2kids.second);
    attachChildren(t1kids.first, t1, t2);

    checkTreeValidity(t1);

    return t1;
}

std::pair<std::pair<node, node>, cost> BiasedBinaryForest::getMinEdgeOnPath(BBTNode *root) {
    assert(root->parent == nullptr);
    if(root->isLeaf) return {{none, none},0};

    cost min = root->trackData.netMin;
    cost edgeCost = 0;
    bool reversalState = false;

    while(root->isLeaf == false){
        reversalState = reversalState != root->reversed;
        root->reversalState = reversalState;
        edgeCost += root->trackData.netCost;

        if(root->getChild(RIGHT)->isLeaf == false && root->getChild(RIGHT)->trackData.netMin == 0){
            //There is a minimum edge further right, lets move there
            root = root->getChild(RIGHT);
        }
        else if (edgeCost == min) {
            root->getChild(LEFT)->reversalState = root->getChild(LEFT)->reversed != reversalState;
            root->getChild(RIGHT)->reversalState = root->getChild(RIGHT)->reversed != reversalState;
            return {{root->getChild(LEFT)->getDirMost(RIGHT)->v, root->getChild(RIGHT)->getDirMost(LEFT)->v}, min};
        }
        else {
            root = root->getChild(LEFT);
        }
    }

    throw std::runtime_error("Major tracking issue in getMinEdgeOnPath occured");
    return {{none, none},0};
}

std::pair<std::pair<node, node>, cost> BiasedBinaryForest::getMaxEdgeOnPath(BBTNode *root) {
    assert(root->parent == nullptr);
    if(root->isLeaf) return {{none, none},0};

    cost max = root->trackData.netMax;
    cost edgeCost = 0;
    bool reversalState = false;

    while(root->isLeaf == false){
        reversalState = reversalState != root->reversed;
        root->reversalState = reversalState;
        edgeCost += root->trackData.netCost;

        if(root->getChild(RIGHT)->isLeaf == false && root->getChild(RIGHT)->trackData.netMax == 0){
            //There is a minimum edge further right, lets move there
            root = root->getChild(RIGHT);
        }
        else if (edgeCost == max) {
            root->getChild(LEFT)->reversalState = root->getChild(LEFT)->reversed != reversalState;
            root->getChild(RIGHT)->reversalState = root->getChild(RIGHT)->reversed != reversalState;
            return {{root->getChild(LEFT)->getDirMost(RIGHT)->v, root->getChild(RIGHT)->getDirMost(LEFT)->v}, max};
        }
        else {
            root = root->getChild(LEFT);
        }
    }

    throw std::runtime_error("Major tracking issue in getMaxEdgeOnPath occured");
    return {{none, none},0};
}

std::pair<std::pair<node, node>, diff> BiasedBinaryForest::getTiltedEdgeOnPath(BBTNode *root) {
    assert(root->parent == nullptr);
    if(root->isLeaf) return {{none, none},0};

    bool reversalState = root->reversed;
    root->reversalState = reversalState;

    if(root->getNetDirMin(LEFT) > 0) return {{none, none},0};

    diff netLeftMin = root->getNetDirMin(LEFT), leftTilt;
    count leftWeight = 0;
    //Descend downwards always looking for the rightmost way to get a node with leftTilt <= 0
    while(root->isLeaf == false){
        //Set the reversal state of the kids
        if(root->getChild(LEFT)->isLeaf == false) root->getChild(LEFT)->reversalState = reversalState != root->getChild(LEFT)->reversed;
        if(root->getChild(RIGHT)->isLeaf == false) root->getChild(RIGHT)->reversalState = reversalState != root->getChild(RIGHT)->reversed;

        leftTilt = root->getDirTilt(LEFT) + leftWeight;
        if((root->getChild(RIGHT)->isLeaf == false) && (netLeftMin + root->getChild(RIGHT)->getNetDirMin(LEFT) <= 0)){
            //There is a strongly tilted edge further right
            leftWeight += root->getChild(LEFT)->weight;
            root = root->getChild(RIGHT);
            reversalState = reversalState != root->reversed;
            netLeftMin += root->getNetDirMin(LEFT);
        }
        else if (leftTilt <= 0){
            //The root is strongly tilted and there are no strongly tilted edges further right
            return {{root->getChild(LEFT)->getDirMost(RIGHT)->v, root->getChild(RIGHT)->getDirMost(LEFT)->v}, leftTilt};
        }
        else {
            root = root->getChild(LEFT);
            reversalState = reversalState != root->reversed;
            netLeftMin += root->getNetDirMin(LEFT);
        }
    }

    throw std::runtime_error("Major tracking issue in getTiltedEdgeOnPath occured");
    return {{none, none},0};
}

void BiasedBinaryForest::deleteTree(BBTNode *node) {
    std::deque<BBTNode*> killList(1, getBBTRoot(node));
    while (!killList.empty()){
        node = killList.back();
        killList.pop_back();
        if(node->leftChild != nullptr) killList.push_back(node->leftChild);
        if(node->rightChild != nullptr) killList.push_back(node->rightChild);
        delete node;
    }
}

BBTNode* BiasedBinaryForest::localJoin(BBTNode *t1, BBTNode *t2, BBTNode *conn) {
    //It is highly recommended to read BIASED SEARCH TREES by Bent, Sleator and Tarjan to understand what is going on here
    assert(t1 != nullptr && t2 != nullptr);
    assert(t1->parent == nullptr && t2->parent == nullptr);

    if((t1->rank == t2->rank) || (t1->rank >= t2->rank && t1->isLeaf) || (t2->rank >= t1->rank && t2->isLeaf)){
        //Case 1 of local join
        attachChildren(t1,conn,t2);
        conn->rank = std::max(t1->rank, t2->rank) + 1;
        return conn;
    }
    else if(t1->rank > t2->rank){
        //Case 2 of local join
        t1 = tilt(t1, LEFT);
        std::pair<BBTNode*, BBTNode*> t1kids = separateChildren(t1);
        BBTNode* z = localJoin(t1kids.second, t2, conn);
        attachChildren(t1kids.first, t1, z);
        return t1;
    }
    else {
        //Case 3 of local join
        t2 = tilt(t2, RIGHT);
        std::pair<BBTNode*, BBTNode*> t2kids = separateChildren(t2);
        BBTNode* z = localJoin(t1, t2kids.first, conn);
        attachChildren(z, t2, t2kids.second);
        return t2;
    }
}

BBTNode* BiasedBinaryForest::tilt(BBTNode *node, bool left) {
    assert(node != nullptr);
    assert(node ->isLeaf == false);

    setReversalStates(node);

    bool right = !left;

    //Check for case where nothing needs to be done
    if(node->rank != node->getChild(right)->rank) return node;
    //Check for the case that node can just be promoted
    if(node->rank == node->getChild(left)->rank){
        node->rank++;
        return node;
    }
    //Otherwise move it around so the heavy kid is moved to the right(left)
    return rotate(node, left);
}

BBTNode* BiasedBinaryForest::rotate(BBTNode *node, bool left) {
    assert(node != nullptr);
    assert(node->isLeaf == false);
    assert(node->parent == nullptr);

    setReversalStates(node);

    assert(node->getChild(!left)->isLeaf == false);

    if(left){
        std::pair<BBTNode*, BBTNode*> kids = separateChildren(node);
        std::pair<BBTNode*, BBTNode*> grandKids = separateChildren(kids.second);
        attachChildren(kids.first, node, grandKids.first);
        attachChildren(node, kids.second, grandKids.second);

        return kids.second;
    }
    else {
        std::pair<BBTNode*, BBTNode*> kids = separateChildren(node);
        std::pair<BBTNode*, BBTNode*> grandKids = separateChildren(kids.first);
        attachChildren(grandKids.second, node, kids.second);
        attachChildren(grandKids.first, kids.first, node);

        return kids.first;
    }
}

void BiasedBinaryForest::attachChildren(BBTNode *leftChild, BBTNode *v, BBTNode *rightChild) {
    assert(v->parent == nullptr);
    assert(leftChild->parent == nullptr);
    assert(rightChild->parent == nullptr);

    assert(leftChild != rightChild);
    assert(leftChild != v);
    assert(v != rightChild);

    assert(v->isLeaf == false);

    //Set all pointers correctly
    v->setChild(LEFT, leftChild);
    v->setChild(RIGHT, rightChild);
    leftChild->parent = v;
    rightChild->parent = v;

    //Adjust weight etc properly for v (does not cover rank)
    v->adjustSubtreeTracking();

    //Calculate all the correct values for netMin, netMax, netLeftMin and netRightMin
    //Shift the tilts to account for new left/right neighbours
    if(leftChild->isLeaf == false) leftChild->setNetDirMin(RIGHT, leftChild->getNetDirMin(RIGHT) + rightChild->weight);
    if(rightChild->isLeaf == false) rightChild->setNetDirMin(LEFT, rightChild->getNetDirMin(LEFT) + leftChild->weight);
    //Now get the minimum tilts of the subtrees
    v->setNetDirMin(LEFT,v->getDirTilt(LEFT));
    if(leftChild->isLeaf == false) v->setNetDirMin(LEFT, std::min(leftChild->getNetDirMin(LEFT), v->getNetDirMin(LEFT)));
    if(rightChild->isLeaf == false) v->setNetDirMin(LEFT, std::min(rightChild->getNetDirMin(LEFT), v->getNetDirMin(LEFT)));

    v->setNetDirMin(RIGHT,v->getDirTilt(RIGHT));
    if(leftChild->isLeaf == false) v->setNetDirMin(RIGHT, std::min(leftChild->getNetDirMin(RIGHT), v->getNetDirMin(RIGHT)));
    if(rightChild->isLeaf == false) v->setNetDirMin(RIGHT, std::min(rightChild->getNetDirMin(RIGHT), v->getNetDirMin(RIGHT)));

    //The implicitly tracked values of the kids are currently total. Set them to differences instead
    v->adjustImplicitTracking();
}

std::pair<BBTNode*, BBTNode*> BiasedBinaryForest::separateChildren(BBTNode *v) {
    //Ensure everyone has the correct reversal state
    setReversalStates(v);
    //Grab the kids and disconnect them from their parent
    BBTNode *leftTree = v->getChild(LEFT), *rightTree = v->getChild(RIGHT);
    v->setChild(LEFT, nullptr);
    leftTree->parent = nullptr;
    v->setChild(RIGHT, nullptr);
    rightTree->parent = nullptr;

    //Now set all the tracking (leftMost and rightMost function automatically)
    if(leftTree->isLeaf == false) {
        leftTree->reversed = leftTree->reversalState;
        leftTree->trackData.netCost += v->trackData.netCost;
        leftTree->trackData.netMin += v->trackData.netMin;
        leftTree->trackData.netMax += v->trackData.netMax;
        leftTree->setNetDirMin(LEFT, leftTree->getNetDirMin(LEFT) + v->getNetDirMin(LEFT));
        leftTree->setNetDirMin(RIGHT, leftTree->getNetDirMin(RIGHT) + v->getNetDirMin(RIGHT) - rightTree->weight);
    }
    if(rightTree->isLeaf == false) {
        rightTree->reversed = rightTree->reversalState;
        rightTree->trackData.netCost += v->trackData.netCost;
        rightTree->trackData.netMin += v->trackData.netMin;
        rightTree->trackData.netMax += v->trackData.netMax;
        rightTree->setNetDirMin(LEFT, rightTree->getNetDirMin(LEFT) + v->getNetDirMin(LEFT) - leftTree->weight);
        rightTree->setNetDirMin(RIGHT, rightTree->getNetDirMin(RIGHT) + v->getNetDirMin(RIGHT));
    }

    //Now we clean up everything with v
    v->reversed = false;
    v->reversalState = false;
    v->weight = 0;
    //v->rank = 0;
    v->trackData.leftMost = nullptr;
    v->trackData.rightMost = nullptr;
    v->trackData.netMin = v->trackData.netCost;
    v->trackData.netMax = v->trackData.netCost;
    v->trackData.netLeftMin = 0;
    v->trackData.netRightMin = 0;

    return {leftTree, rightTree};
}

void BiasedBinaryForest::setReversalStates(BBTNode *v) {
    assert(v->isLeaf == false);

    bool rev = v->reversed;
    v->reversalState = rev;
    //Set children reversal states to the xor on their path
    v->getChild(LEFT)->reversalState = rev != v->getChild(LEFT)->reversed;
    v->getChild(RIGHT)->reversalState = rev != v->getChild(RIGHT)->reversed;
}

BBTNode *BBTNode::getChild(bool left) {
    if(reversalState == left) return rightChild;
    else return leftChild;
}

void BBTNode::setChild(bool left, BBTNode *newChild) {
    if(reversalState == left) rightChild = newChild;
    else leftChild = newChild;
}

bool BBTNode::isLeftChild() {
    if(parent == nullptr) return true;
    else if (parent->getChild(LEFT) == this) return true;
    else return false;
}

BBTNode* BBTNode::getDirMost(bool left) {
    if(reversalState != left) return trackData.leftMost;
    else return trackData.rightMost;
}

diff BBTNode::getNetDirMin(bool left) {
    if(reversalState == left) return trackData.netRightMin;
    else return trackData.netLeftMin;
}

void BiasedBinaryForest::BBTNode::setNetDirMin(bool left, diff val) {
    if(reversalState == left) trackData.netRightMin = val;
    else trackData.netLeftMin = val;
}

diff BBTNode::getDirTilt(bool left) {
    assert(isLeaf == false);
    return getChild(left)->weight - getChild(!left)->getDirMost(left)->weight;
}

void BBTNode::adjustSubtreeTracking() {
    assert(!isLeaf);

    weight = getChild(LEFT)->weight + getChild(RIGHT)->weight;
    trackData.leftMost = getChild(LEFT)->getDirMost(LEFT);
    trackData.rightMost = getChild(RIGHT)->getDirMost(RIGHT);
    trackData.netMin = trackData.netCost;
    trackData.netMax = trackData.netCost;
    if(getChild(LEFT)->isLeaf == false){
        trackData.netMin = std::min(trackData.netMin, getChild(LEFT)->trackData.netMin);
        trackData.netMax = std::max(trackData.netMax, getChild(LEFT)->trackData.netMax);
    }
    if(getChild(RIGHT)->isLeaf == false){
        trackData.netMin = std::min(trackData.netMin, getChild(RIGHT)->trackData.netMin);
        trackData.netMax = std::max(trackData.netMax, getChild(RIGHT)->trackData.netMax);
    }
}

void BBTNode::adjustImplicitTracking() {
    assert(!isLeaf);

    setToDiff(leftChild);
    setToDiff(rightChild);
}

void BBTNode::setToDiff(BBTNode* child) {
    if(child->isLeaf == false){
        child->reversed = child->reversed != reversed;
        child->trackData.netCost -= trackData.netCost;
        child->trackData.netMin -= trackData.netMin;
        child->trackData.netMax -= trackData.netMax;
        child->setNetDirMin(LEFT, child->getNetDirMin(LEFT) - getNetDirMin(LEFT));
        child->setNetDirMin(RIGHT, child->getNetDirMin(RIGHT) - getNetDirMin(RIGHT));
    }
}

void BBTNode::setWeight(count w) {
    assert(isLeaf == true);
    assert(parent == nullptr);

    weight = w;
    rank = std::floor(std::log2(w));
}

void BiasedBinaryForest::checkTreeValidity(BBTNode *root) {
#ifndef NDEBUG // the following code is only executed in debug mode
    bool fine = true;

    if(root->isLeaf == false){
        assert(root->weight == root->leftChild->weight + root->rightChild->weight);
        assert(root->rank <= std::floor(std::log2(root->weight)) + 1);

        assert(root->leftChild != nullptr && root->rightChild != nullptr);
        assert(root->leftChild->parent == root);
        assert(root->rightChild->parent == root);

        checkTreeValidity(root->leftChild);
        checkTreeValidity(root->rightChild);
    }
    else {
        assert(root->rank == std::floor(std::log2(root->weight)));
        if(root->parent != nullptr) assert(root->rank < root->parent->rank);

        assert(root->leftChild == nullptr && root->rightChild == nullptr);
    }

    if(root->parent != nullptr){
        assert(root->rank <= root->parent->rank);
        if(root->parent->parent != nullptr) assert(root->rank < root->parent->parent->rank);
        //Check for neighbouring leaves condition
        if(root->rank + 2 <= root->parent->rank){
            //minor node, check for neighbouring leaves
            BBTNode* leftNL = getNeighbouringLeaf(root, LEFT).first;
            BBTNode* rightNL = getNeighbouringLeaf(root, RIGHT).first;

            if(leftNL != nullptr) assert(leftNL->rank >= root->parent->rank - 1);
            if(rightNL != nullptr) assert(rightNL->rank >= root->parent->rank - 1);
        }
    }
#endif //NDEBUG
}

std::pair<BBTNode*, cost> BiasedBinaryForest::getNeighbouringLeaf(BBTNode *node, bool left) {
    bool right = !left;

    assert(node != nullptr);

    if(node->parent == nullptr) return {nullptr, 0};

    bool reversalState = false;
    diff trueCost = 0;

    BBTNode* move = node->parent, * prev = node;
    //Calc the reversal state of node
    while (move != nullptr){
        reversalState = (reversalState != move->reversed);
        trueCost += move->trackData.netCost;
        move = move->parent;
    }

    //Now move up until move is the right(left) child
    move = node->parent;

    while (move != nullptr){
        move->reversalState = reversalState;
        //Check if we finally moved left(right) while going up
        if(move->getChild(right) == prev){
            move->getChild(left)->reversalState = (reversalState != move->getChild(left)->reversed);
            return {move->getChild(left)->getDirMost(right), trueCost};
        }

        reversalState = reversalState != move->reversed;
        trueCost -= move->trackData.netCost;
        prev = move;
        move = move->parent;
    }

    return {nullptr, 0};
}

void BiasedBinaryForest::writePath(BBTNode *root, std::vector<node> *path, bool reversed) {
    if(root->isLeaf) path->push_back(root->v);
    else {
        reversed = reversed != root->reversed;
        root->reversalState = reversed;
        writePath(root->getChild(LEFT), path, reversed);
        writePath(root->getChild(RIGHT), path, reversed);
    }
}

#endif //BIASEDBINARYTREES_BIASEDBINARYFOREST_CPP
