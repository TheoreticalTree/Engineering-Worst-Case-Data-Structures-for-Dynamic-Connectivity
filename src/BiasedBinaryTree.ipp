#include <cassert>
#include <deque>
#include <stdexcept>

#include "BiasedBinaryTree.hpp"

//template<class Key, class Val> using Node = BiasedBinaryTree<Key, Val>::Node;

template<class Key, class Val>
BiasedBinaryTree<Key, Val>::BiasedBinaryTree(std::function<bool(Key, Key)> pLess){
    less = pLess;
}

template<class Key, class Val>
BiasedBinaryTree<Key, Val>::~BiasedBinaryTree(){
    if(root == nullptr) return;
    std::deque<Node*> killList(1,root);
    while (not killList.empty()){
        Node* v = killList.back();
        killList.pop_back();
        if(not v->isLeaf){
            killList.push_back(v->leftChild);
            killList.push_back(v->rightChild);
        }
        delete v;
    }
}

template<class Key, class Val>
void BiasedBinaryTree<Key, Val>::insert(Key k, Val v, count weight) {
    if(empty()){
        root = new Node(k, v, weight);
        return;
    }

    SplitResult splitResult = split(root, k);
    if(splitResult.splitNode != nullptr){
        root = splitResult.splitNode;
        if(splitResult.leftTree != nullptr){
            root = globalJoin(splitResult.leftTree, root, splitResult.leftSplitNode);
        }
        if(splitResult.rightTree != nullptr){
            root = globalJoin(root, splitResult.rightTree, splitResult.rightSplitNode);
        }
        throw std::runtime_error("Key is already contained\n");
    }

    Node* newNode = new Node(k, v, weight);

    Node* joinNode;
    if(splitResult.leftSplitNode != nullptr) joinNode = splitResult.leftSplitNode;
    else if(splitResult.rightSplitNode != nullptr) joinNode = splitResult.rightSplitNode;
    else joinNode = new Node;

    if(splitResult.leftTree != nullptr){
        newNode = globalJoin(splitResult.leftTree, newNode, joinNode);
        joinNode = nullptr;
    }
    if(splitResult.rightTree != nullptr){
        if(joinNode == nullptr){
            if(splitResult.rightSplitNode != nullptr) joinNode = splitResult.rightSplitNode;
            else joinNode = new Node;
        }
        newNode = globalJoin(newNode, splitResult.rightTree, joinNode);
    }

    root = newNode;
}

template<class Key, class Val>
Val BiasedBinaryTree<Key, Val>::remove(Key k) {
    assert(root != nullptr);

    //We split
    SplitResult splitResult = split(root, k);
    if(splitResult.splitNode == nullptr) {
        //Case: The value didn't exist
        //This is so even on an error the memory management works fine (in case the error is caught)
        if(splitResult.leftTree != nullptr){
            if(splitResult.rightTree != nullptr){
                Node* joinNode = splitResult.leftSplitNode;
                if(splitResult.rightSplitNode != nullptr) joinNode = splitResult.rightSplitNode;
                root = globalJoin(splitResult.leftTree, splitResult.rightTree, joinNode);
            }
            else root = splitResult.leftTree;
        }
        else root = splitResult.rightTree;
        throw std::runtime_error("No value stored for key\n");
    }

    Val ret = splitResult.splitNode->v;
    delete splitResult.splitNode;

    if(splitResult.leftTree == nullptr) {
        root = splitResult.rightTree;
        delete splitResult.rightSplitNode;
    }
    else if(splitResult.rightTree == nullptr){
        root = splitResult.leftTree;
        delete splitResult.leftSplitNode;
    }
    else {
        root = globalJoin(splitResult.leftTree, splitResult.rightTree, splitResult.leftSplitNode);
        delete splitResult.rightSplitNode;
    }

    return ret;
}

template<class Key, class Val>
bool BiasedBinaryTree<Key, Val>::contains(Key k) const {
    Node* here = root;
    //Move until a leaf is found
    while (not here->isLeaf){
        if(less(k, here->k)){
            here = here->leftChild;
        }
        else here = here->rightChild;
    }

    if(here->k == k) return true;
    else return false;
}

template<class Key, class Val>
Val BiasedBinaryTree<Key, Val>::findVal(Key k) const {
    Node* here = root;
    //Move until a leaf is found
    while (not here->isLeaf){
        if(less(k, here->k)){
            here = here->leftChild;
        }
        else here = here->rightChild;
    }

    if(here->k == k) return here->v;
    else throw std::runtime_error("No value stored for key\n");
}

