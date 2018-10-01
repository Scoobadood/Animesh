
# Mesh/Graph/Singularity Confusion

When working with a triangular (or quad) mesh, we're given vertices and faces defined in a file. When computing the orientation field, I convert this mesh format into a graph.

Two main options here seem to be:

* Make nodes and edges in graphs correspond to vertices and edges in the mesh OR;
* Make a graph node for each face in the mesh and join nodes with an edge if the corresponding faces are adjacent in the mesh.

Adjacency in this case is defined as 'sharing an edge' though it could conceivably be defined as 'sharing a vertex'.

If we derive our graph from some other data source i.e. an unstructured point cloud, we do not have any face information. For this reason I've been focussing on graph based methods that do not assume an underlying mesh.

To compute singularities in this graph I need to extract minimal cycles and then walk these cycles in a consistent direction computing the sum of differences between 'k' values for each edge. (modulo 4).


## First confusion

Let's say we used node/edge = vertex/edge representation for the mesh below.

A      B      C      D
+------+------+------+
|\     |\     |\     |
| \    | \    | \    |
|  \   |  \   |  \   |
|   \  |   \  |   \  |
|    \ |    \ |    \ |
|     \|     \|     \|
+------+------+------+
E      F      G      H

Assuming faces are defined anti-clockwise, then we have the following 6 cycles:
AEFA AFBA
BFGB BGCB
CGHC CHDC

But this list contains unnecessary duplication; for each node (except D and E) there are at least two cycles containing that node.
As an alternative, I can define a list of minimal cycles:

AEFA BGCB CHDC

which still has the property that each vertex appears in a cycle but now I only need 3 cycles and not 6.

The challenge is to determine which is 'correct'.  It's not obvious to me whether I should include every conceivable cycle of minimal length
when computing singularities or whether I should aim to have a single, minimal cycle, for each node in the graph.
Do I lose singularities with this, simpler set of cycles or are they all retained?


## Second confusion

If I consider this cube:

       A          B
        +--------+
       /:       /|
   D  / :    C / |
     +--------+  |
     |  :     |  |
     | E:.....|..+ F
     | ;      | /
     |;       |/
   H +--------+ G

   This time, we let each face be a node in the graph and graph edges represent face adjacency.

   The faces are:
   F1: ADCB     F2: DHGC
   F3: HEFG     F4: CGFB
   F5: AEHD     F6: BFEA

   So the graph has 6 nodes with the following adjacency:

   F1 : F2 F4 F5 F6
   F2 : F3 F4 F5 F1
   F3 : F2 F4 F5 F6
   F4 : F1 F2 F3 F6
   F5 : F1 F2 F3 F6
   F6 : F1 F3 F4 F5

   So we *could* extract the following minimal cycles:
   F1F2F4F1, F1F6F5F1, F3F2F4F3

   All faces are represented here so we can compute an index for each node in our graph.
   But we'd end up with at most 3 singularities since we are considering only 3 cycles.

   Intuitively there are 8 singularities, one at each vertex of the original cube.
   (Also because topologically the cube has the same genus as a sphere which has 8 singularities).

   But even if we consider _every_ node in the graph and get a separate minimal cycle for it, that's still only 6 singularities at most.

   ## Finally
   If I map each vertex in the cube to a node in the graph then I have a graph with adjacencies:
   A: B D E       E: A F H       
   B: A C F       F: B E G
   C: B D G       G: C H F
   D: A C H       H: D E G

   Now I can compute 4 minimal cycles which include all vertices
   AEHDA, DHGCD, CGFBC, BFEAB
   But again, 4 cycles == 4 singularities at most.

   Or I could compute every minimal cycle including each node.
   AEHDA, ABFEA, ADCBA,
   BCGFB, CDHGC, EFGHE
   But this is still only 6 cycles.

   # In Summary
   It seems that the number of singularities that are computed for a field on a mesh is critically dependent on the graph representation used for the mesh and the choice of how to extract minimal cycles from that graph. Is there some canonical approach to this that I'm missing?
