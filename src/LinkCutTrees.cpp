#include <cassert>
#include <functional>

#include "LinkCutTrees.hpp"

LinkCutTrees::LinkCutTrees(count pn) {
    n = pn;
    parent.resize(n, none);
    pCost.resize(n, 0);
    pathNodes.resize(n);
    //This abomination provides the BBH with a comparison function for lexicographic order
    pathSets.resize(n, BiasedBinaryTree<std::pair<count, node>, Path>(
            std::function<bool(std::pair<count, node>, std::pair<count, node>)> {[](std::pair<count, node> a, std::pair<count, node> b) {
                    if(a.first == b.first) return a.second < b.second;
                    else return a.first < b.first;
            }
        }
    ));
    for(count i = 0; i < n; i++){
        pathNodes[i] = new BBTNode(i,1);
    }
}

LinkCutTrees::~LinkCutTrees() {
    for(count i = 0; i < n; i++){
        if(pathNodes[i] != nullptr){
            std::vector<node> pathOfI;
            forest.writePath(forest.getBBTRoot(pathNodes[i]), &pathOfI);
            forest.deleteTree(pathNodes[i]);
            for(node v : pathOfI){
                pathNodes[v] = nullptr;
            }
        }
    }
}

void LinkCutTrees::link(node v, node w, cost x) {
    assert(getRoot(v) != getRoot(w));

    reroot(v);
    Path pathV = forest.getBBTRoot(pathNodes[v]);
    Path pathW = expose(w);
    Path path = forest.globalJoin(pathV, pathW, x);
    conceal(path);

    checkForestValidity();
}

cost LinkCutTrees::cut(node v) {
    assert(v != getRoot(v));

    expose(v);
    SplitResult splitRes = forest.globalSplit(pathNodes[v]);
    parent[v] = none;

    conceal(splitRes.rightTree);
    conceal(pathNodes[v]);

    checkForestValidity();

    return splitRes.rCost;
}

cost LinkCutTrees::cutEdge(node v, node w) {
    assert(getRoot(v) == getRoot(w));
    assert((v == getParent(w)) || (w == getParent(v)));

    node oldRoot = getRoot(v);
    reroot(w);
    cost ret = cut(v);
    reroot(oldRoot);

    checkForestValidity();

    return ret;
}

node LinkCutTrees::getParent(node v) {
    Path pathV = forest.getBBTRoot(pathNodes[v]);
    if(v != forest.getEnd(pathV)) return forest.getAfter(pathNodes[v]).first;
    else return parent[v];
}

node LinkCutTrees::getRoot(node v) {
    Path path = expose(v);
    node ret = forest.getEnd(path);
    conceal(path);

    checkForestValidity();

    return ret;
}

cost LinkCutTrees::getCost(node v) {
    Path path = forest.getBBTRoot(pathNodes[v]);
    if(forest.getEnd(path) == v){
        if(parent[v] != none) return pCost[v];
        else return 0;
    }
    else return forest.getAfter(pathNodes[v]).second;
}

std::pair<std::pair<node, node>, cost> LinkCutTrees::getMinEdge(node v) {
    Path path = expose(v);
    std::pair<std::pair<node, node>, cost> ret = forest.getMinEdgeOnPath(path);
    conceal(path);

    checkForestValidity();

    return ret;
}

std::pair<std::pair<node, node>, cost> LinkCutTrees::getMaxEdge(node v) {
    Path path = expose(v);
    std::pair<std::pair<node, node>, cost> ret = forest.getMaxEdgeOnPath(path);
    conceal(path);

    checkForestValidity();

    return ret;
}

void LinkCutTrees::update(node v, diff x) {
    Path path = expose(v);
    forest.update(path, x);
    conceal(path);

    checkForestValidity();
}

void LinkCutTrees::reroot(node v) {
    Path pathToRoot = expose(v);
    forest.reverse(pathToRoot);
    parent[v] = none;
    conceal(pathToRoot);

    checkForestValidity();
}

Path LinkCutTrees::splice(Path path) {
    assert(path == forest.getBBTRoot(path));

    node u = forest.getEnd(path);
    node v = parent[u];

    assert(v != none);

    SplitResult splitRes = forest.globalSplit(pathNodes[v]);
    pathNodes[v]->setWeight(pathNodes[v]->weight - path->weight);
    pathSets[v].remove({path->weight, forest.getEnd(path)});
    if(splitRes.leftTree != nullptr){
        parent[forest.getEnd(splitRes.leftTree)] = v;
        pCost[forest.getEnd(splitRes.leftTree)] = splitRes.lCost;
        pathNodes[v]->setWeight(pathNodes[v]->weight + splitRes.leftTree->weight);
        pathSets[v].insert({splitRes.leftTree->weight, forest.getEnd(splitRes.leftTree)}, splitRes.leftTree, splitRes.leftTree->weight);
    }
    path = forest.globalJoin(path, pathNodes[v], pCost[u]);
    if(splitRes.rightTree != nullptr){
        path = forest.globalJoin(path, splitRes.rightTree, splitRes.rCost);
    }

    return path;
}