template<class Key, class Val>
Val BiasedBinaryTree<Key, Val>::changeVal(Key k, Val v) {
    Node* here = root;
    //Move until a leaf is found
    while (not here->isLeaf){
        if(less(k, here->k)){
            here = here->leftChild;
        }
        else here = here->rightChild;
    }

    if(here->k == k) {
        Val ret = here->v;
        here->v = v;
        return ret;
    }
    else throw std::runtime_error("No value stored for key\n");
}

template<class Key, class Val>
bool BiasedBinaryTree<Key, Val>::empty() const {
    return root == nullptr;
}

template<class Key, class Val>
std::pair<Key, Val> BiasedBinaryTree<Key, Val>::getMin() const {
    return {root->leftMost->k, root->leftMost->v};
}

template<class Key, class Val>
std::pair<Key, Val> BiasedBinaryTree<Key, Val>::getMax() const {
    return {root->rightMost->k, root->rightMost->v};
}

template<class Key, class Val>
std::vector<std::pair<Key, Val>> BiasedBinaryTree<Key, Val>::getSortedSet() const {
    if(empty()) return std::vector<std::pair<Key, Val>>();

    std::vector<std::pair<Key, Val>> ret;

    std::deque<Node*> queue(1,root);

    while (not queue.empty()){
        Node* node = queue.back();
        queue.pop_back();

        if(node->isLeaf){
            ret.push_back({node->k, node->v});
        }
        else {
            queue.emplace_back(node->rightChild);
            queue.emplace_back(node->leftChild);
        }
    }

    return ret;
}

template<class Key, class Val>
count BiasedBinaryTree<Key, Val>::getWeightSum() const {
    if(root == nullptr) return 0;
    else return root->weight;
}

template<class Key, class Val>
BiasedBinaryTree<Key, Val>::SplitResult BiasedBinaryTree<Key, Val>::split(Node* pRoot, Key k) {
    //Check for trivial cases of key is greater then the greatest or smaller then the smallest entry
    if(less(k, pRoot->leftMost->k)) return {nullptr, nullptr, pRoot, nullptr, nullptr};
    if(less(pRoot->rightMost->k, k)) return {pRoot, nullptr, nullptr, nullptr, nullptr};

    //Step 1: locate the node to split at
    Node* here = pRoot;
    //Move until a leaf is found
    while (not here->isLeaf){
        if(less(k, here->k)){
            here = here->leftChild;
        }
        else here = here->rightChild;
    }

    //Check if we are splitting at a leaf
    Node* splitLeaf = nullptr;
    Node *splitNodeLeft = nullptr, *splitNodeRight = nullptr;
    Node *leftTree = nullptr, *rightTree = nullptr;
    Node *prev = here, *parent = nullptr;
    if(here->k == k){
        splitLeaf = here;
    }
    else{
        //The value we are splitting at is between this leaf and the next bigger leaf value
        here = here->parent;
        while(here != nullptr && prev != here->leftChild){
            //Move up the tree until either we move right or we shoot out of the root
            prev = here;
            here = here->parent;
        }
        if(here == nullptr){
            throw std::runtime_error("BiasedBinaryTrees suffered an internal error. less may not define a total ordering");
        }
        else {
            CutOffKids splitKids = separateKids(here);
            splitNodeLeft = here;
            leftTree = splitKids.first, rightTree = splitKids.second;
        }
    }

    prev = here;
    here = here->parent;
    prev->parent = nullptr;
    if(here != nullptr){
        if(prev == here->leftChild) here->leftChild = nullptr;
        else here->rightChild = nullptr;
    }
    while (here != nullptr){
        //Chop off here from its parent
        parent = here->parent;
        here->parent = nullptr;
        if(parent != nullptr){
            if(parent->leftChild == here) parent->leftChild = nullptr;
            else parent->rightChild = nullptr;
        }

        CutOffKids hereKids = separateKids(here);
        if(hereKids.first == nullptr){
            if(rightTree == nullptr){
                rightTree = hereKids.second;
                splitNodeRight = here;
            }
            else {
                rightTree = localJoin(rightTree, hereKids.second, here);
            }

            checkTreeValidity(rightTree);
            assert(rightTree->parent == nullptr);
        }
        else {
            if(leftTree == nullptr){
                leftTree = hereKids.first;
                splitNodeLeft = here;
            }
            else {
                leftTree = localJoin(hereKids.first, leftTree, here);
            }

            checkTreeValidity(leftTree);
            assert(leftTree->parent == nullptr);
        }

        here = parent;
    }

    return {leftTree, splitLeaf, rightTree, splitNodeLeft, splitNodeRight};
}

