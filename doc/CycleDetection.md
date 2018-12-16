# Cycle Detection
Detecting cycles in the graph is very time consuming.

According to Instruments, the vast majority of the time is spent in Either
* Path copy constructor OR
* Free small amount of memory

This suggests that the creation of Path objects is a major bottleneck and we may
gain a performance lift by either
- finding an alternative approach
- pre-allocating Path objects and reusing them.
- using simpler data structures

## Lets consider fixed data structures

There are N nodes in the graph.
We expect that a minimal cycle would be 3 or 4 steps long though we might allow for longer, say 10.
That would mean storing 10*N `size_t`s to represent all possible paths.
For 100,000 nodes, this would be 8MB which is not a huge amount of memory.
As such we could easily store and grow paths in memory.
We could also choose to prefix each path with its current length to avoid an end of path marker.
This saves lots of object allocations.
