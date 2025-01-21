import argparse
import numpy as np

class KL_Algorithm:
    # Constant Global Configuration Variables
    PARTITION_NUMBER = 2
    PARAMETERS_NUMBER = 3

    def __init__(self, file_path):
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
