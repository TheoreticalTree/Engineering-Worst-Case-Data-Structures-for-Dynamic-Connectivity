import sys
import os
import time, datetime
from random import randint

import requests
import zipfile
import gzip
import io

from _collections import defaultdict

path = "instances_raw/"
fileNames = {'dnc':'email-dnc.edges',
             'call':'ia-reality-call.edges',
             'messages':'fb-messages.edges',
             'fb':'fb-forum.edges',
             'wiki':'soc-wiki-elec.edges',
             'tech':'tech-as-topology.edges',
             'enron':'ia-enron-email-dynamic.edges',
             'youtube':'youtube-growth-sorted.txt',
             'stackoverflow':'sx-stackoverflow.txt'}
fileURLs = {'dnc':'https://nrvis.com/download/data/dynamic/email-dnc.zip',
            'call':'https://nrvis.com/download/data/dynamic/ia-reality-call.zip',
            'messages':'https://nrvis.com/download/data/dynamic/fb-messages.zip',
            'fb':'https://nrvis.com/download/data/dynamic/fb-forum.zip',
            'wiki':'https://nrvis.com/download/data/dynamic/soc-wiki-elec.zip',
            'tech':'https://nrvis.com/download/data/dynamic/tech-as-topology.zip',
            'enron':'https://nrvis.com/download/data/dynamic/ia-enron-email-dynamic.zip',
            'youtube':'http://socialnetworks.mpi-sws.mpg.de/data/youtube-u-growth.txt.gz',
            'stackoverflow':'http://snap.stanford.edu/data/sx-stackoverflow.txt.gz'}

def getGraph(instance):
    if not os.path.isfile("instances_raw/" + fileNames[instance]):
        #print("downloading instance", instance, "from", fileURLs[instance])
        r = requests.get(fileURLs[instance], allow_redirects=True, stream=True)
        if fileURLs[instance].endswith('.zip'):
            z = zipfile.ZipFile(io.BytesIO(r.content))
            z.extract(member=fileNames[instance], path='instances_raw/')
        elif fileURLs[instance].endswith('.gz'):
            f = open(file=path + fileNames[instance], mode='w')
            f.write(gzip.decompress(r.content).decode('utf-8'))
        #print("download complete")

def loadGraph(testcase):
    edges = []
    numNodes = 0
    if testcase == "test":
        lines = open(path + testcase, mode='r', encoding='utf-8-sig').readlines()
        for line in lines:
            items = line.rstrip().split(',')
            if items[0] == items[1]:
                continue
            t = int(items[2])
            u, v = order(int(items[0]), int(items[1]))
            edges.append([u, v, t])
        edges.sort(key=lambda x: x[2])
    elif testcase == "stackoverflow":
        getGraph(testcase)
        lines = open(path + fileNames[testcase], 'r').readlines()
        for line in lines:
            items = line.rstrip().split()
            if items[0] == items[1]:
                continue
            u, v = order(int(items[0]), int(items[1]))
            edges.append([u, v, int(items[2])])
        edges.sort(key = lambda x: x[2])
    elif testcase == 'dnc':
        getGraph(testcase)
        lines = open(path + fileNames[testcase], mode='r', encoding='utf-8-sig').readlines()
        for line in lines:
            items = line.rstrip().split(',')
            if items[0] == items[1]:
                continue
            t = int(items[2])
            u, v = order(int(items[0]), int(items[1]))
            edges.append([u, v, t])
        edges.sort(key = lambda x: x[2])
    elif testcase == 'enron':
        getGraph(testcase)
        lines = open(path + fileNames[testcase], 'r').readlines()
        for line in lines[1:]:
            items = line.rstrip().split()
            if items[0] == items[1]:
                continue
            t = int(items[3])
            u, v = order(int(items[0]), int(items[1]))
            edges.append([u, v, t])
        edges.sort(key = lambda x: x[2])
    elif testcase == 'youtube':
        getGraph(testcase)
        lines = open(path + fileNames[testcase], 'r').readlines()
        for line in lines:
            items = line.rstrip().split('\t')
            if items[0] == items[1]:
                continue
            u, v = order(int(items[0]), int(items[1]))
            edges.append([u, v, int(time.mktime(datetime.datetime.strptime(items[2], "%Y-%m-%d").timetuple()))])
        edges.sort(key = lambda x: x[2])
    elif testcase == 'tech':
        getGraph(testcase)
        lines = open(path + fileNames[testcase], 'r').readlines()
        for line in lines:
            if line.startswith('%'):
                continue
            items = line.rstrip().split()
            if items[0] == items[1]:
                continue
            u, v = order(int(items[0]), int(items[1]))
            edges.append([u, v, int(items[3])])
        edges.sort(key = lambda x: x[2])
    elif testcase == 'wiki':
        getGraph(testcase)
        lines = open(path + fileNames[testcase], 'r').readlines()
        lines.pop(0)
        for line in lines:
            items = line.rstrip().split(" ")
            if items[0] == items[1]:
                continue
            u, v = order(int(items[0]), int(items[1]))
            edges.append([u, v, int(items[3])])
        edges.sort(key = lambda x: x[2])
    elif testcase in ['fb', 'messages', 'call']:
        getGraph(testcase)
        lines = open(path + fileNames[testcase], 'r').readlines()
        for line in lines:
            items = line.rstrip().split(',')
            if items[0] == items[1]:
                continue
            u, v = order(int(items[0]), int(items[1]))
            edges.append([u, v, int(float(items[2]))])
        edges.sort(key = lambda x: x[2])
    elif testcase == 'test1':
        edges = [(1, 2, 0), (1, 3, 0), (1, 4, 0), (2, 3, 0), (3, 4, 0), (2, 5, 0), (3, 5, 0), (4, 5, 0)]

    for edge in edges:
        numNodes = max(numNodes, edge[0], edge[1])

    return edges, numNodes + 1