Path LinkCutTrees::expose(node v) {
    SplitResult splitRes = forest.globalSplit(pathNodes[v]);

    if(splitRes.leftTree != nullptr){
        pathSets[v].insert({splitRes.leftTree->weight, forest.getEnd(splitRes.leftTree)}, splitRes.leftTree, splitRes.leftTree->weight);
        parent[forest.getEnd(splitRes.leftTree)] = v;
        pCost[forest.getEnd(splitRes.leftTree)] = splitRes.lCost;
        pathNodes[v]->setWeight(pathNodes[v]->weight + splitRes.leftTree->weight);
    }

    Path path = pathNodes[v];

    if(splitRes.rightTree != nullptr){
        path = forest.globalJoin(path, splitRes.rightTree, splitRes.rCost);
    }

    while (parent[forest.getEnd(path)] != none){
        path = splice(path);
    }

    return path;
}

Path LinkCutTrees::slice(Path path) {
    assert(path == forest.getBBTRoot(path));

    std::pair<std::pair<node, node>, diff> light = forest.getTiltedEdgeOnPath(path);
    //v is the node that should have the dashed edge entering it
    node v = light.first.second;

    SplitResult splitRes = forest.globalSplit(pathNodes[v]);
    path = splitRes.leftTree;
    Path pathUp = pathNodes[v];

    pathNodes[v]->setWeight(pathNodes[v]->weight + path->weight);

    if(pathSets[v].empty() == false) {
        //Check if another edge of v should be solid instead
        std::pair<std::pair<count, node>, Path> heavyChild = pathSets[v].getMax();
        if (heavyChild.first.first * 2 > pathNodes[v]->weight) {
            pathSets[v].remove(heavyChild.first);
            pathNodes[v]->setWeight(pathNodes[v]->weight - heavyChild.first.first);
            if (splitRes.rightTree != nullptr) {
                pathUp = forest.globalJoin(pathUp, splitRes.rightTree, splitRes.rCost);
            }
            pathUp = forest.globalJoin(heavyChild.second, pathUp, pCost[forest.getEnd(heavyChild.second)]);
        } else {
            if (splitRes.rightTree != nullptr) {
                pathUp = forest.globalJoin(pathUp, splitRes.rightTree, splitRes.rCost);
            }
        }
    }
    else if (splitRes.rightTree != nullptr) {
        pathUp = forest.globalJoin(pathUp, splitRes.rightTree, splitRes.rCost);
    }

    refreshPathEntry(pathUp);

    pathSets[v].insert({path->weight, forest.getEnd(path)}, path, path->weight);
    parent[forest.getEnd(path)] = v;
    pCost[forest.getEnd(path)] = splitRes.lCost;

    return path;
}

void LinkCutTrees::conceal(Path path) {
    //Cut out all edges in path that should be dashed
    // TODO (MK) the while check can be more efficient
    while (forest.getTiltedEdgeOnPath(path).first.first != none){
        path = slice(path);
    }

    //Now just check if the last vertex of path may need to add another solid edge
    node v = forest.getStart(path);
    if(pathSets[v].empty() == false){
        std::pair<std::pair<count, node>, Path> heavyChild = pathSets[v].getMax();
        if(heavyChild.first.first * 2 > pathNodes[v]->weight){
            pathSets[v].remove(heavyChild.first);
            SplitResult splitRes = forest.globalSplit(pathNodes[v]);
            pathNodes[v]->setWeight(pathNodes[v]->weight - heavyChild.first.first);
            if(splitRes.rightTree != nullptr){
                path = forest.globalJoin(pathNodes[v], splitRes.rightTree, splitRes.rCost);
            }
            path = forest.globalJoin(heavyChild.second, path, pCost[forest.getEnd(heavyChild.second)]);

            refreshPathEntry(path);
        }
    }
}

void LinkCutTrees::refreshPathEntry(Path p) {
    node end = forest.getEnd(p);
    if(parent[end] == none) return;
    pathSets[parent[end]].changeVal({p->weight, end}, p);
}

void LinkCutTrees::checkForestValidity() {
#ifndef NDEBUG //The following code is only executed in debug mode
    for(node i = 0; i < n; i++){
        //Test that path nodes stay legit
        assert(pathNodes[i]->v == i);
        assert(pathNodes[i]->weight == pathSets[i].getWeightSum() + 1);

        //Test if dashed edges and path sets function properly
        Path path = forest.getBBTRoot(pathNodes[i]);
        if(i == forest.getEnd(path)) {
            assert(forest.getTiltedEdgeOnPath(forest.getBBTRoot(pathNodes[i])).first.first == none);
            if(parent[i] != none){
                //Assure that this path is in the parents pathSet
                assert(pathSets[parent[i]].contains({path->weight, i}));
            }
            //Check that this path has no tiled edges of its own
            assert(forest.getTiltedEdgeOnPath(path).first.first == none);
        }
        if(i == forest.getStart(path)){
            if(pathSets[i].empty() == false) assert(pathSets[i].getMax().first.first * 2 <= pathSets[i].getWeightSum() + 1);
        }
    }
#endif //NDEBUG
}