#include "CutSet.hpp"

CutSet::CutSet(count pn, count pBoostLevel, count pNumLevels, count seed, const std::vector<AVLTree<node, bool>> &pAdjacencyTrees): adjacencyTrees(pAdjacencyTrees) {
    n = pn;
    boostLevel = pBoostLevel;
    numLevels = pNumLevels;
    lognsqr = std::ceil(2.0 * std::log2(n)) + 1;

    std::mt19937 rng(seed);
    std::uniform_int_distribution<count> dis(0, std::numeric_limits<count>::max());

    treeEdges.resize(numLevels);
    activeEdges.resize(numLevels);
    forests.resize(numLevels, {boostLevel, n});
    xOrEdgeVectors.resize(numLevels);
    hashes.resize(numLevels);

    for(count i = 0; i < numLevels; i++){
        treeEdges[i].resize(n);
        activeEdges[i].resize(n, nullptr);
        xOrEdgeVectors[i].resize(n);
        hashes[i].reserve(boostLevel);
        for(count j = 0; j < boostLevel; j++) hashes[i].emplace_back(n, dis(rng), blockSize);

        for(count j = 0; j < n; j++){
            xOrEdgeVectors[i][j].resize(boostLevel);
            for(count k = 0; k < boostLevel; k++){
                xOrEdgeVectors[i][j][k].resize(lognsqr, {0,0});
            }
        }
    }
}

CutSet::~CutSet() {
    //Ensure that every tree edge is properly deleted
    for(count i = 0; i < numLevels; i++){
        for(node v = 0; v < n; v++){
            std::vector<std::pair<node, address>> killList = treeEdges[i][v].getSortedSet();
            for(std::pair<node, address> e : killList) delete e.second;
        }
    }
}

edge CutSet::search(node v, count level) {
    std::vector<std::vector<edge>> accumulatedEdges;

    if(activeEdges[level][v] == nullptr){
        accumulatedEdges = xOrEdgeVectors[level][v];
    }
    else {
        accumulatedEdges = forests[level].getTrackingData(forests[level].getRoot(activeEdges[level][v])).accumulatedData;
    }

    node vRoot = compRepresentative(v, level);

    //Try through all the various boost level
    for(count i = 0; i < boostLevel; i++){
        //Check the levels from sparce to dense
        for(count j = 0; j < lognsqr; j++){
            if(accumulatedEdges[i][j] != noEdge){
                //If the edge isn't {0,0} check if its a valid solution
                edge candidate = accumulatedEdges[i][j];
                //Trivial invalidity check
                if(candidate.v >= n || candidate.w >= n) break;
                //exactly one of the endpoints has to be in the same component as v and the edge has to be real
                if(adjacencyTrees[candidate.v].contains(candidate.w) && ((compRepresentative(candidate.v, level) == vRoot) != (compRepresentative(candidate.w, level) == vRoot))){
                    //This edge is real and leaves the component of v
                    return candidate;
                }
                else break;
            }
        }

    }

    return noEdge;
}

void CutSet::deleteEdge(edge e) {
    if(treeEdges[numLevels - 1][e.v].contains(e.w)) makeNonTreeEdge(e);

    //Since for bitwise xor addition and subtraction is the same we can just use addToXor to also remove it
    addEdgeToSet(e);
}

void CutSet::makeNonTreeEdge(edge e) {
    //Ensure that it's actually a tree edge
    assert(treeEdges[numLevels - 1][e.v].contains(e.w) && treeEdges[numLevels - 1][e.w].contains(e.v));

    //Move down the levels and remove the edges
    for(int i = numLevels - 1; i >= 0; i--){
        if(treeEdges[i][e.v].contains(e.w)){
            address edge = treeEdges[i][e.v].remove(e.w), backEdge = treeEdges[i][e.w].remove(e.v);
            forests[i].deleteETEdge(edge, backEdge);

            //If either of the deleted edges was an active edge replace it
            if(activeEdges[i][e.v] == edge) refreshActiveInstance(i, e.v);
            if(activeEdges[i][e.w] == backEdge) refreshActiveInstance(i, e.w);
        }
        else break;
    }
}

void CutSet::refreshActiveInstance(count level, node v){
    if(treeEdges[level][v].empty()) activeEdges[level][v] = nullptr;
    else {
        activeEdges[level][v] = treeEdges[level][v].getAnyEntry().second;
        forests[level].setTrackingData(activeEdges[level][v], &(xOrEdgeVectors[level][v]));
    }
}

void CutSet::makeTreeEdge(edge e, count level) {
    for(count i = level; i < numLevels; i++){
        assert(compRepresentative(e.v, i) != compRepresentative(e.w, i));

        //Insert the edge
        address vEdge = activeEdges[i][e.v], wEdge = activeEdges[i][e.w];
        std::pair<address, address> newTreeEdges = forests[i].insertETEdge(e.v, e.w, vEdge, wEdge);

        //Store the new tree edges
        treeEdges[i][e.v].insert(e.w, newTreeEdges.first);
        treeEdges[i][e.w].insert(e.v, newTreeEdges.second);

        //Check if these edges are the new active instances
        if(vEdge == nullptr) {
            activeEdges[i][e.v] = newTreeEdges.first;
            forests[i].setTrackingData(newTreeEdges.first, &(xOrEdgeVectors[i][e.v]));
        }
        if(wEdge == nullptr) {
            activeEdges[i][e.w] = newTreeEdges.second;
            forests[i].setTrackingData(newTreeEdges.second, &(xOrEdgeVectors[i][e.w]));
        }
    }
}

void CutSet::addEdgeToSet(edge e) {
    assert(e.v < n && e.w < n && e.v != e.w);
    if(e.v >= e.w) e = {e.w, e.v};

    for(count i = 0; i < numLevels; i++){
        std::vector<uint8_t> startingLevel(boostLevel, 0);

        for(count j = 0; j < boostLevel; j++){
            count hashVal = hashes[i][j].hash(e);
            //Stores 2^k
            count binPotk = 1;
            //Skip all the xOrLevels that e doesn't get inserted into
            while (hashVal >= binPotk){
                startingLevel[j]++;
                binPotk *= 2;
            }

            //e will get inserted into all levels from this point on
            for(uint8_t k = startingLevel[j]; k < lognsqr; k++){
                xOrEdgeVectors[i][e.v][j][k] ^= e;
                xOrEdgeVectors[i][e.w][j][k] ^= e;
            }
        }

        //Inform the etForest that some edges were added
        if(activeEdges[i][e.v] != nullptr) forests[i].addEdgeToData(activeEdges[i][e.v], e, startingLevel);
        if(activeEdges[i][e.w] != nullptr) forests[i].addEdgeToData(activeEdges[i][e.w], e, startingLevel);
    }
}

node CutSet::compRepresentative(node v, count level) {
    if(activeEdges[level][v] == nullptr) return v;
    return forests[level].getRoot(activeEdges[level][v])->getV();
}

count CutSet::compSize(node v, count level) {
    if(activeEdges[level][v] == nullptr) return 1;
    return forests[level].getRoot(activeEdges[level][v])->getSize();
}