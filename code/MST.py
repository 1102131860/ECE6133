from itertools import combinations

points = {
    'a': (0, 3),
    'b': (2, 5),
    'c': (3, 2),
    'd': (5, 7),
    'e': (6, 3),
    'f': (6, 0),
}

def find_MST(points):
    def manhattan(p1, p2):
        return abs(p1[0] - p2[0]) + abs(p1[1] - p2[1])

    # Construct the connections from points
    connections = {}
    # Combination of any two nodes
    for u, v in combinations(points.keys(), 2):
        dist = manhattan(points[u], points[v])
        connections[frozenset({u, v})] = dist

    # Sorted those connections
    sorted_connections = sorted(connections.items(), key=lambda x: x[1])

    # Initialization
    nodes = set(points.keys())
    parent = {node: node for node in nodes}

    # Define Find-Union set method
    def find(u):
        while parent[u] != u:
            # This is a optimal operation, connect the subnode to root, e.g, a -> b -> c will be a -> c
            parent[u] = parent[parent[u]]
            u = parent[u]
        return u

    def union(u, v):
        pu, pv = find(u), find(v)
        if pu == pv:
            return False # if u and v are in the same set, don't union them
        parent[pu] = pv # union u to v
        return True

    # Construct MST
    MST = []
    sum_distance = 0

    for edge, distance in sorted_connections:
        u, v = list(edge)
        if union(u, v):
            MST.append(set(edge))
            sum_distance += distance

            # N nodes needs N-1 lengths to build up MST
            if len(MST) == len(nodes) - 1:
                break

    print(f"the MST is {MST}, and the total length is {sum_distance}")

if __name__ == '__main__':
    stiener_points = [('p1', (2,3)), ('p2', (5,3)), ('p3', (3,3))]
    for sp in stiener_points:
        points[sp[0]] = sp[1]
        find_MST(points)


# # Now assume we found the minimum_sum_distance, we want to know all the possible MSTs that has the same minimum_distance

# def is_valid_mst(edge_set, nodes):
#     parent = {node: node for node in nodes}

#     def find(u):
#         while parent[u] != u:
#             parent[u] = parent[parent[u]]
#             u = parent[u]
#         return u

#     def union(u, v):
#         pu, pv = find(u), find(v)
#         if pu == pv:
#             return False
#         parent[pu] = pv
#         return True

#     for edge in edge_set:
#         u, v = list(edge)
#         if not union(u, v):
#             return False  # has cycle

#     # check if all nodes are connected
#     roots = set(find(n) for n in nodes)
#     return len(roots) == 1  # all nodes in one connected component

# # Target MST cost from your Kruskal output
# target_cost = sum_distance

# all_msts = []
# for edges in combinations(connections.items(), len(points) - 1):  # choose 5 edges
#     edge_set = [e[0] for e in edges]
#     total_cost = sum(e[1] for e in edges)
#     if total_cost == target_cost and is_valid_mst(edge_set, points.keys()):
#         all_msts.append(edge_set)

# print(f"Total number of MSTs with length {target_cost}: {len(all_msts)}")
# for i, mst in enumerate(all_msts):
#     print(f"MST #{i+1}: {[set(edge) for edge in mst]}")
