#include <algorithm>
#include <cassert>
#include <random>
#include <iostream>
#include <deque>
#include <string>

#include "Graph.hpp"

#include "graph_generator.hpp"

constexpr int MAXRAND = 1000000000;
constexpr int MAXRANDQUERY = 1000000;

std::vector<IO::Edge> GraphGen::createRandomGraph(unsigned int n, double p, unsigned int seed) {
    assert(0 <= p && p <= 1);

    // Convert p to a integer value to make code later more readable
    unsigned int pAsInt = (int) (p * MAXRAND);

    // Seed the random engine
    std::mt19937 rng;
    rng.seed(seed);
    std::uniform_int_distribution<uint32_t> randomNumber(0, MAXRAND);

    std::vector<IO::Edge> edges;

    // Iterate over all edges
    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            // Check the chance by comparing a random value with p adjusted to int
            // values
            if (randomNumber(rng) <= pAsInt) {
                edges.emplace_back(i, j);
            }
        }
    }
    return edges;
}

std::vector<IO::Action> GraphGen::MixedTestFromGraph(std::vector<IO::Edge> &edges, double pStart, double pAddEdgeInStep,
                                                     unsigned int stepsToFinal, unsigned int seed,
                                                     double queryFrequency) {
    node n = 0;
    for (const auto &e: edges) {
        n = std::max(n, e.second);
    }

    assert(0 <= queryFrequency);
    assert(0 <= pStart && pStart <= 0.5);
    assert(0 <= pAddEdgeInStep && pAddEdgeInStep <= 1);

    // Seed the random engine
    std::mt19937 rng;
    rng.seed(seed);
    std::mt19937 rngQuery;
    rngQuery.seed(seed);
    std::uniform_int_distribution<uint32_t> randNumbLarge(0, MAXRAND - 1);
    std::uniform_int_distribution<uint32_t> randNumbQuery(0, MAXRANDQUERY);
    std::uniform_int_distribution<uint32_t> randNumbInN(0, n - 1);
    // Prepare stuff for percentage stuff
    auto pAddEdgeInStepAsInt = (unsigned int) (pAddEdgeInStep * MAXRAND);
    auto pqfAsInt = (unsigned int) (queryFrequency * MAXRANDQUERY);

    // Randomize edges
    std::shuffle(edges.begin(), edges.end(), rng);

    std::vector<IO::Action> actions;

    // Now add in the starting number of edges
    auto borderInOut = (unsigned int) (pStart * edges.size());

    for (int i = 0; i < borderInOut; i++) {
        actions.emplace_back(IO::ActionType::addAction, edges[i].first, edges[i].second);
    }

    // Now start adding and deleting edges
    for (int i = 0; i < stepsToFinal; i++) {
        // Check if an edge is added or deleted
        if (randNumbLarge(rng) < pAddEdgeInStepAsInt) {
            // Check if there are still edges to be added
            if (borderInOut < edges.size() - 1) {
                // Add edge by swapping a random edge to the first position of edges not
                // in
                std::size_t rand = (randNumbLarge(rng) % (edges.size() - borderInOut)) +
                                   borderInOut;
                std::swap(edges[borderInOut], edges[rand]);

                // Now declare that the edge has been added
                actions.emplace_back(IO::ActionType::addAction, edges[borderInOut].first,
                                     edges[borderInOut].second);
                borderInOut++;
            }
#if DEBUG
            else std::cout << "Addition of edge not possible \n";
#endif
        } else {
            // Check if there are still edges to be removed
            if (borderInOut > 0) {
                // Add edge by swapping a random edge to the first position of edges not
                // in
                std::size_t rand = (randNumbLarge(rng) % (borderInOut));
                std::swap(edges[borderInOut - 1], edges[rand]);

                // Now declare that the edge has been removed
                actions.emplace_back(IO::deleteAction, edges[borderInOut - 1].first,
                                     edges[borderInOut - 1].second);
                borderInOut--;
            }
#if DEBUG
            else std::cout << "Removal of edge not possible \n";
#endif
        }

        // Now check for queries
        // Check weather or not to add a query
        node u, w;
        unsigned int rand = randNumbQuery(rngQuery);
        //Add a bunch of queries that are on average the desired frequency
        while (rand <= pqfAsInt) {
            //if(i % 1000 == 0) std::cout << "Query added\n";
            // Get two random vertexes
            u = randNumbInN(rng);
            // Make sure the vertexes are different
            do
                w = randNumbInN(rng);
            while (w == u);

            actions.emplace_back(IO::queryAction, u, w);

            rand += MAXRANDQUERY;
        }
    }

    return actions;
}

