import argparse
from Bucket import Bucket

class FM_Algorithm:
    # Constant Global Configuration Variables
    CONSTRAINT_NUMBER = 2
    PARTITION_NUMBER = 2

    def __init__(self, file_path):
        self.file_path = file_path

        # graph attributes
        self.degree = 0
        self.constraints = tuple()
        self.vertices = set()
        self.hyperedges = list()
        self.neigbors = dict()

        # bucket structure (self_built bucket)
        self.bucket = Bucket()

        # result records
        self.partitions = list()
        self.cutsizes = list()
        self.max_gains = list()
        self.max_moves = list()


    def gain(self, x, p1, p2):
        good_hyed, bad_hyed = 0, 0

        for hyed in self.hyperedges:
            if x not in hyed:
                continue

            curr_part, oppo_part = (p1, p2) if x in p1 else (p2, p1)
            if hyed <= curr_part:   # issubset of current partition
                bad_hyed += 1   # bad_hyperedges: the hyperegde of which all the vertices in the current partition
            if hyed - {x} <= oppo_part:  # except {x} issubset of opposite partition
                good_hyed += 1  # good_hyperedges: the hyperedge of which all vertices are in the opposite partition except x
        
        return good_hyed - bad_hyed
    

    def cutsize(self, p1, p2):
        cutsize_sum = 0
        for hped in self.hyperedges:
            if hped & p1 and hped & p2: # hyperedge is intersected with p1 and p2
                cutsize_sum += 1

        return cutsize_sum
    
    def initialize(self):
        try:
            with open(self.file_path, mode="r", encoding="utf-8") as input_file:
                for i, line in enumerate(input_file):
                    parts = line.split()
                    if i == 0:
                        if len(parts) != FM_Algorithm.PARTITION_NUMBER:
                            print(f"Parition error: The inital parition is not equal for {line}")
                            break
                        vertices = set(''.join(parts))
                        self.vertices = vertices
                        for v in vertices:
                            self.neigbors[v] = set()
                        self.partitions.append(tuple(map(set, parts))) # tuple up the sets
                        continue
                    
                    if i == 1:
                        if len(parts) != FM_Algorithm.PARTITION_NUMBER:
                            print(f"Constraints error: the number of low bound and high bound {line}")
                            break
                        self.constraints = tuple(map(int, parts)) # low_boud & high_bound
                        continue

                    hyed = set(parts)
                    self.hyperedges.append(hyed)
                    for v in parts:
                        self.neigbors[v] |= hyed - {v} # union up neigbours (except v)

                # fill degree
                degree = (len(self.hyperedges) + 1) // 2
                self.degree = degree

                # fill cutsizes 
                p1, p2 = self.partitions[0]
                self.cutsizes.append(self.cutsize(p1, p2)) 

                # initialize bucket
                for v in self.vertices:
                    self.bucket.insert(v, self.gain(v, p1, p2))
                
        except Exception as e:
            print(f"Error: {e}")


    def partionizing(self):
        unlocked_vertices = self.vertices.copy()
        low_bound, high_bound = self.constraints
        p1, p2 = self.partitions[0]

        while unlocked_vertices:
            max_gain, max_gain_vertice = self.bucket.pop()
            invalid_pop_items = [] # store invalid_pop_items

            # find valid max_gain_vertice
            while max_gain is not None:
                if max_gain_vertice in p1 and low_bound < len(p1) and high_bound > len(p2) or \
                    max_gain_vertice in p2 and low_bound < len(p2) and high_bound > len(p1): # ensure area constraints
                    # Add those invalid_max_gain_vertices into bucket again
                    for gain, v in invalid_pop_items:
                        self.bucket.insert(v, gain)
                    break
                else:
                    invalid_pop_items.append((max_gain, max_gain_vertice))
                    max_gain, max_gain_vertice = self.bucket.pop()

            # move the selected vertice into opposite partition
            if max_gain_vertice in p1:
                p1.remove(max_gain_vertice)
                p2.add(max_gain_vertice)
            else:
                p2.remove(max_gain_vertice)
                p1.add(max_gain_vertice)
            
            # lock the selected vertice
            unlocked_vertices.remove(max_gain_vertice)

            # update the unclocked neigbours' gain
            neigbours = self.neigbors[max_gain_vertice]
            for v in neigbours:
                if v in unlocked_vertices: # ensure don't need to update the locked vertice
                    self.bucket.update_priority(v, self.gain(v, p1, p2))

            # just for testing
            print(f"max_gain: {max_gain}; max_gain_vertice: {max_gain_vertice}")
            print(f"Move {len(self.vertices) - len(unlocked_vertices)}. bucket: {self.bucket.heap}")

            # Record the results
            self.partitions.append((p1.copy(), p2.copy()))
            self.cutsizes.append(self.cutsize(p1, p2))
            self.max_gains.append(max_gain)
            self.max_moves.append(max_gain_vertice)
        

if __name__ == "__main__":
    # obtain file path from cmd
    parser = argparse.ArgumentParser()
    parser.add_argument("-f", "--file_path", required=True, help="Path to the file to parse.")
    args = parser.parse_args()

    # construct an object
    FM = FM_Algorithm(args.file_path)

    print("********************************************************************************")
    print("Initialization:")

    FM.initialize()

    print(f"Degree: {FM.degree}")
    print(f"Vertices: {FM.vertices}")
    print(f"Constraints: {FM.constraints}")
    print(f"Hyperedges: {FM.hyperedges}")
    print(f"Neighbours: {FM.neigbors}")
    print(f"Bucket: {FM.bucket.heap}")
    print(f"Partitions: {FM.partitions}")
    print(f"Cutsizes: {FM.cutsizes}")

    print("\n********************************************************************************")
    print("Partitioning:")

    FM.partionizing()
    
    print(f"\n*******************************************************************************")
    print("Summary:")
    # show the partitionized results
    for i in range(len(FM.partitions)):
        print(f"Iteration {i}. partitions: {FM.partitions[i]}, cutsizes: {FM.cutsizes[i]}")
    
