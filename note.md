# Kernighan-Lin Algorithm

- It is a **biosectioning** algorithm
    - The input graph is partitioned into two subsets of equal sizes.

- Till the cutsize keeps improving
    - Vertex pairs which give the **largest decrease in cutsize** are exchanged.
    - These vertices are then **locked**
    - If no improvement is possible and some vertices are still unlocked, the vertices which give the smallest increase are exchanged.

- Kernighan-Lin Algorithm  
    - The formula of `gain`: 

        $$D(x, y) = g(x, y) = E_x - I_x + E_y - I_y - 2c(x, y)$$

        , where $E_x$ **is the # of extended neighbors of x**,  
        $I_x$ **is the # of interior neighbors of x**,   
        and $c(x, y)$ **is the # of mutual connected lines between x and y** 

    - **Algorithm KL**
    ```pesudocode
    begin
        INITIALIZE()
        while (IMPROVE(table) == TRUE) do (if and improvement has been made during last iteration, the process is carried out again.)
            while (UNLOCK(A) == TRUE) do (if there exists any unlocked vertex in A, more tentaive exchanges are carried out.)
                for (each a in A) do
                    if (a == unlocked) then
                        for (each b in B) then
                            if (b == unlocked) then
                                if (D_max < D(a) + D(b)) then
                                    D_max = D(a) + D(b)
                                    a_max = a
                                    b_max = b
                TENT-EXCHANGE(a_max, b_max)
                LOCK(a_max, b_max);
                LOG(table)
                D_max = negative infinite
            ACTUAL-EXCHANGE(table)
    end
    ```