std::vector<IO::Action> GraphGen::ActionBlockTestFromActionList(std::vector<IO::Action> &actions, count blockSize, bool timeAddActions){
    std::vector<IO::Action> addActions;
    std::vector<IO::Action> deleteActions;

    count n = 0;

    std::vector<IO::Action> returnActions;

    for(IO::Action act : actions){
        n = std::max({n, act.u, act.v});
    }
    n++;

    Graph g(n);

    if(not timeAddActions) returnActions.push_back({IO::ActionType::timerAction, 0, 0});

    for(IO::Action act : actions){
        if(act.type == IO::ActionType::addAction){
            addActions.push_back(act);
        }
        if(act.type == IO::ActionType::deleteAction){
            deleteActions.push_back(act);
        }
        if(addActions.size() >= blockSize){
            for(IO::Action addAct : addActions){
                if (not g.hasEdge(addAct.u, addAct.v)) {
                    returnActions.push_back(addAct);
                    g.addEdge(addAct.u, addAct.v);
                }
            }
            addActions.resize(0);
        }
        if(deleteActions.size() >= blockSize){
            returnActions.push_back({IO::ActionType::timerAction, 0,0});
            for(IO::Action delAct : deleteActions){
                if(g.hasEdge(delAct.u, delAct.v)){
                    returnActions.push_back(delAct);
                    g.deleteEdge(delAct.u, delAct.v);
                }
            }
            returnActions.push_back({IO::ActionType::timerAction, 0,0});
            deleteActions.resize(0);
        }
    }

    return returnActions;
}

std::vector<IO::Action> GraphGen::IncrementalTestFromGraph(std::vector<IO::Edge> &edges, unsigned int seed,
                                                           double queryFrequency) {
    assert(0 <= queryFrequency);

    unsigned int qfAsInt = (int) (queryFrequency * MAXRANDQUERY);

    // Seed the random engine
    std::mt19937 rng;
    rng.seed(seed);
    std::mt19937 rngQuery;
    rngQuery.seed(seed);
    std::uniform_int_distribution<uint32_t> randNumbLarge(0, MAXRAND);
    std::uniform_int_distribution<uint32_t> randNumbQuery(0, MAXRANDQUERY);

    node n = 0;
    for (const auto &e: edges) {
        n = std::max(n, e.second);
    }
    std::uniform_int_distribution<uint32_t> randNumbInN(
        0, n - 1); // For choosing a random Vertex

    std::shuffle(edges.begin(), edges.end(), rng);

    std::vector<IO::Action> actions;
    node u, w;
    for (IO::Edge &edge: edges) {
        actions.emplace_back(IO::addAction, edge.first, edge.second);

        // Check whether to add a query
        unsigned int rand = randNumbQuery(rngQuery);
        while (rand <= qfAsInt) {
            // Get two random different vertexes
            u = randNumbInN(rng);
            do
                w = randNumbInN(rng);
            while (w == u);

            actions.emplace_back(IO::queryAction, u, w);

            rand += MAXRANDQUERY;
        }
    }
    return actions;
}

std::vector<IO::Action> GraphGen::edgeListToActions(const std::vector<IO::Edge> &edges) {
    std::vector<IO::Action> actions;
    actions.reserve(edges.size());
    for (IO::Edge e: edges)
        actions.emplace_back(IO::addAction, e.first, e.second);
    return actions;
}

std::vector<IO::Action>
GraphGen::StaticTestFromGraph(std::vector<IO::Edge> &edges, unsigned int seed, double queryFrequency) {
    assert(0 <= queryFrequency);

    std::mt19937 rng;
    rng.seed(seed);
    std::mt19937 rngQuery;
    rngQuery.seed(seed);
    unsigned int qfAsInt = (int) (queryFrequency * MAXRANDQUERY);
    std::uniform_int_distribution<uint32_t> randNumbLarge(0, MAXRAND);
    std::uniform_int_distribution<uint32_t> randNumbQuery(0, MAXRANDQUERY);

    node n = 0;
    for (const auto &e: edges) {
        n = std::max(n, e.second);
    }
    std::uniform_int_distribution<uint32_t> randNumbInN(
        0, n - 1); // For choosing a random Vertex

    auto actions = edgeListToActions(edges);

    std::shuffle(edges.begin(), edges.end(), rng);

    node u, w;
    // Check whether to add a query
    unsigned int rand = randNumbQuery(rng);
    while (rand <= qfAsInt) {
        // Get two random vertexes
        u = randNumbInN(rng);
        // Make sure vertexes are different
        do
            w = randNumbInN(rng);
        while (w == u);

        actions.emplace_back(IO::queryAction, u, w);

        rand += MAXRANDQUERY;
    }
    return actions;
}

