# Implement by max heap and a hash map
# Pop max priority item O(1)
# Search a item O(1)
# Insert a item O(logN)
# Update a item O(logN)


class Bucket:
    def __init__(self):
        self.heap = []
        self.position_map = {}  # Dictionary to track each item's index in the heap
        self.size = 0


    def isLarger(self, item1, item2):
        if item1[0] != item2[0]:
            return item1[0] > item2[0]
        else:
            return item1[1] < item2[1]  # if priority is equal, smaller item is larger ('a' < 'c')
        

    def heapify_up(self, i):
        while i > 0:
            parent = (i - 1) // 2 
            if self.isLarger(self.heap[i], self.heap[parent]): # If node's priority > its parent
                self.swap(i, parent)
                i = parent
            else:
                break

    
    def heapify_down(self, i):
        while 2*i + 1 < self.size:    # i has at least one child
            left = 2 * i + 1
            right = 2 * i + 2
            max_child = left  # Assume left is larger

            if right < self.size and self.isLarger(self.heap[right], self.heap[left]):
                max_child = right  # Right child is larger
 
            if self.isLarger(self.heap[max_child], self.heap[i]):   # Swap if necessary
                self.swap(i, max_child)
                i = max_child  # Continue heapifying down
            else:
                break

    
    def swap(self, i, j):
        self.position_map[self.heap[i][1]] = j
        self.position_map[self.heap[j][1]] = i
        self.heap[i], self.heap[j] = self.heap[j], self.heap[i]


    def insert(self, item, priority):
        if item in self.position_map:
            raise ValueError(f"Item {item} already exists in the heap.")
        
        self.heap.append((priority, item))
        self.position_map[item] = self.size
        self.heapify_up(self.size)
        self.size += 1


    def pop(self):
        if self.size == 0:
            print("Pop from an empty heap")
            return None
        
        pop_item = self.heap[0]  # Highest-priority item
        last_item = self.heap.pop()  # Remove last element
        self.size -= 1
        
        if self.size > 0:  # If heap is not empty after pop
            self.heap[0] = last_item
            self.position_map[last_item[1]] = 0 # record the new index in heap
            self.heapify_down(0)
        
        del self.position_map[pop_item[1]]  # Remove from position map
        return pop_item # return (priority, item)
    

    def update_priority(self, item, new_priority):
        if item not in self.position_map:
            raise ValueError(f"Item {item} not found in the heap.")
        
        i = self.position_map[item]  # Get item's index
        old_priority = self.heap[i][0]
        self.heap[i] = (new_priority, item)  # Update priority

        # Restore heap property
        if new_priority > old_priority:
            self.heapify_up(i)
        else:
            self.heapify_down(i)


    def contains(self, item):
        return item in self.position_map
    

    def get_priority(self, item):
        if item in self.position_map:
            return self.heap[self.position_map[item]][0]
        return None
    