- Example
    - Perform single KL pass on the following circuit:
        - KL needs **undirected graph** (**clique-based** weighting)  
            - *a single line represents a weight of 0.5*
            - *a bold line represents a weight of 1*

        ![example circuit graph](./images/image_1.png)

    - **First Swap (16 potential swaps)**

        ![Initial partitioning](./images/image_2.png)

        |   pair    |   $\boldsymbol{E_x - I_x}$    |   $\boldsymbol{E_y - I_y}$    |   $\boldsymbol{c(x,y)}$   |   gain    |
        |-----------|-------------------------------|-------------------------------|---------------------------|-----------|
        |   (a, c)  |   0.5 - 0.5                   |    2.5 - 0.5                  |   0.5                     |   1       |
        |   (a, f)  |   0.5 - 0.5                   |    1.5 - 1.5                  |   0                       |   0       |
        |   (a, g)  |   0.5 - 0.5                   |    1 - 1                      |   0                       |   0       |
        |   (a, h)  |   0.5 - 0.5                   |    0 - 1                      |   0                       |   -1      |
        |   (b, c)  |   0.5 - 0.5                   |    2.5 - 0.5                  |   0.5                     |   1       |
        |   (b, f)  |   0.5 - 0.5                   |    1.5 - 1.5                  |   0                       |   0       |
        |   (b, g)  |   0.5 - 0.5                   |    1 - 1                      |   0                       |   0       |
        |   (b, h)  |   0.5 - 0.5                   |    0 - 1                      |   0                       |   -1      |
        |   **(d, c)**  |   **1.5 - 0.5**                   |   **2.5 - 0.5**                  |   **0.5**                     |   **2**       |
        |   (d, f)  |   1.5 - 0.5                   |    1.5 - 1.5                  |   1                       |   -1      |
        |   (d, g)  |   1.5 - 0.5                   |    1 - 1                      |   0                       |   1       |
        |   (d, h)  |   1.5 - 0.5                   |    0 - 1                      |   0                       |   0       |
        |   **(e, c)**  |   **2.5 - 0.5**                   |    **2.5 - 0.5**                  |   **1**                       |   **2**       |
        |   (e, f)  |   2.5 - 0.5                   |    1.5 - 1.5                  |   0.5                     |   1       |
        |   (e, g)  |   2.5 - 0.5                   |    1 - 1                      |   1                       |   0       |
        |   (e, h)  |   2.5 - 0.5                   |    0 - 1                      |   0                       |   1       |


        Select **(d,c)** pair as next swap

    - **Second Swap (9 potential swaps)**

        ![Second Swap](./images/image_3.png)
   
        |   pair    |   $\boldsymbol{E_x - I_x}$    |   $\boldsymbol{E_y - I_y}$    |   $\boldsymbol{c(x,y)}$   |   gain    |
        |-----------|-------------------------------|-------------------------------|---------------------------|-----------|
        |   (a, f)  |   0 - 1                       |   1 - 2                       |   0                       |   -2      |
        |   (a, g)  |   0 - 1                       |   1 - 1                       |   0                       |   -1      |
        |   (a, h)  |   0 - 1                       |   0 - 1                       |   0                       |   -2      |
        |   (b, f)  |   0.5 - 0.5                   |   1 - 2                       |   0                       |   -1      |
        | **(b, g)** | **0.5 - 0.5** | **1 - 1** | **0**     | **0** |
        | (b, h)    |   0.5 - 0.5                   |   0 - 1                       |   0                       |   -1      |
        | (e, f)    |   1.5 - 1.5                   |   1 - 2                       |   0.5                     |   -2      |
        | (e, g)    |   1.5 - 1.5                   |   1 - 1                       |   1                       |   -2      |
        | (e, h)    |   1.5 - 1.5                   |   0 - 1                       |   0                       |   -1      |

        Select **(b, g)** pair as next swap

    - **Third Swap (4 potential swaps)**

        ![Third Swap](./images/image_4.png)

        |   pair    |   $\boldsymbol{E_x - I_x}$    |   $\boldsymbol{E_y - I_y}$    |   $\boldsymbol{c(x,y)}$   |   gain    |
        |-----------|-------------------------------|-------------------------------|---------------------------|-----------|
        | **(a, f)** | **0 - 1**                    | **1.5 - 1.5**                 |   **0**                   |   **-1**  |
        | (a, h)    |   0 - 1                       |   0.5 - 0.5                   |   0                       |   -1      |
        | (e, f)    |   0.5 - 2.5                   |   1.5 - 1.5                   |   0.5                     |   -3      |
        | (e, h)    |   0.5 - 2.5                   |   0.5 - 0.5                   |   0                       |   -2      |

    Select **(a, f)** pair as next swap

    - **Fourth Swap (The last swap)**

        |   pair    |   $\boldsymbol{E_x - I_x}$    |   $\boldsymbol{E_y - I_y}$    |   $\boldsymbol{c(x,y)}$   |   gain    |
        |-----------|-------------------------------|-------------------------------|---------------------------|-----------|
        |   (e, h)  |   0.5 - 2.5                   |   1 - 0                       |   0                       |   -1      |

    - The last swap always executes.

        ![Last swap and its result](./images/image_5.png)

    - Summary
        - **Cutsize Reduced from 5 to 3**
        - Two best solutions found (solutions are always **area-balanced**)
        - Cutsize: Take look at those images
            - The original Cutsize, look at the image of initial partitioning, and you can found 0.5 + 0.5 + 0.5 + 1 + 1 + 0.5 + 1 = 5 edges
            - Cutsize of 1st iteration looks at the second swap's image, and you can found it cuts 0.5 + 0.5 + 0.5 + 0.5 + 1 = 3 edges
            - Cutsize of 2 iteration looks at the third swap's image, and you can found it cuts 0.5 + 0.5 + 0.5 + 0.5 + 0.5 + 0.5 = 3 edges
            - Cutsize of 3 iteration looks at the last swap's (a), and you can found it cuts 1 + 0.5 + 0.5 + 0.5 + 0.5 + 0.5 + 0.5 = 4 edges
            - Custize of 4 iteration looks at the last swap's (b), and you can found it cuts 1 + 0.5 + 1 + 0.5 + 0.5 + 0.5 + 1 = 5 edges
        - $\Sigma gain(i) = Cutsize_0 - Cutsize_i$ 
        - gain(i) is the select pair's gain.


        | $\boldsymbol{i}$   |  pair     |  gain(i)   | $\Sigma$ gain(i)   | Cutsize |
        |--------------------|-----------|------------|--------------------|---------|
        |   0                | -         | -          | -                  | 5       |
        |   **1**            | **(d, c)** | **2**     | **2**              | **3**   |
        |   **2**            | **(b, g)** | **0**     | **2**              | **3**   |
        |   3                | (a, f)    | -1         | 1                  | 4       |
        |   4                | (e, h)    | -1         | 0                  | 5       |

    
- Drawbacks of K-L Algorithm
    - considers balanced partitions only
    - As vertices have unit weights, it is not possible to allocate a vertex to a partition
    - considers edges instead of hyperedges
    - High time complexity $O(n^3)$    
    
    
# Fiduccia-Mattheyses Algorithm

- A modified version of Kernighan-Lin Algorithm

- A single vertex is moved across the cut in a single move which **permits handling of unbalanced partitions**

- The concept of cutsize is extended to **hypergraphs**
    - The updated **gain** :
    
        $$\text{gain(x)} = \text{good hyperedges} - \text{bad hyperedges}$$
    
        , where $\text{good hyperedges}: \text{total number of hyperedges that contain x as the only gate at the other side}$

        $\text{bad hyperedges}: \text{total number uncut hyperedges that contain x}$

    - **neighbour**: All gates that are contained in the hypergraphs taht contain x

    - The time complexity of calculating a gain for a given vertice is $O(1)$

    - The time complexity of finding the neighhours for a given vertice is also $O(1)$