template<class Key, class Val>
BiasedBinaryTree<Key, Val>::Node* BiasedBinaryTree<Key, Val>::localJoin(Node* leftTree, Node* rightTree, Node* joinNode) {
    assert(leftTree->parent == nullptr);
    assert(rightTree->parent == nullptr);

    if((leftTree->rank == rightTree->rank) || ((leftTree->rank >= rightTree->rank) && leftTree->isLeaf) || ((rightTree->rank >= leftTree->rank) && rightTree->isLeaf)){
        //Case 1 of local join
        attachChildren(joinNode, leftTree, rightTree);
        joinNode->rank = std::max(leftTree->rank, rightTree->rank) + 1;
        checkTreeValidity(joinNode);
        assert(joinNode->parent == nullptr);
        return joinNode;
    }
    else if(leftTree->rank >= rightTree->rank){
        //Case 2 of local join
        leftTree = tilt(leftTree, true);
        Node* lrKid = separateRight(leftTree);
        Node* z = localJoin(lrKid, rightTree, joinNode);
        attachChildRight(leftTree, z);
        checkTreeValidity(leftTree);
        assert(leftTree->parent == nullptr);
        return leftTree;
    }
    else {
        //Case 3 of local join
        rightTree = tilt(rightTree, false);
        Node* rlKid = separateLeft(rightTree);
        Node* z = localJoin(leftTree, rlKid, joinNode);
        attachChildLeft(rightTree, z);
        checkTreeValidity(rightTree);
        assert(rightTree->parent == nullptr);
        return rightTree;
    }
}

template<class Key, class Val>
BiasedBinaryTree<Key, Val>::Node* BiasedBinaryTree<Key, Val>::globalJoin(Node* leftTree, Node* rightTree, Node* joinNode) {
    assert(leftTree != nullptr && leftTree->parent == nullptr);
    assert(rightTree != nullptr && rightTree->parent == nullptr);
    assert(joinNode->parent == nullptr && joinNode->leftChild == nullptr && joinNode->rightChild == nullptr);

    if(((leftTree->rank >= rightTree->rank) && leftTree->isLeaf) || ((rightTree->rank >= leftTree->rank) && rightTree->isLeaf)){
        return globalJoinCase1(leftTree, rightTree, joinNode);
    }
    else if (leftTree->rank != rightTree->rank){
        return globalJoinCase23(leftTree, rightTree, joinNode);
    }
    else {
        return globalJoinCase4(leftTree, rightTree, joinNode);
    }
}

template<class Key, class Val>
BiasedBinaryTree<Key, Val>::Node* BiasedBinaryTree<Key, Val>::globalJoinCase1(Node* leftTree, Node* rightTree, Node* joinNode) {
    attachChildren(joinNode, leftTree, rightTree);
    joinNode->rank = std::max(leftTree->rank, rightTree->rank) + 1;

    checkTreeValidity(joinNode);
    assert(joinNode->parent == nullptr);

    return joinNode;
}

template<class Key, class Val>
BiasedBinaryTree<Key, Val>::Node* BiasedBinaryTree<Key, Val>::globalJoinCase23(Node* leftTree, Node* rightTree, Node* joinNode) {
    if(leftTree->rank > rightTree->rank){
        leftTree = tilt(leftTree, true);

        Node* rightKid = separateRight(leftTree);
        Node* z = globalJoin(rightKid, rightTree, joinNode);
        attachChildRight(leftTree, z);

        checkTreeValidity(leftTree);
        assert(leftTree->parent == nullptr);

        return leftTree;
    }
    else {
        rightTree = tilt(rightTree, false);

        Node* leftKid = separateLeft(rightTree);
        Node* z = globalJoin(leftTree, leftKid, joinNode);
        attachChildLeft(rightTree, z);

        checkTreeValidity(rightTree);
        assert(rightTree->parent == nullptr);

        return rightTree;
    }
}

