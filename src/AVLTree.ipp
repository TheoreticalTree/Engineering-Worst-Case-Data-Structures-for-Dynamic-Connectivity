#include <cassert>
#include <stdexcept>
#include <deque>

#include "AVLTree.hpp"

template<class Key, class Val>
AVLTree<Key, Val>::AVLTree(std::function<bool(Key, Key)> pLess) {
    less = pLess;
}

template<class Key, class Val>
AVLTree<Key, Val>::~AVLTree(){
    if(root == nullptr) return;
    std::deque<Node*> killList(1,root);
    while (not killList.empty()){
        Node* v = killList.back();
        killList.pop_back();
        if(not (v->leftChild == nullptr)) killList.push_back(v->leftChild);
        if(not (v->rightChild == nullptr)) killList.push_back(v->rightChild);
        delete v;
    }
}

template<class Key, class Val>
void AVLTree<Key, Val>::insert(Key k, Val v) {
    if(root == nullptr) {
        root = new Node(k, v);
        return;
    }
    //Step 1: Move down until we find the place where the new pair belongs
    Node* pos = root;
    Node* newNode = new Node(k,v);

    while (pos != nullptr){
        if(less(k, pos->k)){
            if(pos->leftChild == nullptr){
                pos->leftChild = newNode;
                newNode->parent = pos;
                root = rebalance(pos);

                checkTreeValidity(root);

                return;
            }
            else {
                pos = pos->leftChild;
            }
        }
        else if(k == pos->k){
            throw std::runtime_error("Key is already stored\n");
        }
        else {
            if(pos->rightChild == nullptr){
                pos->rightChild = newNode;
                newNode->parent = pos;
                root = rebalance(pos);

                checkTreeValidity(root);

                return;
            }
            else {
                pos = pos->rightChild;
            }
        }
    }
}

template<class Key, class Val>
bool AVLTree<Key, Val>::contains(Key k) const {
    Node* v = root;

    while (v != nullptr){
        if(k == v->k) return true;
        else if(less(k, v->k)) v = v->leftChild;
        else v = v->rightChild;
    }

    return false;
}

template<class Key, class Val>
Val AVLTree<Key, Val>::findVal(Key k) const {
    Node* v = root;

    while (v != nullptr){
        if(k == v->k) return v->v;
        else if(less(k, v->k)) v = v->leftChild;
        else v = v->rightChild;
    }

    throw std::runtime_error("No value stored for key\n");
}

template<class Key, class Val>
Val &AVLTree<Key, Val>::getValReference(Key k) const {
    Node* v = root;

    while (v != nullptr){
        if(k == v->k) return v->v;
        else if(less(k, v->k)) v = v->leftChild;
        else v = v->rightChild;
    }

    throw std::runtime_error("No value stored for key\n");
}

template<class Key, class Val>
Val AVLTree<Key, Val>::changeVal(Key k, Val v) {
    Node* u = root;

    while (u != nullptr){
        if(k == u->k){
            Val ret = u->v;
            u->v = v;
            return ret;
        }
        else if(less(k, u->k)) u = u->leftChild;
        else u = u->rightChild;
    }

    throw std::runtime_error("No value stored under this key not stored\n");
}

template<class Key, class Val>
Val AVLTree<Key, Val>::remove(Key k) {
    Node* u = root;

    //Locating the relevant node
    while (u != nullptr){
        if(k == u->k){
            break;
        }
        else if(less(k, u->k)) u = u->leftChild;
        else u = u->rightChild;
    }

    if(u == nullptr) throw std::runtime_error("No value stored under this key not stored\n");

    //if u has two children swap u with the next element left of it
    if(u->leftChild != nullptr && u->rightChild != nullptr){
        Node* nextRight = u->rightChild;
        while (nextRight->leftChild != nullptr) nextRight = nextRight->leftChild;
        Key tk = u->k;
        Val tv = u->v;
        u->k = nextRight->k;
        u->v = nextRight->v;
        nextRight->k = tk;
        nextRight->v = tv;
        u = nextRight;
    }

    Val ret = u->v;

    //Now u has at most 1 child
    Node* parent = u->parent;
    if(u->leftChild == nullptr && u->rightChild == nullptr){
        if(parent != nullptr){
            if(parent->leftChild == u) parent->leftChild = nullptr;
            else parent->rightChild = nullptr;
            root = rebalance(parent);
        }
        else root = nullptr;
    }
    else if(u->leftChild != nullptr){
        if(parent != nullptr){
            if(parent->leftChild == u) parent->leftChild = u->leftChild;
            else parent->rightChild = u->leftChild;
            u->leftChild->parent = parent;
            root = rebalance(parent);
        }
        else{
            u->leftChild->parent = nullptr;
            root = u->leftChild;
        }
    }
    else if(u->rightChild != nullptr){
        if(parent != nullptr){
            if(parent->leftChild == u) parent->leftChild = u->rightChild;
            else parent->rightChild = u->rightChild;
            u->rightChild->parent = parent;
            root = rebalance(parent);
        }
        else{
            u->rightChild->parent = nullptr;
            root = u->rightChild;
        }
    }

    delete u;

    return ret;
}

template<class Key, class Val>
bool AVLTree<Key, Val>::empty() const {
    return root == nullptr;
}

template<class Key, class Val>
std::vector<std::pair<Key, Val>> AVLTree<Key, Val>::getSortedSet() const {
    if(root == nullptr) return {};

    std::vector<std::pair<Key, Val>> ret;

    std::deque<Node*> queue;

    Node* v = root;

    while ((not queue.empty()) || (not (v == nullptr))){
        if(v != nullptr){
            queue.emplace_back(v);
            v = v->leftChild;
        }
        else {
            v = queue.back();
            queue.pop_back();
            ret.emplace_back(v->k, v->v);
            v = v->rightChild;
        }
    }

    return ret;
}

