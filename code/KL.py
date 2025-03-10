import argparse
import numpy as np
import random
from collections import deque

class KL_Algorithm:
    # Constant Global Configuration Variables
    PARTITION_NUMBER = 2
    PARAMETERS_NUMBER = 3

    def __init__(self, file_path=""):
        self.file_path = file_path

        # graph attributes
        self.vertices = set()
        self.graph = dict()

        # result records
        self.partitions = list()
        self.cutsizes = list()
        self.max_gains = list()
        self.max_swaps = list()


    def gain(self, x, y, p1, p2):
        Ex = sum(w for v, w in self.graph[x].items() if v in p2)
        Ix = sum(w for v, w in self.graph[x].items() if v in p1)
        Ey = sum(w for v, w in self.graph[y].items() if v in p1)
        Iy = sum(w for v, w in self.graph[y].items() if v in p2)
        Cxy = self.graph[x].get(y, 0)

        return Ex - Ix + Ey - Iy - 2*Cxy
    

    def cutsize(self, p1, p2):
        cutsize_sum = 0
        for v1 in p1:
            if v1 in self.graph:
                for v2, weight in self.graph[v1].items():
                    if v2 in p2:
                        cutsize_sum += weight

        return cutsize_sum
    

    def initialize(self):
        try:
            with open(self.file_path, "r", encoding="utf-8") as input_file:
                for i, line in enumerate(input_file):
                    parts = line.split()

                    # initialize vertices and partitions
                    if i == 0:
                        if len(parts) != KL_Algorithm.PARTITION_NUMBER:
                            print(f"Parition error: The inital parition is not equal for {self.initial_partition}")
                            break
                        set1, set2 = set(parts[0].strip()), set(parts[1].strip())
                        self.vertices = set1.union(set2)
                        self.partitions.append((set1, set2))    # tuple up set1 and set2

                        # initialize graph with dict()
                        for v in self.vertices:
                            self.graph[v] = dict()
                        continue
                    
                    # construct a bidirectional graph
                    if len(parts) != KL_Algorithm.PARAMETERS_NUMBER:
                        print(f"The {i}th line has incorrected format in the file {input_file}")
                        continue
                    self.graph[parts[0]][parts[1]] = float(parts[2])
                    self.graph[parts[1]][parts[0]] = float(parts[2])
                
                # caculate the initial cutsize after the graph has been initialized
                p1, p2 = self.partitions[0]
                self.cutsizes.append(self.cutsize(p1, p2))

        except Exception as e:
            print(f"Error reading file: {e}")


    def sub_initialize(self, vertices, graph):
        # record the vertices
        self.vertices = vertices

        # initialize the graph
        for vertice in self.vertices:
            if vertice in graph.items():
                self.graph[vertice] = graph[vertice]
        
        # random select initial partitions
        vers = list(vertices)
        random.shuffle(vers)
        split_index = int(len(vers) / KL_Algorithm.PARTITION_NUMBER)
        set1, set2 = set(vers[:split_index]), set(vers[split_index:])
        self.partitions.append((set1, set2))

        # record the initial cutsizes
        self.cutsizes.append(self.cutsize(set1, set2))


    def partitionize(self):
        # initialized unlocked vertices and partitions
        unlocked_vertices = self.vertices.copy()
        p1, p2 = self.partitions[0]

        while unlocked_vertices:
            gains, swap_pairs = list(), list()
            for x in p1:
                if x in unlocked_vertices:
                    for y in p2:
                        if y in unlocked_vertices: 
                            gains.append(self.gain(x, y, p1, p2))
                            swap_pairs.append((x, y))   # tuple up x, y

            # find max_gain and corresponding swap_pairs
            gains = np.array(gains) # convert to np array 
            max_gain = np.max(gains)
            max_gain_indices = np.where(gains == max_gain)[0]
            max_gain_swap_pairs = [swap_pairs[index] for index in max_gain_indices]
            swap_vertice_1, swap_vertice_2 = min(max_gain_swap_pairs)

            # swap two vertices
            p1.remove(swap_vertice_1)
            p1.add(swap_vertice_2)
            p2.remove(swap_vertice_2)
            p2.add(swap_vertice_1)

            # lock two vertices
            unlocked_vertices.remove(swap_vertice_1)
            unlocked_vertices.remove(swap_vertice_2)

            # record result
            self.max_swaps.append((swap_vertice_1, swap_vertice_2))
            self.max_gains.append(max_gain)
            self.partitions.append((p1.copy(), p2.copy()))
            self.cutsizes.append(self.cutsize(p1, p2))
    

    def result(self):
        np_cutsizes = np.array(self.cutsizes)
        min_cutsize = np.min(np_cutsizes)
        min_cutsize_indices = np.where(np_cutsizes == min_cutsize)[0]
        min_cutsize_index = min_cutsize_indices[0]
        return self.cutsize[min_cutsize_index], self.partitions[min_cutsize_index]


def mincut_placement(file_path, cut_num = 1):
    partitions = []
    cutsizes = []
    cut_count = 0

    # initialize FIFO
    KL = KL_Algorithm(file_path)
    KL.initialize()
    KL_de = deque([KL])

    while cut_count < cut_num:
        for i in range(int(np.log2(cut_count + 1)) + 1):
            # FIFO and partionize
            kl = KL_de.popleft()
            kl.partitionize()

            # record minimum cutsize and corresponding partition
            min_cutsize, best_partition = kl.result()
            cutsizes.append(min_cutsize)
            partitions.append(best_partition)
            
            # add the new sub partitions
            vertices_1, vertices_2 = best_partition
            kl_1, kl_2 = KL_Algorithm(), KL_Algorithm()
            kl_1.sub_initialize(vertices_1, kl.graph)
            kl_2.sub_initialize(vertices_2, kl.graph)
            KL_de.append(kl_1)
            KL_de.append(kl_2)

        cut_count += 1


if __name__ == "__main__":
    # obtain file path from cmd
    parser = argparse.ArgumentParser()
    parser.add_argument("-f", "--file_path", required=True, help="Path to the file to parse.")
    args = parser.parse_args()

    # construct an object
    KL = KL_Algorithm(args.file_path)

    # initialize firstly
    KL.initialize()
    # show the initialized graph
    print(f"Vertices: {KL.vertices}")
    print(f"Graph: {KL.graph}")

    # start partitionize
    KL.partitionize()
    # show the partitionized results
    for i in range(len(KL.partitions)):
        print(f"Iteration {i}. partitions: {KL.partitions[i]}, cutsizes: {KL.cutsizes[i]}")
    for i in range(len(KL.max_gains)):
        print(f"swap pairs {i+1}: {KL.max_swaps[i]}, gain: {KL.max_gains[i]}")

    