template<class Key, class Val>
BiasedBinaryTree<Key, Val>::Node* BiasedBinaryTree<Key, Val>::globalJoinCase4(Node* leftTree, Node* rightTree, Node* joinNode) {
    assert(leftTree->rank == rightTree->rank);
    assert(not leftTree->isLeaf && not rightTree->isLeaf);

    //u is the rightmost node in leftTree with rank == leftTree->rank, uRight is the right child
    //v is symmetric
    Node *u, *v, *uRight, *vLeft;
    if(leftTree->rightChild->rank == leftTree->rank){
        u = separateRight(leftTree);
    }
    else {
        u = leftTree;
    }
    uRight = separateRight(u);
    if(rightTree->leftChild->rank == rightTree->rank){
        v = separateLeft(rightTree);
    }
    else {
        v = rightTree;
    }
    vLeft = separateLeft(v);

    Node* z = globalJoin(uRight, vLeft, joinNode);

    if(z->rank == leftTree->rank){
        return globalJoinCase4a(leftTree, rightTree, u, v, z, joinNode);
    }
    else {
        if(u != leftTree){
            return globalJoinCase4bi(leftTree, rightTree, u, v, z, joinNode);
        }
        else if(v != rightTree){
            return globalJoinCase4bii(leftTree, rightTree, u, v, z, joinNode);
        }
        else if(leftTree->leftChild->rank == leftTree->rank){
            return globalJoinCase4biii(leftTree, rightTree, u, v, z, joinNode);
        }
        else if(rightTree->rightChild->rank == rightTree->rank){
            return globalJoinCase4biv(leftTree, rightTree, u, v, z, joinNode);
        }
        else {
            return globalJoinCase4bv(leftTree, rightTree, u, v, z, joinNode);
        }
    }
}

template<class Key, class Val>
BiasedBinaryTree<Key, Val>::Node* BiasedBinaryTree<Key, Val>::globalJoinCase4a(Node* leftTree, Node* rightTree, Node* u, Node* v, Node* z, Node* joinNode) {
    CutOffKids zKids = separateKids(z);
    attachChildRight(u, zKids.first);
    if(u != leftTree) attachChildRight(leftTree, u);
    attachChildLeft(v, zKids.second);
    if(v != rightTree) attachChildLeft(rightTree, v);
    attachChildren(z, leftTree, rightTree);

    z->rank = leftTree->rank + 1;

    checkTreeValidity(z);
    assert(z->parent == nullptr);

    return z;
}

template<class Key, class Val>
BiasedBinaryTree<Key, Val>::Node* BiasedBinaryTree<Key, Val>::globalJoinCase4bi(Node* leftTree, Node* rightTree, Node* u, Node* v, Node* z, Node* joinNode) {
    Node* uLeft = separateLeft(u);
    attachChildRight(leftTree, uLeft);
    attachChildLeft(u, leftTree);
    attachChildLeft(v, z);
    if(v != rightTree) attachChildLeft(rightTree, v);
    attachChildRight(u, rightTree);

    u->rank = leftTree->rank + 1;

    checkTreeValidity(u);
    assert(u->parent == nullptr);

    return u;
}

template<class Key, class Val>
BiasedBinaryTree<Key, Val>::Node* BiasedBinaryTree<Key, Val>::globalJoinCase4bii(Node* leftTree, Node* rightTree, Node* u, Node* v, Node* z, Node* joinNode) {
    Node* vRight = separateRight(v);
    attachChildLeft(rightTree, vRight);
    attachChildRight(v, rightTree);
    attachChildRight(u, z);
    if(u != leftTree) attachChildRight(leftTree, u);
    attachChildLeft(v, leftTree);

    v->rank = rightTree->rank + 1;

    checkTreeValidity(v);
    assert(v->parent == nullptr);

    return v;
}

template<class Key, class Val>
BiasedBinaryTree<Key, Val>::Node* BiasedBinaryTree<Key, Val>::globalJoinCase4biii(Node* leftTree, Node* rightTree, Node* u, Node* v, Node* z, Node* joinNode) {
    attachChildLeft(rightTree, z);
    attachChildRight(leftTree, rightTree);

    leftTree->rank++;

    checkTreeValidity(leftTree);
    assert(leftTree->parent == nullptr);

    return leftTree;
}

template<class Key, class Val>
BiasedBinaryTree<Key, Val>::Node* BiasedBinaryTree<Key, Val>::globalJoinCase4biv(Node* leftTree, Node* rightTree, Node* u, Node* v, Node* z, Node* joinNode) {
    attachChildRight(leftTree, z);
    attachChildLeft(rightTree, leftTree);

    rightTree->rank++;

    checkTreeValidity(rightTree);
    assert(rightTree->parent == nullptr);

    return rightTree;
}

