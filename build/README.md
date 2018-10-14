# Optimise

We currently run on small toy meshes and processing is slow.
It's probably O(n^2) in places
This will make it unreasonable to run on large point clouds.
We should take this opportunity to optimise the code.

We'll do it in a structured way, obtaining performance stats for loading and smoothing and using e.g. valgrind to find bottlenecks.

# Next
Replace index lookup in Graph which is O(n) in number of nodes with an O(1) equivalent.



Profiler finds that most time is spent in `compute_tangents_for_tier_and_frame` with the bulk of the time split between matrix multiplication  and reprojection.



# Results

## Release mode
+------------------+--------+--------+--------+--------+--------------------------------------+
|                  |        |        |        |        | Times (ms)                           |
|       Test       | Frames | Tiers  |  Vert  | Edges  |------------+------------+------------+
|                  |        |        |        |        |    Load    |  Optimise  |   Total    |
+------------------+--------+--------+--------+--------+------------+------------+------------+
|      sphere10x10 |      1 |      4 |     92 |    190 |         21 |          8 |         30 |
+------------------+--------+--------+--------+--------+------------+------------+------------+
|      sphere20x20 |      1 |      6 |    382 |    780 |        125 |         17 |        143 |
+------------------+--------+--------+--------+--------+------------+------------+------------+
|      sphere32x32 |      1 |      7 |    994 |   2016 |        543 |         58 |        602 |
+------------------+--------+--------+--------+--------+------------+------------+------------+
|       quadsphere |      1 |      7 |    602 |   1200 |        225 |         35 |        260 |
+------------------+--------+--------+--------+--------+------------+------------+------------+
|         cloth2_1 |      1 |      6 |    289 |    544 |         64 |         32 |         97 |
+------------------+--------+--------+--------+--------+------------+------------+------------+
|         horse-04 |      1 |     10 |   8431 |  25274 |      68400 |       1761 |      70162 |
+------------------+--------+--------+--------+--------+------------+------------+------------+
|      mini-horse/ |      3 |     10 |   8431 |  25274 |      69026 |      46190 |     115217 |
+------------------+--------+--------+--------+--------+------------+------------+------------+
|    mini-horse-5/ |      5 |     10 |   8431 |  25274 |      69825 |      98956 |     168782 |
+------------------+--------+--------+--------+--------+------------+------------+------------+
|   mini-horse-10/ |     10 |     10 |   8431 |  25274 |      74372 |     185451 |     259824 |
+------------------+--------+--------+--------+--------+------------+------------+------------+
|   mini-horse-20/ |     20 |     10 |   8431 |  25274 |      74694 |     483612 |     558306 |
+------------------+--------+--------+--------+--------+------------+------------+------------+
|        cloth2-1/ |      1 |      6 |    289 |    544 |         68 |         52 |        121 |
+------------------+--------+--------+--------+--------+------------+------------+------------+
|        cloth2-3/ |      3 |      6 |    289 |    544 |         77 |        313 |        390 |
+------------------+--------+--------+--------+--------+------------+------------+------------+
|        cloth2-5/ |      5 |      6 |    289 |    544 |         84 |        515 |        599 |
+------------------+--------+--------+--------+--------+------------+------------+------------+
|       cloth2-10/ |     10 |      6 |    289 |    544 |        107 |       1163 |       1270 |
+------------------+--------+--------+--------+--------+------------+------------+------------+
|       cloth2-20/ |     20 |      6 |    289 |    544 |        149 |       2399 |       2549 |
+------------------+--------+--------+--------+--------+------------+------------+------------+
For mini-horse, we can plot these values on a chart with frames against optimise time and we see that
this is effectively a linear growth.

Numbers linear regression gives :
Optimise Time = 25,118 * frames - 32728

For cloth, we have different results.
Optimise Time = 123.6 * frames - 75.3

So computation is linear in frames.

However, it is O(n^2) in nodes.
+-------+------+----------+
|Object	| Nodes| 	Optimise|
+-------+------+----------+
|Sp10	  | 92	 | 8        |
+-------+------+----------+
|cloth1	| 289	 | 32       |
+-------+------+----------+
|Sp20	  | 382	 | 17       |
+-------+------+----------+
|Spq	  | 602	 | 35       |
+-------+------+----------+
|Sp32	  | 994	 | 58       |
+-------+------+----------+
|horse4	| 8431 | 1761     |
+-------+------+----------+

