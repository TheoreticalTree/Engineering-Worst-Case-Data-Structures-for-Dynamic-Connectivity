#include <cassert>
#include <deque>
#include <cmath>
#include <utility>

#include "ETForestCutSet.hpp"

constexpr bool LEFT = true;
constexpr bool RIGHT = false;

ETForestCutSet::ETForestCutSet(count pBoostLevel, count n, bool pNoTracking) {
    boostLevel = pBoostLevel;
    if (n != 0) lognsqr = std::ceil(2.0 * std::log2(n)) + 1;
    else lognsqr = 0;
    noTracking = pNoTracking;
}

address ETForestCutSet::getRoot(ETForestCutSet::Node *v) const {
    if (v == nullptr) return nullptr;
    while (v->parent != nullptr) v = v->parent;
    return v;
}

const ETForestCutSet::Node::TrackingData &ETForestCutSet::getTrackingData(ETForestCutSet::Node *edge) {
    return edge->trackingData;
}

void ETForestCutSet::setTrackingData(ETForestCutSet::Node *e, std::vector<std::vector<edge>> *trackingDataRef) {
    e->trackingData.ownData = trackingDataRef;
    refreshTrackingDataUpwards(e);
}

void ETForestCutSet::addEdgeToData(ETForestCutSet::Node *e, edge newEdge, const std::vector<uint8_t> &startingLevels) {
    while (e != nullptr) {
        for (uint8_t i = 0; i < boostLevel; i++) {
            for (uint8_t j = startingLevels[i]; j < lognsqr; j++) {
                e->trackingData.accumulatedData[i][j] ^= newEdge;
            }
        }

        e = e->parent;
    }
}

std::vector<std::pair<node, node>> ETForestCutSet::getTour(ETForestCutSet::Node *e) {
    Node *root = getRoot(e);
    Node *n = root;

    std::deque<Node *> stack;

    std::vector<std::pair<node, node>> ret;

    while ((not stack.empty()) || (n != nullptr)) {
        if (n != nullptr) {
            stack.push_back(n);
            n = n->leftChild;
        } else {
            n = stack.back();
            stack.pop_back();
            ret.emplace_back(n->v, n->w);
            n = n->rightChild;
        }
    }

    return ret;
}

std::pair<address, address> ETForestCutSet::insertETEdge(node v, node w, ETForestCutSet::Node *vEdge, ETForestCutSet::Node *wEdge) {
    if (vEdge != nullptr) assert(getRoot(vEdge) != getRoot(wEdge));

    Node *vTree = nullptr, *wTree = nullptr;
    //Ensure that the Eulertours start and end with v and w respectively
    if (vEdge != nullptr) vTree = makeFront(vEdge);
    if (wEdge != nullptr) wTree = makeFront(wEdge);

    Node *vwEdge = new Node(v, w, {std::vector<std::vector<edge>>(boostLevel, std::vector<edge>(lognsqr)), nullptr});
    Node *wvEdge = new Node(w, v, {std::vector<std::vector<edge>>(boostLevel, std::vector<edge>(lognsqr)), nullptr});

    //Join the Euler tours with the new edges
    vTree = join(vTree, wTree, vwEdge);
    vTree = trivialInsert(wvEdge, vTree, false);

    return {vwEdge, wvEdge};
}

void ETForestCutSet::deleteETEdge(ETForestCutSet::Node *edge, ETForestCutSet::Node *backEdge) {
    assert(edge != nullptr && backEdge != nullptr);

    //Rotate the edge to the front so the backedge is a clean cut
    makeFront(edge);
    std::pair<Node *, Node *> splitTour = split(backEdge);

    //Now we simply have to remove edge and backedge and delete them
    std::pair<Node *, Node *> edgePair = trivialDelete(splitTour.first, true);
    assert(edgePair.first == edge);
    delete edge;
    std::pair<Node *, Node *> backEdgePair = trivialDelete(splitTour.second, true);
    assert(backEdgePair.first == backEdge);
    delete backEdge;
}

address ETForestCutSet::makeFront(ETForestCutSet::Node *newFront) {
    assert(newFront != nullptr);
    std::pair<Node *, Node *> sequenceParts = split(newFront);
    if (sequenceParts.first == nullptr) return sequenceParts.second;

    //We wish to swap around the order of stuff, so we use the first node in the front path as the join node
    std::pair<Node *, Node *> splitFirst = trivialDelete(sequenceParts.first, true);
    //Now we just glue together the two parts the other way around
    Node *newTree = join(sequenceParts.second, splitFirst.second, splitFirst.first);

    checkTreeValidity(newTree);

    return newTree;
}