template<class Key, class Val>
BiasedBinaryTree<Key, Val>::Node* BiasedBinaryTree<Key, Val>::globalJoinCase4bv(Node* leftTree, Node* rightTree, Node* u, Node* v, Node* z, Node* joinNode) {
    attachChildLeft(rightTree, z);
    attachChildRight(leftTree, rightTree);

    checkTreeValidity(leftTree);
    assert(leftTree->parent == nullptr);

    return leftTree;
}

template<class Key, class Val>
BiasedBinaryTree<Key, Val>::Node* BiasedBinaryTree<Key, Val>::tilt(Node* pRoot, bool left) {
    assert(pRoot->parent == nullptr);
    assert(not pRoot->isLeaf);

    if(pRoot->getDirChild(not left)->rank < pRoot->rank) return pRoot;
    else if (pRoot->getDirChild(left)->rank == pRoot->rank){
        pRoot->rank++;
        return pRoot;
    }
    else return rotate(pRoot, left);
}

template<class Key, class Val>
BiasedBinaryTree<Key, Val>::Node* BiasedBinaryTree<Key, Val>::rotate(Node *pRoot, bool left) {
    assert(not pRoot->isLeaf);
    if(left) assert(not pRoot->rightChild->isLeaf);
    else assert(not pRoot->leftChild->isLeaf);

    if(left){
        Node* kid = separateRight(pRoot);
        Node* middleTree = separateLeft(kid);
        attachChildRight(pRoot, middleTree);
        attachChildLeft(kid, pRoot);
        return kid;
    }
    else {
        Node* kid = separateLeft(pRoot);
        Node* middleTree = separateRight(kid);
        attachChildLeft(pRoot, middleTree);
        attachChildRight(kid, pRoot);
        return kid;
    }
}

template<class Key, class Val>
BiasedBinaryTree<Key, Val>::Node* BiasedBinaryTree<Key, Val>::separateLeft(Node* v) {
    assert(not v->isLeaf);

    Node* kid = v->leftChild;
    v->leftChild = nullptr;
    if(kid != nullptr) {
        kid->parent = nullptr;
        v->weight -= kid->weight;
    }
    return kid;
}

template<class Key, class Val>
BiasedBinaryTree<Key, Val>::Node* BiasedBinaryTree<Key, Val>::separateRight(Node* v) {
    assert(not v->isLeaf);

    Node* kid = v->rightChild;
    v->rightChild = nullptr;
    if(kid != nullptr) {
        kid->parent = nullptr;
        v->weight -= kid->weight;
    }
    return kid;
}

template<class Key, class Val>
BiasedBinaryTree<Key, Val>::CutOffKids BiasedBinaryTree<Key, Val>::separateKids(Node* v) {
    assert(not v->isLeaf);

    Node* kidLeft = v->leftChild;
    Node* kidRight = v->rightChild;
    v->leftChild = nullptr;
    v->rightChild = nullptr;
    if(kidLeft != nullptr) kidLeft->parent = nullptr;
    if(kidRight != nullptr) kidRight->parent = nullptr;
    v->weight = 0;
    return {kidLeft, kidRight};
}

template<class Key, class Val>
void BiasedBinaryTree<Key, Val>::attachChildLeft(Node *v, Node* child) {
    assert(not v->isLeaf);
    assert(v->leftChild == nullptr);
    assert(child->parent == nullptr);

    v->leftChild = child;
    child->parent = v;
    v->leftMost = child->leftMost;
    v->weight += child->weight;
}

template<class Key, class Val>
void BiasedBinaryTree<Key, Val>::attachChildRight(Node *v, Node* child) {
    assert(not v->isLeaf);
    assert(v->rightChild == nullptr);
    assert(child->parent == nullptr);

    v->rightChild = child;
    child->parent = v;
    v->rightMost = child->rightMost;
    v->weight += child->weight;

    v->k = child->leftMost->k;
}

template<class Key, class Val>
void BiasedBinaryTree<Key, Val>::attachChildren(Node* v, Node* leftChild, Node* rightChild) {
    assert(not v->isLeaf);
    assert(v->leftChild == nullptr);
    assert(v->rightChild == nullptr);
    assert(leftChild->parent == nullptr);
    assert(rightChild->parent == nullptr);

    v->leftChild = leftChild;
    v->rightChild = rightChild;
    v->leftMost = leftChild->leftMost;
    v->rightMost = rightChild->rightMost;
    leftChild->parent = v;
    rightChild->parent = v;

    v->weight = leftChild->weight + rightChild->weight;

    v->k = rightChild->leftMost->k;
}

