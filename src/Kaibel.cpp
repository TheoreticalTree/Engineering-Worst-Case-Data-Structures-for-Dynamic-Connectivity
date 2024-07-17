#include "Kaibel.hpp"

#include <cmath>

Kaibel::Kaibel(Graph &G, float c, uint seed, count boostLevel, Mode mode) : n(G.getN()), c(c), boostLevel(boostLevel) {
    switch (mode) {
        case pHeu:
            p = (1 - std::pow(0.5, boostLevel));
            break;
        default:
            p = (1 - std::pow(1 - precision, boostLevel));
    }

    switch (mode) {
        case lvlHeu:
            numLevels = std::ceil(4*c*log2(n));
            break;
        default:
            numLevels = std::ceil(std::max(2*std::ceil(log2(n)/log2(4/(4 - p)))*(1 - p/2)/(1 - p), 8*c*log2(n) * p * (1 - p/2)/(1 - p)));
            break;
    }

    cutSet = std::make_unique<CutSet>(n, boostLevel, numLevels, seed, adjacencyTrees);
    linkCutTrees = std::make_unique<LinkCutTrees>(n);
    queryForest = std::make_unique<QueryForestAVL>(n);

    adjacencyTrees.resize(n);

    for(node u = 0; u < n; u++){
        for(node v : G.getNeighbors(u)){
            if(v > u) addEdge(u, v);
        }
    }
}

Kaibel::Kaibel(count n, float c, uint seed, count boostLevel, Mode mode) : n(n), c(c), boostLevel(boostLevel) {
    switch (mode) {
        case pHeu:
            p = (1 - std::pow(0.5, boostLevel));
            break;
        default:
            p = (1 - std::pow(1 - precision, boostLevel));
    }

    switch (mode) {
        case lvlHeu:
            numLevels = std::ceil(4*c*log2(n));
            break;
        default:
            numLevels = std::ceil(std::max(2*std::ceil(log2(n)/log2(4/(4 - p)))*(1 - p/2)/(1 - p), 8*c*log2(n) * p * (1 - p/2)/(1 - p)));
            break;
    }

    cutSet = std::make_unique<CutSet>(n, boostLevel, numLevels, seed, adjacencyTrees);
    linkCutTrees = std::make_unique<LinkCutTrees>(n);
    queryForest = std::make_unique<QueryForestAVL>(n);

    adjacencyTrees.resize(n);
}

bool Kaibel::query(node u, node v) {
    return queryForest->query(u, v);
}

count Kaibel::numberOfComponents() {
    return queryForest->numberOfComponents();
}

void Kaibel::addEdge(node u, node v) {
    adjacencyTrees[u].insert(v, false);
    adjacencyTrees[v].insert(u, false);

    cutSet->addEdgeToSet({u, v});

    if(not queryForest->query(u, v)) {
        queryForest->addEdge(u, v);
        linkCutTrees->link(u, v, 0);
        cutSet->makeTreeEdge({u, v}, 0);
    }

    sanityCheck();
}

void Kaibel::deleteEdge(node u, node v) {
    adjacencyTrees[u].remove(v);
    adjacencyTrees[v].remove(u);

    cutSet->deleteEdge({u, v});
    if(queryForest->isTreeEdge(u, v)) {
        queryForest->deleteEdge(u, v);
        linkCutTrees->cutEdge(u, v);

        refreshTrees(u, v);
    }

    sanityCheck();
}

void Kaibel::refreshTrees(node u, node v) {
    bool separate = true;

    //All layers except the last one only operate within the CutSet data structure
    for(count i = 0; i < numLevels - 1; i++){
        if(cutSet->compSize(u, i) == cutSet->compSize(u, i+1)){
            searchAndInsert(u, i);
        }
        if(separate && (cutSet->compSize(v, i) == cutSet->compSize(v, i+1))){
            searchAndInsert(v, i);
        }
        //Check if the components of u and v are now one and we only have to search once per level
        if (separate)
            separate = cutSet->compRepresentative(u, i+1) != cutSet->compRepresentative(v, i+1);
    }

    //The last layer must be handled on its own
    if(cutSet->compSize(u, numLevels - 1) == queryForest->compSize(u)){
        searchAndInsert(u, numLevels - 1);
    }
    if(separate && (cutSet->compSize(v, numLevels - 1) == queryForest->compSize(v))){
        searchAndInsert(v, numLevels - 1);
    }
}

void Kaibel::searchAndInsert(node v, count level) {
    edge e = cutSet->search(v, level);
    if(e != noEdge){
        //New edge to be inserted found: Check if insertion would create a circle higher up
        if(queryForest->query(e.v, e.w)){
            linkCutTrees->reroot(e.v);
            std::pair<std::pair<node, node>, cost> lowestEdge = linkCutTrees->getMaxEdge(e.w);
            //Remove the edge that closes the circle that is inserted at the highest level
            cutSet->makeNonTreeEdge({lowestEdge.first.first, lowestEdge.first.second});
            linkCutTrees->cut(lowestEdge.first.first);
            queryForest->deleteEdge(lowestEdge.first.first, lowestEdge.first.second);
        }

        //Now actually insert the edge on all layers
        cutSet->makeTreeEdge(e, level+1);
        queryForest->addEdge(e.v, e.w);
        linkCutTrees->link(e.v, e.w, level+1);
    }
}

void Kaibel::sanityCheck() {
#ifndef NDEBUG

#endif //NDEBUG
}