std::vector<IO::Action>
GraphGen::DecrementalTestFromGraph(std::vector<IO::Edge> &edges, unsigned int seed, double queryFrequency) {
    assert(0 <= queryFrequency);

    unsigned int qfAsInt = (int) (queryFrequency * MAXRANDQUERY);


    // Seed the random engine
    std::mt19937 rng;
    rng.seed(seed);
    std::mt19937 rngQuery;
    rngQuery.seed(seed);
    std::uniform_int_distribution<uint32_t> randNumbLarge(0, MAXRAND);
    std::uniform_int_distribution<uint32_t> randNumbQuery(0, MAXRANDQUERY);

    node n = 0;
    for (const auto &e: edges) {
        n = std::max(n, e.second);
    }
    auto actions = edgeListToActions(edges);

    std::uniform_int_distribution<uint32_t> randNumbInN(
        0, n - 1); // For choosing a random Vertex
    std::shuffle(edges.begin(), edges.end(), rng);

    node u, w;
    for (IO::Edge &edge: edges) {
        // Note down added edge
        actions.emplace_back(IO::deleteAction, edge.first, edge.second);

        // Check whether to add a query
        unsigned int rand = randNumbQuery(rngQuery);
        while (rand <= qfAsInt) {
            // Get two random vertexes
            u = randNumbInN(rng);
            // Make sure vertexes are different
            do
                w = randNumbInN(rng);
            while (w == u);

            actions.emplace_back(IO::queryAction, u, w);

            rand += MAXRANDQUERY;
        }
    }
    return actions;
}


std::vector<IO::Edge> GraphGen::generateDiamGraph(unsigned int n, unsigned int k) {
    unsigned int f = n / k;

    std::vector<IO::Edge> edges;
    std::pair<node, node> e0(0, 0);
    std::pair<node, node> e1(0, 0);

    for (node i = 1; i <= n; i += f) {
        e0.second = i;
        edges.push_back(e0);
        for (node j = i; j % f != 0; j++) {
            e1.first = j;
            e1.second = j + 1;
            edges.push_back(e1);
        }
    }

    return edges;
}

double GraphGen::calcAvgSp(std::vector<std::vector<node>> &adjacency_list) {
    double len_total = 0; // added length of all paths
    double count = 0;

    std::vector<char> visited;
    std::vector<unsigned int> dist;
    std::vector<node> prev;

    std::deque<node> queue;

    // run bfs once with each node as root
    for (int i = 0; i < adjacency_list.size(); i++) {
        visited = std::vector<char>(adjacency_list.size(), false);
        dist = std::vector<unsigned int>(adjacency_list.size(), 0);
        prev = std::vector<node>(adjacency_list.size(), 0);
        queue.push_back(i);

        while (not queue.empty()) {
            node cur = queue.front();
            queue.pop_front();
            visited[cur] = true;
            len_total += dist[cur];
            for (const auto &x: adjacency_list[cur]) {
                if (not visited[x]) {
                    prev[x] = cur;
                    dist[x] = dist[cur] + 1;
                    count += 1;
                    queue.push_back(x);
                }
            }
        }
    }

    const double avg_sp = len_total / count;
    return avg_sp;
}

std::vector<IO::Edge> GraphGen::generateLineGraph(unsigned int n) {
    std::vector<IO::Edge> edges(n - 1);
    for (int i = 1; i < n; i++) {
        edges[i - 1] = {i - 1, i};
    }
    return edges;
}