template<class Key, class Val>
BiasedBinaryTree<Key, Val>::Node* BiasedBinaryTree<Key, Val>::Node::getDirChild(bool left){
    if(left) return leftChild;
    else return rightChild;
}

template<class Key, class Val>
BiasedBinaryTree<Key, Val>::Node::Node(Key pk, Val pv, count pWeight) {
    k = pk;
    v = pv;
    weight = pWeight;
    if(pWeight == 0) throw std::runtime_error("Weight must be > 0");
    rank = std::floor(std::log2(weight));
    leftMost = this;
    rightMost = this;
    isLeaf = true;
}

template<class Key, class Val>
BiasedBinaryTree<Key, Val>::Node* BiasedBinaryTree<Key, Val>::getRoot(BiasedBinaryTree::Node *v) {
    while (v->parent != nullptr) v = v->parent;
    return v;
}

template<class Key, class Val>
BiasedBinaryTree<Key, Val>::Node* BiasedBinaryTree<Key, Val>::getDirNeighbouringLeaf(BiasedBinaryTree::Node *v, bool left) {
    Node* prev = v;
    v = v->parent;
    while (v != nullptr){
        if(left){
            if(prev == v->rightChild){
                //We moved upwards to the left
                return v->leftChild->rightMost;
            }
        }
        else {
            if(prev == v->leftChild){
                //We moved upwards to the right
                return v->rightChild->leftMost;
            }
        }
        prev = v;
        v = v->parent;
    }
    return nullptr;
}

template<class Key, class Val>
void BiasedBinaryTree<Key, Val>::checkTreeValidity(Node *pRoot) {
#ifndef NDEBUG //this code only runs in debug mode
    //Step 1: assert that the pointers are set in both directions in all cases
    if(pRoot->parent != nullptr){
        assert(pRoot->parent->leftChild == pRoot || pRoot->parent->rightChild == pRoot);
    }
    if(pRoot->leftChild != nullptr) assert(pRoot->leftChild->parent == pRoot);
    if(pRoot->rightChild != nullptr) assert(pRoot->rightChild->parent == pRoot);

    //Step 2: assert that isLeaf is correct
    if(pRoot->isLeaf){
        assert(pRoot->leftChild == nullptr && pRoot->rightChild == nullptr);
        assert(pRoot->leftMost == pRoot && pRoot->rightMost == pRoot);
    }
    else {
        assert(pRoot->leftChild != nullptr && pRoot->rightChild != nullptr);
        assert(pRoot->leftMost == pRoot->leftChild->leftMost && pRoot->rightMost == pRoot->rightChild->rightMost);
    }

    //Step 3: Assert that weight and rank are calculated properly
    if(pRoot->isLeaf){
        assert(pRoot->rank == std::floor(std::log2(pRoot->weight)));
    }
    else {
        assert(pRoot->weight == pRoot->leftChild->weight + pRoot->rightChild->weight);
        assert(pRoot->rank <= std::floor(std::log2(pRoot->weight)) + 1);
    }

    //Step 4: Assert that the rank-rules with parents and grandparents are followed properly
    if(pRoot->parent != nullptr){
        if(pRoot->isLeaf) assert(pRoot->rank < pRoot->parent->rank);
        else assert(pRoot->rank <= pRoot->parent->rank);
        if(pRoot->parent->parent != nullptr) assert(pRoot->rank < pRoot->parent->parent->rank);
    }

    //Step 5: Assert that global bias is intact
    if(pRoot->parent != nullptr){
        if(pRoot->rank + 2 <= pRoot->parent->rank){
            Node* leftNeighbouringLeaf = getDirNeighbouringLeaf(pRoot, true);
            if(leftNeighbouringLeaf != nullptr) assert(leftNeighbouringLeaf->rank + 1 >= pRoot->parent->rank);
            Node* rightNeighbouringLeaf = getDirNeighbouringLeaf(pRoot, false);
            if(rightNeighbouringLeaf != nullptr) assert(rightNeighbouringLeaf->rank + 1 >= pRoot->parent->rank);
        }
    }

    //Step 6: Assert search tree properties and terrorize children
    if(not pRoot->isLeaf){
        assert(less(pRoot->leftChild->k, pRoot->k));
        assert(less(pRoot->leftChild->rightMost->k, pRoot->k));
        assert(pRoot->k == pRoot->rightChild->leftMost->k);

        checkTreeValidity(pRoot->leftChild);
        checkTreeValidity(pRoot->rightChild);
    }
#endif
}