- Vertices to be moved are selected in a way (**bucket list**) to improve time complexity
    - **bucket lists (priority queues)**
        - Sort nodes by gain within each partition

    ![bucket structure](./images/image_6.png)

- Overall time complexity of the algorithm is $O(n^2)$

- Pesudocode
    ```
    Input: 
    - G(V, E): Hypergraph with vertices V and hyperedges E
    - P1, P2: Initial partition of V such that |P1| ≈ |P2|
    - Max_imbalance: Allowed imbalance between partitions

    Output:
    - Improved partition (P1', P2') with minimized edge cut

    Initialize iteration variables:
    - Best_cutsize = Current cutsize
    - Best_move_sequence = []
    - Current_cutsize = Current cutsize
    - Moved_nodes = []
    - create a new bucket list
    - compute gain for each node and put them into bucket list

    Begin
    while there exists unlocked node in the bucket list:
        i. select the highest gain node from non-empty bucket list 
            - ensure the selected node will not exceed allowed imbalance between two partitions
        ii. move the selected node to the other partition
        iii. lock the moved node
        iv. update cutsize and track changes
            - Update Current_cutsize
            - Append move to Moved_nodes
        v. update gain of affected neigbours
            - Recompute gains for adjacent nodes
            - Move them in the bucket list accordingly
        vi. if Current_cutsize < Best_cutsize:
            - Store `Best_cutsize`
            - Store `Best_move_sequence`
    End
    ```

