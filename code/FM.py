import argparse

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
        self.graph = dict()

        # bucket structure
        self.bucket = dict()

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
                            self.graph[v] = set()
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
                        self.graph[v] |= hyed - {v} # union up neigbours (except v)

                # initialize bucket with self.degree
                degree = (len(self.hyperedges) + 1) // 2
                self.degree = degree
                for gain in range(-degree, degree): 
                    self.bucket[gain] = set()

                # fill cutsizes and initialized bucket
                p1, p2 = self.partitions[0]
                self.cutsizes.append(self.cutsize(p1, p2)) 
                for v in self.vertices:
                    self.bucket[self.gain(v, p1, p2)].add(v)
                
        except Exception as e:
            print(f"Error: {e}")


    def partionizing(self):
        unlocked_vertices = self.vertices.copy()
        low_bound, high_bound = self.constraints
        p1, p2 = self.partitions[0]

        while unlocked_vertices:
            max_gain_vertice = None
            max_gain = max((k for k, v in self.bucket.items() if v), default=None)

            # find max_gain_vertice
            while max_gain is not None:
                max_gain_vertices = self.bucket[max_gain]

                for v in sorted(max_gain_vertices): # based on alphabetical order
                    if v in unlocked_vertices: # should in the unlocked_vertices
                        if v in p1 and low_bound < len(p1) and high_bound > len(p2) or \
                            v in p2 and low_bound < len(p2) and high_bound > len(p1): # ensure the area constraint
                            max_gain_vertice = v
                            break
                
                if max_gain_vertice is not None:
                    break
                
                # find the second max gain
                max_gain = max((k for k, v in self.bucket.items() if v and k < max_gain), default=None)

            # move the selected vertice into opposite partition
            if max_gain_vertice in p1:
                p1.remove(max_gain_vertice)
                p2.add(max_gain_vertice)
            else:
                p2.remove(max_gain_vertice)
                p1.add(max_gain_vertice)
            
            # lock the selected vertice
            unlocked_vertices.remove(max_gain_vertice)

            # remove max_gain_vertice from self.bucket
            self.bucket[max_gain].remove(max_gain_vertice)

            # update the unclocked neigbours' gain
            neigbours = self.graph[max_gain_vertice]
            for v in neigbours:
                if v in unlocked_vertices: # should in the unlocked_vertices
                    for gain, vs in self.bucket.items():
                        if v in vs:
                            self.bucket[gain].remove(v) # firstly remove neighbour from the old gain
                    
                    update_gain = self.gain(v, p1, p2)  # then add new gain
                    self.bucket[update_gain].add(v)

            # just for testing
            print(f"Move {len(self.vertices) - len(unlocked_vertices)}. bucket: {self.bucket}")

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
    print(f"Graph: {FM.graph}")
    print(f"Bucket: {FM.bucket}")
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
    for i in range(len(FM.max_gains)):
        print(f"Move {i+1}: {FM.max_moves[i]}, gain: {FM.max_gains[i]}")