Formula is approx y = 2x10^-5 x^2 + 0.03x + 10

How can we improve this?

## Addressing O(n^2)
Optimisation requires that for each node, we compute the mean of its neighbours.
This means that we should have O(n) performance where we are linear in the number of neighbours.

Is there some operation that we perform for each node which is linear in number of nodes?
The heart of optimise is optimise_do_one_step

We'll change add_edge in graph to add both node neighbours and index neighbours for now.

We'll also need to update any edge removal.

```
     [optimise_begin]
         update_tangents( compute_tangents_for_tier_and_frame )

O(1) [optimise_begin_tier]

      compute_new_tangents_for_tier
        for each vertex
          compute_new_tangent_for_vertex
             copy_all_neighbours_for
               neighbour_indices
                 O(n) - checks each node
               O(num neighbours)
               for each frame
                 compute_tangents_for_tier_and_frame
                   for each difference in tier
                     O(num nodes)
                   O(num frames)

                 reproject_to_tangent_space
                   O(1)

             for each neighbour
               average_rosy_vectors

     update_tangents

     check_convergence

     [optimise_end_tier]
         [update_tangents( propagate_tangents_down )]

O(1) [optimise_end]
```
This cursory analysis shows that `compute_new_tangent_for_vertex` is O(n^2) due to compute_new_tangent_for_vertex being O(nodes * frames)
`compute_new_tangent_for_vertex` is O(n) due to the neighbour index check which we should be able to make O(1).
Also we push computation of each tangent across all frames. this is done as a series of matrix ops which might best be turned into a single one.

### First step.
We should definitely replace the index lookup O(n) iteration.









# Standalone Test Case
Loading horse-04 (release code) 69,079ms, 70.198ms,  i.e. 70 seconds!

Most time is spent :
35.94 s   51.3%	1.18 s	 	                animesh::Path::Path(animesh::Path const&) (new)
14.98 s   21.4%	14.81 s	 	                free_tiny


## Getting tangents once
[----------] 1 test from TestPerformance
[ RUN      ] TestPerformance.timeLoadHorse
              Load time (ms): 72524
              compute tangents (7,0) (ms): 9
[       OK ] TestPerformance.timeLoadHorse (72565 ms)
[----------] 1 test from TestPerformance (72565 ms total)

# First optimisation
We can either reduce the number of calls to or the performance of `compute_tangents_for_tier_and_frame`

1. We can cache tangents at each tier and frame and only recompute them as needed
  1.1 Store tangents first time they are needed
  1.2 Set a flag indicating that they are good for the given tier and frame
  1.3 (Important) invalidate cache for tier and frame as needed
    1.3.1 When smooth once is called
    1.3.2 When randomise is called
    1.3.3 When a new object is loaded

2. We can speed up computation of the tangents.
  2.1


Sphere_20x20 seems to have tiers and frames section calculations being equally expensive and both seemingly scaling linearly with the number of nodes.
To optimise frames we might copy all data into a matrix, multiply out and copy back.
This will force the use of Eigen libraries which are better optimised than my handrolled code.
OTOH we should alos run timing tests with optimisation on...
compute_tangents_for_tier_and_frame (5,0) from tier : 0
	tiers  : 2506008ns
	frames : 1779ns
	total  : 2507787ns
compute_tangents_for_tier_and_frame (5,0) from tier : 5
	tiers  : 1289ns
	frames : 1121ns
	total  : 2410ns
compute_tangents_for_tier_and_frame (4,0) from tier : 4
	tiers  : 1161ns
	frames : 1094ns
	total  : 2255ns
compute_tangents_for_tier_and_frame (3,0) from tier : 3
	tiers  : 2393ns
	frames : 2203ns
	total  : 4596ns
compute_tangents_for_tier_and_frame (2,0) from tier : 2
	tiers  : 3247ns
	frames : 2834ns
	total  : 6081ns
compute_tangents_for_tier_and_frame (1,0) from tier : 1
	tiers  : 4625ns
	frames : 4740ns
	total  : 9365ns
compute_tangents_for_tier_and_frame (0,0) from tier : 0
	tiers  : 9257ns
	frames : 8801ns
	total  : 18058ns