template<class Key, class Val>
std::pair<Key, Val> AVLTree<Key, Val>::getAnyEntry() const {
    if(root != nullptr) return {root->k, root->v};
    else throw std::runtime_error("AVLTree is empty");
}

template<class Key, class Val>
AVLTree<Key, Val>::Node* AVLTree<Key, Val>::rebalance(AVLTree::Node *start) {
    Node* prev = start;
    while (start != nullptr){
        prev = start;
        start = start->parent;
        refreshSubtreeInfo(prev);
        if(getHeight(prev->leftChild) > getHeight(prev->rightChild) + 1){
            //Right rotation necessary
            if(getHeight(prev->leftChild->rightChild) > getHeight(prev->leftChild->leftChild)){
                //Left-Right-Rotation necessary
                rotate(prev->leftChild, LEFT);
            }
            prev = rotate(prev, RIGHT);
        }
        else if(getHeight(prev->rightChild) > getHeight(prev->leftChild) + 1){
            //Right rotation necessary
            if(getHeight(prev->rightChild->leftChild) > getHeight(prev->rightChild->rightChild)){
                //Left-Right-Rotation necessary
                rotate(prev->rightChild, RIGHT);
            }
            prev = rotate(prev, LEFT);
        }
    }

    return prev;
}

template<class Key, class Val>
AVLTree<Key, Val>::Node* AVLTree<Key, Val>::rotate(Node* head, bool left){
    Node* parent = head->parent;
    Node* kid;

    //Standard rotate of AVL-Trees
    if(left){
        assert(head->rightChild != nullptr);
        //assert(head->rightChild->rightChild != nullptr);
        kid = separateDirChild(head, RIGHT);
        Node* innerGrandKid = separateDirChild(kid, LEFT);
        kid->leftChild = head;
        head->parent = kid;
        head->rightChild = innerGrandKid;
        if(innerGrandKid != nullptr) innerGrandKid->parent = head;
    }
    else {
        assert(head->leftChild != nullptr);
        //assert(head->leftChild->leftChild != nullptr);
        kid = separateDirChild(head, LEFT);
        Node* innerGrandKid = separateDirChild(kid, RIGHT);
        kid->rightChild = head;
        head->parent = kid;
        head->leftChild = innerGrandKid;
        if(innerGrandKid != nullptr) innerGrandKid->parent = head;
    }

    refreshSubtreeInfo(head);
    refreshSubtreeInfo(kid);

    //Fix connection to parents
    kid->parent = parent;

    if(parent != nullptr){
        if(parent->leftChild == head) parent->leftChild = kid;
        else parent->rightChild = kid;
    }

    return kid;
}

template<class Key, class Val>
void AVLTree<Key, Val>::refreshSubtreeInfo(AVLTree::Node *v) {
    v->size = 1;
    v->height = 1;
    if(v->leftChild != nullptr){
        v->size += v->leftChild->size;
        v->height = v->leftChild->height + 1;
    }
    if(v->rightChild != nullptr){
        v->size += v->rightChild->size;
        v->height = std::max(v->height, v->rightChild->height + 1);
    }
}

template<class Key, class Val>
AVLTree<Key, Val>::Node* AVLTree<Key, Val>::separateDirChild(Node* v, bool left){
    Node* kid;
    if(left){
        if(v->leftChild == nullptr) return nullptr;
        kid = v->leftChild;
        kid->parent = nullptr;
        v->leftChild = nullptr;
    }
    else {
        if(v->rightChild == nullptr) return nullptr;
        kid = v->rightChild;
        kid->parent = nullptr;
        v->rightChild = nullptr;
    }

    return kid;
}

template<class Key, class Val>
count AVLTree<Key, Val>::getHeight(Node *v) {
    if(v == nullptr) return 0;
    return v->height;
}

template<class Key, class Val>
count AVLTree<Key, Val>::getSize(Node *v) {
    if(v == nullptr) return 0;
    return v->size;
}

template<class Key, class Val>
AVLTree<Key, Val>::Node::Node(Key pk, Val pv) {
    k = pk;
    v = pv;
}

template<class Key, class Val>
void AVLTree<Key, Val>::checkTreeValidity(Node *v) {
#ifndef NDEBUG //This code is only executed in debug mode
    //Check if size and height are calculated correctly
    assert(v->height == (std::max(getHeight(v->leftChild), getHeight(v->rightChild)) + 1));
    assert(v->size == (getSize(v->leftChild) + getSize(v->rightChild) + 1));

    //Check if parents and children are fine
    if(v->parent == nullptr) assert(root == v);
    if(v->leftChild != nullptr) assert(v->leftChild->parent == v);
    if(v->rightChild != nullptr) assert(v->rightChild->parent == v);

    //Check if everything is balanced properly
    assert(v->height - getHeight(v->leftChild) <= 2);
    assert(v->height - getHeight(v->rightChild) <= 2);

    //Check if everything is ordered
    if(v->leftChild != nullptr){
        Node* prevInOrder = v->leftChild;
        while (prevInOrder->rightChild != nullptr) prevInOrder = prevInOrder->rightChild;
        assert(less(prevInOrder->k, v->k));
        checkTreeValidity(v->leftChild);
    }
    if(v->rightChild != nullptr){
        Node* nextInOrder = v->rightChild;
        while (nextInOrder->leftChild != nullptr) nextInOrder = nextInOrder->leftChild;
        assert(less(v->k, nextInOrder->k));
        checkTreeValidity(v->rightChild);
    }
#endif
}