void GraphGen::makeBFSTestFiles(const std::string &filename, const std::string &fileSolution,
                                unsigned int n, double p, unsigned int seed) {
    assert(0 <= p && p <= 1);
    assert(n > 50);

    // Seed the random engine
    std::mt19937 rng;
    rng.seed(seed);
    std::uniform_int_distribution<uint32_t> randNumbLarge(0, 1000000000);
    std::uniform_int_distribution<uint32_t> randNumbInN(0, n - 1);
    // Prepare stuff for percentage stuff
    unsigned int pAsInt = (int) (p * MAXRAND);

    // Make vector of all vertex indexes
    std::vector<node> vertexes(n, 0);
    // initialize vertexes
    for (int i = 0; i < n; i++)
        vertexes[i] = i;
    // Randomize vertex order
    std::shuffle(vertexes.begin(), vertexes.end(), rng);

    // This vector will hold vertexes that are explicitly connected
    std::vector<IO::Edge> connected;
    std::vector<IO::Action> actions;

    // Add in Paths between random vertexes and note these down
    node u = 0, v = 0;
    while (u < n) {
        // Select part of vertexes of random length
        v = u + (randNumbLarge(rng) % (n - u)) + 1;
        // Make sure the component isn't too big
        if (v > u + n / 10)
            v = u + n / 10;
        // Make sure the component actually exists
        if (v >= n)
            break;

        // Note down, that these two vertexes at these positions are connected
        connected.emplace_back(u, v);

        // actually connect them
        for (int i = u; i < v; i++) {
            actions.emplace_back(IO::ActionType::addAction, vertexes[i], vertexes[i + 1]);
        }

        // Set u for the next connection of vertexes
        u = v + 1;
    }

    // Now add in more random edges between the paths
    for (int i = 0; i < connected.size(); i++) {
        // Add random edges from path i to path i i+2, i+4,...
        // Iterate over vertexes in the path itself
        for (node j = connected[i].first; j <= connected[i].second; j++) {
            // Add edges to the later vertexes of the path
            for (int k = j + 2; k <= connected[i].second; k++) {
                if (randNumbLarge(rng) <= pAsInt) {
                    // Write the line
                    actions.emplace_back(IO::addAction, vertexes[j], vertexes[k]);
                }
            }

            // Now do the same for the further paths in steps of 2 (so at most two
            // connectivity components exist)
            for (int j2 = i + 2; j2 < connected.size(); j2 += 2) {
                // Iterate over all vertexes in path j2
                for (node k = connected[j2].first; k <= connected[j2].second; k++) {
                    if (randNumbLarge(rng) <= pAsInt) {
                        // Write the line
                        actions.emplace_back(IO::addAction, vertexes[j], vertexes[k]);
                    }
                }
            }
        }
    }

    // Now all edges are added
    // Start adding queries
    // First all the solvable queries will be added. Then an equal number of
    // unsolvable queries are added
    for (auto &e: connected) {
        actions.emplace_back(IO::queryAction, vertexes[e.first], vertexes[e.second]);
    }
    // Add unsolvable queries (paths with even index and odd index aren't
    // connected)
    actions.emplace_back(IO::queryAction, vertexes[connected[0].first], vertexes[connected[1].second]);
    for (int i = 1; i < connected.size(); i++) {
        actions.emplace_back(IO::queryAction, vertexes[connected[i].first], vertexes[connected[i - 1].second]);
    }

    std::stringstream ss;
    ss << "solutionFile: " << fileSolution
       << " n: " << n << " p: " << p << " seed: " << seed << "\n";
    IO::actionListToFile(actions, filename, ss.str());

    // Start writing the solutions
    std::ofstream fileWriter(fileSolution);
    for (int i = 0; i < connected.size(); i++)
        fileWriter << "1\n";
    for (int i = 0; i < connected.size(); i++)
        fileWriter << "0\n";
}

std::vector<IO::Action> GraphGen::ChordalTestFromGraph(std::vector<IO::Edge> &edges, unsigned int seed) {
    throw std::runtime_error("not implemented yet");
    /*
    auto actions = edgeListToActions(edges);
    graph_simple g(edges);
    for (node v = 0; v < g.getNumberOfNodes(); v++) {
        for (node w: g.getNeighbors(v)) {
            actions.emplace_back(IO::deleteAction, v, w);
            actions.emplace_back(IO::queryAction, v, w);
            for (node u: g.getNeighbors(w)) {
                actions.emplace_back(IO::deleteAction, w, u);
            }
        }
        for (node w: g.getNeighbors(v)) {
            for (node u: g.getNeighbors(w)) {
                if (u > v) {
                    actions.emplace_back(IO::addAction, w, u);
                }
            }
        }
    }
     */
    //return actions;
}

std::vector<IO::Edge> GraphGen::starLineTest(const count n) {
    std::vector<IO::Edge> edges;
    for (node i = 0; i < n / 2; i++)
        edges.emplace_back(n / 2, i);
    for (node i = n / 2 + 1; i < n; i++)
        edges.emplace_back(i - 1, i);
    return edges;
}