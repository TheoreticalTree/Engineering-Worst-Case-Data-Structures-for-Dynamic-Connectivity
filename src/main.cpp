#include <iostream>
#include <vector>
#include <random>
#include <fstream>

#include "PrimitiveStructures/RootedTree.hpp"
#include "LinkCutTrees.hpp"

#include "GKKT.hpp"

enum ActionType { Link, Cut, CutEdge, Reroot, Update, Getmin, Getmax};

struct Action {
    ActionType type;
    node u,v;
    diff x;
};

int main(int argc, char** argv){
    count n = 1000;
    count numLevels = 1000;

    std::fstream outFile("../benchmark/results/experiments_cutSet.csv");
    outFile << "size,probability\n";

    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dis(0, n);

    std::vector<AVLTree<node, bool>> adjacencyTrees(n);

    CutSet cutSet(n, 1, numLevels, 42, adjacencyTrees);

    std::vector<std::pair<node, node>> edges;

    for (node v = 0; v < n/2; v++) {
        for (node w = n/2 + 1; w < n; w++) {
            edges.emplace_back(v, w);
        }
    }

    for (count i = 0; i < edges.size(); i++) {
        std::swap(edges[i], edges[i + (dis(rng) % (edges.size() - i))]);
    }

    for (node v = 0; v < n/2 - 1; v++) {
        cutSet.makeTreeEdge({v, v+1}, 0);
    }
    for (node w = n/2 + 1; w < n-1; w++) {
        cutSet.makeTreeEdge({w, w+1}, 0);
    }

    count i = 0;

    for (std::pair<node, node> edge : edges) {
        cutSet.addEdgeToSet({edge.first, edge.second});
        adjacencyTrees[edge.first].insert(edge.second, false);
        adjacencyTrees[edge.second].insert(edge.first, false);

        i++;
        if (i % n == 0 || i < n) {
            count successes = 0;
            for (count j = 0; j < numLevels; j++) {
                if (cutSet.search(0, j) != noEdge)
                    successes++;
            }

            outFile << i << "," << static_cast<double>(successes) / static_cast<double>(numLevels) << "\n";

            std::cout << "Percentage of successes on cutSet with size " << i << " is " << static_cast<double>(successes) / static_cast<double>(numLevels) << " (" << successes << "/" << numLevels <<  ")\n";
        }
    }


    /*
    count n = 1000000;

    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dis(0, n);

    //Step 1: Generate a sequence of inserts and deletes
    std::vector<Action> actionList;
    RootedForest primitive(n);
    LinkCutTrees linkCutTrees(n);

    count u, v;
    for(count i = 0; i < (999*n/1000); i++){
        do{
            u = dis(rng) % n;
            v = dis(rng) % n;
        } while(primitive.getRoot(u) == primitive.getRoot(v));
        u = primitive.getRoot(u);
        cost x = dis(rng) % n;
        primitive.link(u,v,x);
        actionList.push_back({Link, u, v, x});
        if(i % (n/100) == 0){
            std::cout << i << " done\n";
        }
    }

    std::cout << "Starting edges found\n";

    for(count i = 0; i < 10*n; i++){
        //Insert one random edge
        do {
            u = dis(rng) % n;
            v = dis(rng) % n;
        } while(primitive.getRoot(u) == primitive.getRoot(v));
        u = primitive.getRoot(u);
        cost x = dis(rng) % n;
        primitive.link(u,v,x);
        actionList.push_back({Link, u, v, x});

        //Now delete one random edge
        do u = dis(rng) % n; while(primitive.getParent(u) == none);
        //u now has a parent, so we separate the family
        //Flip a coin if cut or cutEdge is used
        if(dis(rng) % 2 == 0){
            primitive.cut(u);
            actionList.push_back({Cut, u, 0, 0});
        }
        else {
            v = primitive.getParent(u);
            if(dis(rng) % 2 == 0) std::swap(u,v);
            primitive.cutEdge(u, v);
            actionList.push_back({CutEdge, u, v, 0});
        }

        //Now do a random reroot
        u = dis(rng) % n;
        primitive.reroot(u);
        actionList.push_back({Reroot, u, 0, 0});

        u = dis(rng) % n;
        actionList.push_back({Getmin, u, 0, 0});
        actionList.push_back({Getmax, u, 0, 0});
        cost minCost = primitive.getMinEdge(u).second;
        cost maxCost = primitive.getMaxEdge(u).second;
        diff xUpdate = (dis(rng) % (n - maxCost + minCost)) - minCost;
        primitive.update(u, xUpdate);
        actionList.push_back({Update, u, 0, xUpdate});

        if(i % (n/10) == 0){
            std::cout << i << " done\n";
        }
    }

    std::cout << "Test generated\n";

    primitive = RootedForest(n);

    auto t0 = clock();
    for(Action act : actionList){
        if(act.type == Link) primitive.link(act.u, act.v, act.x);
        if(act.type == Cut) primitive.cut(act.u);
        if(act.type == CutEdge) primitive.cutEdge(act.u, act.v);
        if(act.type == Reroot) primitive.reroot(act.u);
        if(act.type == Getmin) primitive.getMinEdge(act.u);
        if(act.type == Getmax) primitive.getMaxEdge(act.u);
        if(act.type == Update) primitive.update(act.u, act.x);
    }
    auto t1 = clock();

    std::cout << "Primitive ran in time: " << ((t1 - t0)*1000/CLOCKS_PER_SEC) << "ms\n";

    t0 = clock();
    for(Action act : actionList){
        if(act.type == Link) linkCutTrees.link(act.u, act.v, act.x);
        if(act.type == Cut) linkCutTrees.cut(act.u);
        if(act.type == CutEdge) linkCutTrees.cutEdge(act.u, act.v);
        if(act.type == Reroot) linkCutTrees.reroot(act.u);
        if(act.type == Getmin) linkCutTrees.getMinEdge(act.u);
        if(act.type == Getmax) linkCutTrees.getMaxEdge(act.u);
        if(act.type == Update) linkCutTrees.update(act.u, act.x);
    }
    t1 = clock();

    std::cout << "LinkCut ran in time: " << ((t1 - t0)*1000/CLOCKS_PER_SEC) << "ms\n";
     */
}