std::pair<address, address> ETForestCutSet::split(ETForestCutSet::Node *v) {
    assert(v != nullptr);

    //Get the starting trees
    Node *leftTree = separateDirChild(v, true);
    Node *rightTree = separateDirChild(v, false);

    //Trivial case that this was it
    if (v->parent == nullptr) {
        if (rightTree != nullptr) rightTree = trivialInsert(v, rightTree, true);
        else {
            cleanNode(v);
            rightTree = v;
        }

        if (leftTree != nullptr) checkTreeValidity(leftTree);
        checkTreeValidity(rightTree);

        return {leftTree, rightTree};
    }

    //Other Case: Now we move up the tree and join all the left/right subtrees
    Node *pos = v->parent;
    Node *parent;
    Node *joinTree;
    bool left, nextLeft;

    //Cut off v from its parent
    if (pos->leftChild == v) {
        pos->leftChild = nullptr;
        left = false;
    } else {
        pos->rightChild = nullptr;
        left = true;
    }
    rightTree = join(nullptr, rightTree, v);

    while (pos != nullptr) {
        parent = pos->parent;
        //Cut off pos from parent
        if (parent != nullptr) {
            if (parent->leftChild == pos) {
                parent->leftChild = nullptr;
                nextLeft = false;
            } else {
                parent->rightChild = nullptr;
                nextLeft = true;
            }
            pos->parent = nullptr;
        }

        //Get the tree we want to join in
        joinTree = separateDirChild(pos, left);

        if (left) {
            leftTree = join(joinTree, leftTree, pos);
            checkTreeValidity(leftTree);
        } else {
            rightTree = join(rightTree, joinTree, pos);
            checkTreeValidity(rightTree);
        }

        pos = parent;
        left = nextLeft;
    }

    return {leftTree, rightTree};
}

address ETForestCutSet::join(ETForestCutSet::Node *leftTree, ETForestCutSet::Node *rightTree, ETForestCutSet::Node *joinNode) {
    assert(joinNode != nullptr);
    assert(leftTree != joinNode && joinNode != rightTree);
    if (leftTree != nullptr) assert(leftTree != rightTree);
    if (leftTree != nullptr) assert(leftTree->parent == nullptr);
    if (rightTree != nullptr) assert(rightTree->parent == nullptr);
    cleanNode(joinNode);
    if (leftTree == nullptr && rightTree == nullptr) return joinNode;
    if (leftTree == nullptr) return trivialInsert(joinNode, rightTree, true);
    if (rightTree == nullptr) return trivialInsert(joinNode, leftTree, false);

    //Trivial case that both are roughly equal in height
    if (leftTree->height <= rightTree->height + 1 && rightTree->height <= leftTree->height + 1) {
        joinNode->leftChild = leftTree;
        joinNode->rightChild = rightTree;
        leftTree->parent = joinNode;
        rightTree->parent = joinNode;

        refreshSubtreeInfo(joinNode);

        checkTreeValidity(joinNode);

        return joinNode;
    }

    Node *pos;

    //Case of a left join
    if (leftTree->height > rightTree->height) {
        pos = leftTree;
        while (getHeight(pos->rightChild) > rightTree->height) pos = pos->rightChild;
        //Now the right child of pos is equal or 1 smaller then right tree
        Node *smallTree = separateDirChild(pos, false);

        joinNode->leftChild = smallTree;
        if (smallTree != nullptr) smallTree->parent = joinNode;
        joinNode->rightChild = rightTree;
        rightTree->parent = joinNode;
        refreshSubtreeInfo(joinNode);
        pos->rightChild = joinNode;
        joinNode->parent = pos;
        pos = rebalance(pos);
    } else {
        pos = rightTree;
        while (getHeight(pos->leftChild) > leftTree->height) pos = pos->leftChild;
        //Now the right child of pos is equal or 1 smaller then right tree
        Node *smallTree = separateDirChild(pos, true);

        joinNode->rightChild = smallTree;
        if (smallTree != nullptr) smallTree->parent = joinNode;
        joinNode->leftChild = leftTree;
        leftTree->parent = joinNode;
        refreshSubtreeInfo(joinNode);
        pos->leftChild = joinNode;
        joinNode->parent = pos;
        pos = rebalance(pos);
    }

    checkTreeValidity(pos);

    return pos;
}

address ETForestCutSet::trivialInsert(Node *newNode, ETForestCutSet::Node *root, bool first) {
    assert(root != nullptr);
    assert(newNode != nullptr);
    cleanNode(newNode);
    //Step 1: Move down until we find the place where the new pair belongs
    Node *pos = root;

    if (first) {
        while (pos->leftChild != nullptr) pos = pos->leftChild;
        pos->leftChild = newNode;
    } else {
        while (pos->rightChild != nullptr) pos = pos->rightChild;
        pos->rightChild = newNode;
    }

    newNode->parent = pos;

    pos = rebalance(pos);

    checkTreeValidity(pos);

    return pos;
}