- Example
    - Perform FM algorithm on the following circuit:
        - Area constraint = [3, 5]
            - The maximum imbalance between two sides cannot exceed 2

        - Break ties in alphabetical order

        - Example circuit and the hypergraphs

        ![Eaxmple circuit and the hypergraphs](./images/image_7.png)
    
    - Random initial partioning is gaiven
        - {a, c, d, g} and {b, e, f, h}

        - Six hyperedges
            - n1: (a, c, e)
            - n2: (b, c, d)
            - n3: (c, f, e)
            - n4: (g, f, h)
            - n5: (d, f)
            - n6: (g, e)

        ![Random inital partioning](./images/image_8.png)

    - **Initial Bucket**
        
        |  Vertice     |  good hyperedge     |   bad hyperedge    |   gain    |
        |--------------|---------------------|--------------------|-----------|
        |   a          |    0                |     0              |   0       |
        |   b          |    1                |     0              |   1       |
        |   c          |    1                |     0              |   1       |
        |   d          |    1                |     0              |   1       |
        |   **e**      |    **2**            |     **0**          |   **2**   |
        |   f          |    1                |     0              |   1       |
        |   **g**      |    **2**            |     **0**          |   **2**   |
        |   h          |    0                |      0             |   0       |

        ![Initial Bucket](./images/image_9.png)

        - Cutsize: 6

    - **First Move**
        - both cell *e* and *g* have the maximum gain and can be **moved without violating the area constraint**
        
        - Move **e** based on alphabetical order

        - Lock cell **e**
        
        - The neighbour of e is {a, c, f, g}

        - Update the neighbour of c and get the updated bucket list

        |  Vertice     |   good hyperedge    |  bad hyperedge     |   gain   |
        |--------------|---------------------|--------------------|----------|
        |  *a*         |   *0*               |  *1*               |   *-1*   |
        |   b          |    1                |  0                 |   1      |
        |  *c*         |   *0*               |  *1*               |   *-1*   |
        |   d          |    1                |  0                 |   1      |
        |  ***f***     |   ***2***           |  ***0***           |   ***2***|
        |  *g*         |   *1*               |  *1*               |   *0*    |
        |   h          |    0                |   0                |   0      |

        ![First Move](./images/image_10.png)

        - Cutsize: 4

    - **Second Move**
        - cell *f* has the maximum gain but **move it will lead imbalance to exceed the area constraint**
        
        - Move **d** because it has the second maximum gain

        - Lock cell **d**

        - The neighbour of d is {b, c, f}

        - Update the neighbour of d and get the updated bucket list

        |  Vertice     |   good hyperedge    |  bad hyperedge     |   gain   |
        |--------------|---------------------|--------------------|----------|
        |  a           |   0                 |  1                 |   -1     |
        |   ***b***    |    ***0***          |  ***0***           |   ***0***|
        |   ***c***    |    ***1***          |  ***1***           |   ***0***|
        |   ***f***    |    ***1***          |  ***1***           |   ***0***|
        |  **g**       |   **1**             |  **1**             |   **0**  |
        |  **h**       |   **0**             |  **0**             |   **0**  |

        ![Second Move](./images/image_11.png)

        - Cutsize: 3

    - **Third Move**
        - cell *b*, *c*, *f*, *g*, *h* have the maximum gain

        - Move **b** as **move it without violating the area constraint** and on the alphabetical order

        - Lock cell **b**

        - The neighbour of b is {c} (cell d has been locked)

        - Update the neighbor of b and get the updated bucket list 

        |  Vertice     |   good hyperedge    |  bad hyperedge     |   gain   |
        |--------------|---------------------|--------------------|----------|
        |  a           |   0                 |  1                 |   -1     |
        |   *c*        |    *0*              |  *1*               |   *-1*   |
        |  **f**       |   **1**             |  **1**             |   **0**  |
        |  **g**       |   **1**             |  **1**             |   **0**  |
        |  **h**       |   **0**             |  **0**             |   **0**  |

        ![Third Move](./images/image_12.png)

        - Cutsize: 3

    - **Forth Move**
        - cell *f*, *g*, *h* have the maximum gain

        - Move **g** as **move it withut violating the area constraint** and on the alphabetical order

        - Lock cell **g**

        - The neighbour of g is {f, h} (cell e has been locked)

        - Update the neighbor of g and get the updated bucket list

        |  Vertice     |   good hyperedge    |  bad hyperedge     |   gain   |
        |--------------|---------------------|--------------------|----------|
        |  ***a***     |   ***0***           |  ***1***           |  ***-1***|
        |  ***c***     |   ***0***           |  ***1***           |  ***-1***|
        |  ***f***     |   ***1***           |  ***2***           |  ***-1***|
        |  ***h***     |   ***0***           |  ***1***           |  ***-1***|

        ![Forth move](./images/image_13.png)

        - Cutsize: 3

    - **Fifth Move**
        - cell *a*, *c*, *f*, *h* have the maximum gain

        - Move **a** on the alphabetical order

        - Lock cell **a**

        - The neighbour of a is {c} (cell e has been locked)

        - Update the neigbour of a and get the updated bucket list

        |  Vertice     |   good hyperedge    |  bad hyperedge     |   gain   |
        |--------------|---------------------|--------------------|----------|
        |  ***c***     |   ***0***           |  ***0***           |  ***0*** |
        |  f           |   1                 |  2                 |  -1      |
        |  h           |   0                 |  1                 |  -1      |

        ![Fifth Move](./images/image_14.png)

        - Cutsize: 4

    - **Sixth Move**
        - cell *c* has the maxiumum gain but **moving c will violates the area constraint**

        - Move **f** on the alphabetical order

        - Lock cell **f**

        - The neighbour of f is {c, h} (cell c, d and g have been locked)

        - Update the neighbour of f and get the updated bucket list

        |  Vertice     |   good hyperedge    |  bad hyperedge     |   gain   |
        |--------------|---------------------|--------------------|----------|
        |  *c*         |   *0*               |  *1*               |  *-1*    |
        |  ***h***     |   ***0***           |  ***0***           |  ***0*** |

        ![Sixth Move](./images/image_15.png)

        - Cutsize: 5

    - **Seventh Move**
        - cell *h* has the maximum gain and move without violating the area constraint

        - Move **h**

        - Lock cell **h**

        - **h has no neigbhour** and the bucket list will be

        |  Vertice     |   good hyperedge    |  bad hyperedge     |   gain   |
        |--------------|---------------------|--------------------|----------|
        |  c           |   0                 |  1                 |  -1      |
        
        ![Seventh Move](./images/image_16.png)

        - Cutsize: 5

    - **Last Move**
        - can only move cell **c**

        ![Last Move](./images/image_17.png)

        - Custzie: 6

    - **Summary**
        - Found three best solutions

        - Solutions after moving 2 and 4 are better balanced

        |   i   |   cell    |   g(i)    |   $\sum g(i)$ |   cutsize |
        |-------|-----------|-----------|---------------|-----------|
        |   0   |   -       |   -       |   -           |   6       |
        |   1   |   e       |   2       |   2           |   4       |
        | **2** |   **d**   |   **1**   |   **3**       |   **3**   |
        | **3** |   **b**   |   **0**   |   **3**       |   **3**   |
        | **4** |   **g**   |   **0**   |   **3**       |   **3**   |
        |   5   |   a       |   -1      |   2           |   4       |
        |   6   |   f       |   -1      |   1           |   5       |
        |   7   |   h       |   0       |   1           |   5       |
        |   8   |   c       |   -1      |   0           |   6       |

        - when i = 2 and i = 4, the two sides both have 4 cells, while i = 3, one side has 3 cells and another side has 5 cells.
            - {a, c, e, g} and {b, d, f, h}, {a, b, c, e} and {d, f, h, g} are more balanced than {a, b, c, e, g} and {f, g, h}