def setup(testcase, start_timestamp, end_timestamp):
    # setup surviving time for different datasets
    survival_time = 1296000  # by default, 14 days

    if testcase in ['dblp', 'scholar']:
        survival_time = 5  # 5 years
    elif testcase == 'osmswitzerland':
        survival_time = 1
    elif testcase == 'test':
        survival_time = 3

    # First, set up test_num, the number of tests of performance are conducted
    # Second, calculate test_query_frequency = (t_e - t_s) / test_num.
    # that is how frequent we test queries performance.
    # For example, test_query_frequency = 1000000, we test every 1000000 seconds.
    if testcase in ['dblp', 'scholar']:
        test_query_frequency = 1  # run performance test once per year
        test_query_num = end_timestamp - start_timestamp + survival_time  # from year 1980 to year 2021
    else:
        test_query_num = 100  # select 100 test point, calculate the frequency of testing
        test_query_frequency = (end_timestamp - start_timestamp + 2 * survival_time) // test_query_num

    test_points = []
    for i in range(1, test_query_num + 1):
        test_points.append(start_timestamp + i * test_query_frequency)

    if testcase == 'scholar':
        test_points.append(2028)

    # return survival_time, test_points, query_writer, Sd_writer
    return survival_time, test_points

def order(a, b):
    if a < b:
        return a, b
    else:
        return b, a

def generateInstance(testcase, queryFrequency, blockQueries = False):
    records, n = loadGraph(testcase)

    outFile = "instances/" + testcase + "_" + str(queryFrequency) + ".inst"
    if(blockQueries):
        outFile = "instances/" + testcase + "_bq.inst"

    output = open(outFile, "w")
    output.write("c file: " + testcase + " " + str(queryFrequency) + "\n")
    output.write("c Chen-Test: " + testcase + "\n")
    if blockQueries:
        output.write("c blockQueries\n")
        output.write("t\n")

    # setup starting point and ending point
    start_timestamp = records[0][2]
    end_timestamp = records[-1][2]

    # As described in the paper, we slightly change the start_timestamp
    # (and end_timestamp), which includes almost all edges
    if testcase == 'dblp':
        start_timestamp = 1980
    elif testcase == 'dnc':
        start_timestamp = 1423298633
    elif testcase == 'enron':
        start_timestamp = 915445260
        end_timestamp = 1040459085

    #  setups
    survival_time, test_points = setup(testcase, start_timestamp, end_timestamp)

    test_points = test_points[25:75]

    if testcase == "test":
        test_points = [0, 10]

    # start from an empty graph
    idx = 0
    max_priority = sys.maxsize
    graph = defaultdict(set)

    expiredDict = defaultdict(set)
    inserted_edge = defaultdict()

    v_set = set()
    edges_num = 0
    current_time = start_timestamp
    count_snapshot = 0

    num_life_increaces = 0

    # while current_time <= end_timestamp + survival_time:
    while current_time <= end_timestamp + survival_time:
        # loop records and start with the record with current_time
        while idx < len(records) and records[idx][2] < current_time:
            idx += 1
        while idx < len(records) and records[idx][2] == current_time:
            # filter out (v, v) edges
            if records[idx][0] == records[idx][1]:
                idx += 1
                continue

            a, b = order(records[idx][0], records[idx][1])
            v_set.add(a)
            v_set.add(b)

            idx += 1
            if (a, b) not in inserted_edge:  # a new edge
                inserted_edge[(a, b)] = current_time + survival_time  # we keep the expired time for the inserted edge.
                expiredDict[current_time + survival_time].add((a, b))
                output.write("a " + str(a) + " " + str(b) + "\n")
                #Slap in queries as appropriate
                rand = randint(0, 999)
                while rand < queryFrequency*1000:
                    rand += 1000
                    u = randint(0, n-1)
                    v = randint(0, n-1)
                    output.write("q " + str(u) + " " + str(v) + "\n")
            else:  # re-insert this edge, refresh the expired timestamp
                num_life_increaces += 1
                expired_ts = inserted_edge[(a, b)]
                expiredDict[expired_ts].remove((a, b))
                inserted_edge[
                    (a, b)] = current_time + survival_time  # we refresh the expired time for the inserted edge.
                expiredDict[current_time + survival_time].add((a, b))

        if current_time in expiredDict:
            for (a, b) in expiredDict[current_time]:
                del inserted_edge[(a, b)]
                output.write("d " + str(a) + " " + str(b) + "\n")
                edges_num -= 1
                #Slap in queries as appropriate
                rand = randint(0, 999)
                while rand < queryFrequency*1000:
                    rand += 1000
                    u = randint(0, n-1)
                    v = randint(0, n-1)
                    output.write("q " + str(u) + " " + str(v) + "\n")

            del expiredDict[current_time]

        if current_time in test_points:
            if blockQueries:
                output.write("b\n")
        current_time += 1

    output.close()