std::pair<address, address> ETForestCutSet::trivialDelete(ETForestCutSet::Node *root, bool first) {
    assert(root != nullptr);

    Node *u = root;

    //Locating the relevant node
    if (first) {
        while (u->leftChild != nullptr) u = u->leftChild;
    } else {
        while (u->rightChild != nullptr) u = u->rightChild;
    }

    //u has at most 1 child
    Node *parent = u->parent;
    if (u->leftChild == nullptr && u->rightChild == nullptr) {
        if (parent != nullptr) {
            if (parent->leftChild == u) parent->leftChild = nullptr;
            else parent->rightChild = nullptr;
            root = rebalance(parent);
        } else root = nullptr;
    } else if (u->leftChild != nullptr) {
        if (parent != nullptr) {
            if (parent->leftChild == u) parent->leftChild = u->leftChild;
            else parent->rightChild = u->leftChild;
            u->leftChild->parent = parent;
            root = rebalance(parent);
        } else {
            u->leftChild->parent = nullptr;
            root = u->leftChild;
        }
    } else if (u->rightChild != nullptr) {
        if (parent != nullptr) {
            if (parent->leftChild == u) parent->leftChild = u->rightChild;
            else parent->rightChild = u->rightChild;
            u->rightChild->parent = parent;
            root = rebalance(parent);
        } else {
            u->rightChild->parent = nullptr;
            root = u->rightChild;
        }
    }

    cleanNode(u);

    if (root != nullptr) checkTreeValidity(root);

    return {u, root};
}

address ETForestCutSet::rebalance(address start) {
    Node *prev = start;
    while (start != nullptr) {
        prev = start;
        start = start->parent;
        refreshSubtreeInfo(prev);
        if (getHeight(prev->leftChild) > getHeight(prev->rightChild) + 1) {
            //Right rotation necessary
            if (getHeight(prev->leftChild->rightChild) > getHeight(prev->leftChild->leftChild)) {
                //Left-Right-Rotation necessary
                rotate(prev->leftChild, LEFT);
            }
            prev = rotate(prev, RIGHT);
        } else if (getHeight(prev->rightChild) > getHeight(prev->leftChild) + 1) {
            //Right rotation necessary
            if (getHeight(prev->rightChild->leftChild) > getHeight(prev->rightChild->rightChild)) {
                //Left-Right-Rotation necessary
                rotate(prev->rightChild, RIGHT);
            }
            prev = rotate(prev, LEFT);
        }
    }

    return prev;
}

address ETForestCutSet::rotate(ETForestCutSet::Node *head, bool left) {
    Node *parent = head->parent;
    Node *kid;

    //Standard rotate of AVL-Trees
    if (left) {
        assert(head->rightChild != nullptr);
        //assert(head->rightChild->rightChild != nullptr);
        kid = separateDirChild(head, RIGHT);
        Node *innerGrandKid = separateDirChild(kid, LEFT);
        kid->leftChild = head;
        head->parent = kid;
        head->rightChild = innerGrandKid;
        if (innerGrandKid != nullptr) innerGrandKid->parent = head;
    } else {
        assert(head->leftChild != nullptr);
        //assert(head->leftChild->leftChild != nullptr);
        kid = separateDirChild(head, LEFT);
        Node *innerGrandKid = separateDirChild(kid, RIGHT);
        kid->rightChild = head;
        head->parent = kid;
        head->leftChild = innerGrandKid;
        if (innerGrandKid != nullptr) innerGrandKid->parent = head;
    }

    refreshSubtreeInfo(head);
    refreshSubtreeInfo(kid);

    //Fix connection to parents
    kid->parent = parent;

    if (parent != nullptr) {
        if (parent->leftChild == head) parent->leftChild = kid;
        else parent->rightChild = kid;
    }

    return kid;
}

void ETForestCutSet::cleanNode(ETForestCutSet::Node *v) {
    v->parent = nullptr;
    v->leftChild = nullptr;
    v->rightChild = nullptr;
    v->height = 1;
    v->size = 1;
    if (not noTracking) refreshTracking(v);
}

void ETForestCutSet::refreshSubtreeInfo(ETForestCutSet::Node *v) {
    v->size = 1;
    v->height = 1;
    if (v->leftChild != nullptr) {
        v->size += v->leftChild->size;
        v->height = v->leftChild->height + 1;
    }
    if (v->rightChild != nullptr) {
        v->size += v->rightChild->size;
        v->height = std::max(v->height, v->rightChild->height + 1);
    }

    if (not noTracking) refreshTracking(v);
}

void ETForestCutSet::refreshTrackingDataUpwards(ETForestCutSet::Node *v) {
    while (v != nullptr) {
        refreshTracking(v);
        v = v->parent;
    }
}

