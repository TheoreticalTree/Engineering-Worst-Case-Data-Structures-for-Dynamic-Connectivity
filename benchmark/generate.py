import graphPortChen
import randomGraphClusters

names = ["dnc", "call", "messages", "fb", "wiki", "tech", "enron"#,
#	"stackoverflow", "youtube"
        ]

queryFrequencies = [0, #0.1, 1.0, 10.0,# 100.0
                    ]

if __name__ == '__main__':
    for name in names:
        for queryFrequency in queryFrequencies:
            print(name + "_" + str(queryFrequency) + ".inst")
            graphPortChen.generateInstance(name, queryFrequency)
        print(name + "_" + "bq" + ".inst")
        graphPortChen.generateInstance(name, 0, True)

    for n in [1000, 10000, 100000]:
        for s in [42, 666]:
            randomGraphClusters.generateRandomClusterGraph(n, 100, 10000000, s,
                                                           "instances/cliques_small_" + str(n) + "_100_" + str(s) + ".inst",
                                                           "instances/cliques_small_" + str(n) + "_100_" + str(s) + "_bq.inst")
            print("instances/cliques_small_" + str(n) + "_100_" + str(s) + ".inst")
            print("instances/cliques_small_" + str(n) + "_100_" + str(s) + "_bq.inst")

    for n in [1000, 10000, 100000]:
        for s in [42, 666]:
            randomGraphClusters.generateRandomClusterGraph(n, int(n/100), 10000000, s,
                                                           "instances/cliques_large_" + str(n) + "_" + str(int(n/100)) + "_" + str(s) + ".inst",
                                                           "instances/cliques_large_" + str(n) + "_" + str(int(n/100)) + "_" + str(s) + "_bq.inst")
            print("instances/cliques_large_" + str(n) + "_" + str(int(n/100)) + "_" + str(s) + ".inst")
            print("instances/cliques_large_" + str(n) + "_" + str(int(n/100)) + "_" + str(s) + "_bq.inst")
