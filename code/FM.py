import argparse

class FM_Algorithm:
    def __init__(self, file_path):
        self.file_path = file_path

        # graph attributes
        self.degree = 0
        self.vertices = set()
        self.hyperedges = list()
        self.graph = dict()
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
                        vertices = set(''.join(parts))
                        self.vertices = vertices
                        for v in vertices:
                            self.graph[v] = set()
                        self.partitions.append(tuple(map(set, parts))) # tuple up the sets
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
                    # when you add a new item, remove old gain firstly then add a new gain 
                    self.bucket[self.gain(v, p1, p2)].add(v)
                
        except Exception as e:
            print(f"Error reading file: {e}")


    def partionizing(self):
        # When you select the best move, consider about area constraint
        pass
    


if __name__ == "__main__":
    # obtain file path from cmd
    parser = argparse.ArgumentParser()
    parser.add_argument("-f", "--file_path", required=True, help="Path to the file to parse.")
    args = parser.parse_args()

    # construct an object
    FM = FM_Algorithm(args.file_path)

    FM.initialize()

    # Just for test
    print(f"Degree: {FM.degree}")
    print(f"Vertices: {FM.vertices}")
    print(f"Hyperedges: {FM.hyperedges}")
    print(f"Graph: {FM.graph}")
    print(f"Bucket: {FM.bucket}")
    print(f"Partitions: {FM.partitions}")
    print(f"Cutsizes: {FM.cutsizes}")    
    
    max_gain = max((k for k, v in FM.bucket.items() if v), default=None)
    max_gain_vertices = FM.bucket[max_gain]
    max_gain_vertice = min(max_gain_vertices)  # based on the alphabatical order
    print(f"max_gain of initial partiton: {max_gain}")
    print(f"max_gain_vertices of initial partition: {max_gain_vertices}") 
    print(f"Based on alphabetical order, then best max_gain_vertices is {max_gain_vertice}")