void ETForestCutSet::refreshTracking(ETForestCutSet::Node *v) {
    assert(not noTracking);

    assert(v->trackingData.ownData == nullptr || v->trackingData.ownData->size() == boostLevel);
    assert(v->trackingData.accumulatedData.size() == boostLevel);

    if (v->trackingData.ownData != nullptr)
        for (count i = 0; i < boostLevel; i++)
            for (count j = 0; j < lognsqr; j++)
                v->trackingData.accumulatedData[i][j] = (*v->trackingData.ownData)[i][j];
    else
        for (count i = 0; i < boostLevel; i++)
            for (count j = 0; j < lognsqr; j++)
                v->trackingData.accumulatedData[i][j] = noEdge;

    if (v->leftChild != nullptr)
        for (count i = 0; i < boostLevel; i++)
            for (count j = 0; j < lognsqr; j++)
                v->trackingData.accumulatedData[i][j] ^= v->leftChild->trackingData.accumulatedData[i][j];

    if (v->rightChild != nullptr)
        for (count i = 0; i < boostLevel; i++)
            for (count j = 0; j < lognsqr; j++)
                v->trackingData.accumulatedData[i][j] ^= v->rightChild->trackingData.accumulatedData[i][j];
}

address ETForestCutSet::separateDirChild(ETForestCutSet::Node *v, bool left) {
    Node *kid;
    if (left) {
        if (v->leftChild == nullptr) return nullptr;
        kid = v->leftChild;
        kid->parent = nullptr;
        v->leftChild = nullptr;
    } else {
        if (v->rightChild == nullptr) return nullptr;
        kid = v->rightChild;
        kid->parent = nullptr;
        v->rightChild = nullptr;
    }

    return kid;
}

count ETForestCutSet::getHeight(ETForestCutSet::Node *v) {
    if (v == nullptr) return 0;
    return v->height;
}

count ETForestCutSet::getSize(ETForestCutSet::Node *v) {
    if (v == nullptr) return 0;
    return v->size;
}

ETForestCutSet::Node::Node(node pv, node pw, ETForestCutSet::Node::TrackingData pTrackingData) {
    v = pv;
    w = pw;
    trackingData = std::move(pTrackingData);
}

void ETForestCutSet::checkTreeValidity(ETForestCutSet::Node *v) {
#ifndef NDEBUG //This code is only executed in debug mode
    //Check if size and height are calculated correctly
    assert(v->height == (std::max(getHeight(v->leftChild), getHeight(v->rightChild)) + 1));
    assert(v->size == (getSize(v->leftChild) + getSize(v->rightChild) + 1));

    //Check if parents and children are fine
    if(v->leftChild != nullptr) assert(v->leftChild->parent == v);
    if(v->rightChild != nullptr) assert(v->rightChild->parent == v);

    //Check if everything is balanced properly
    assert(v->height - getHeight(v->leftChild) <= 2);
    assert(v->height - getHeight(v->rightChild) <= 2);

    //Check if tracking is done properly
    assert(v->trackingData.ownData == nullptr || v->trackingData.ownData->size() == boostLevel);
    assert(v->trackingData.accumulatedData.size() == boostLevel);
    for(int i = 0; i < boostLevel; i++){
        assert(v->trackingData.ownData == nullptr || (*v->trackingData.ownData)[i].size() == lognsqr);
        assert(v->trackingData.accumulatedData[i].size() == lognsqr);
        for(int j = 0; j < lognsqr; j++){
            edge exp = (v->trackingData.ownData != nullptr) ? (*v->trackingData.ownData) [i][j] : noEdge;
            if(v->leftChild != nullptr) exp ^= v->leftChild->trackingData.accumulatedData[i][j];
            if(v->rightChild != nullptr) exp ^= v->rightChild->trackingData.accumulatedData[i][j];

            assert(v->trackingData.accumulatedData[i][j] == exp);
        }
    }

    //Check if everything is ordered
    if(v->leftChild != nullptr){
        Node* prevInOrder = v->leftChild;
        while (prevInOrder->rightChild != nullptr) prevInOrder = prevInOrder->rightChild;
        assert(prevInOrder->w == v->v);
        checkTreeValidity(v->leftChild);
    }
    if(v->rightChild != nullptr){
        Node* nextInOrder = v->rightChild;
        while (nextInOrder->leftChild != nullptr) nextInOrder = nextInOrder->leftChild;
        assert(v->w == nextInOrder->v);
        checkTreeValidity(v->rightChild);
    }
#endif
}

void ETForestCutSet::writeTour(Node *root, std::vector<std::pair<node, node>> *path) {
    if (root->leftChild != nullptr) writeTour(root->leftChild, path);
    path->push_back({root->v, root->w});
    if (root->rightChild != nullptr) writeTour(root->rightChild, path);
}