# Multi-time point smoothing

## Core 
The core smoothing operation will be to
For each node n (at time 0)
  For each timepoint t
  Let M = the transform from n_t to n_0 
  For each spatial neighbour nn
    Add the nn's tangent to n's and reproject
    (Let MN = nn's transform matrix at time t)
    Apply M' to nn_t giving nn_t'
    Add nn_t's tangent to n's and reproject
  end
end

## Required Functions
get_all_n_at_t0()
get_num_timepoints()
get_fwd_xform_for( node; n, at_time: t )
get_spatial_neighbours_of( node: n, at time: 0 )
merge_tangent( field_element, new_tangent )


### Add'l Supporting functions:
get_neighbours_of( node: n, at_time:t ) : returning 3d points
get_point_corresponding_to( node: n, at_time: t)
compute_xform( node_at_0: n, neighbours_at_0: vector<n>, timepoint_t)


## Details
### get_all_n_at_t0()
The first pointcloud that is added is the t_0 nodes. They must be stored separately from other points in time
They will be the only nodes stored in the Graph (because they are the only nodes for which we need to manage a neighbourhoods relationship)
They could also be stored in a separate vector<> in the Field
**Decision** We'll take the first option since that's what we already have.

### get_num_timepoints()
Could be managed by a simple count. However, we must also store some relationships between timepoints and so will be storing *something* for each time point and so we can use the vector<> size() for this thing as a count of timepoints.

### get_fwd_xform_for( node; n, at_time: t )
We call this per node, multiple times. Once when the node is under consideration and (possibly, later) once when the node is a neighbour.  It's a relatively expensive operation so we'd like to optimise it.

Computing the fwd transform requires a normal (read from file) and a graph with neighbours defined (for final in plane rotation) For each neighbour we need the relative location at that point in time.

Once the xform is computed, we no longer need the future FE as we can project any FE from t_0 into the future (so long as we also have the translation) 
We can't compute the xform until we have the graph built

Graph can be built without reference to any other points in time

Once we have built these xforms, we can store. Lookup by from node and timepoint
map< FieldElement, vector < to_node, xform, translation>>( )

### get_spatial_neighbours_of( node: n, at time: 0 )
For t0 we have this capability already through graph adjacency.
We also need either the ability to map each of these points to a future point OR
a similar spatial relationship mapping at each time point
The former seems to be a more fundamental need elsewhere

### merge_tangent( field_element, new_tangent )
Thinking was that we could move this function into FE but actually, we don't want that; better to leave it in RoSy where it is now.

### get_neighbours_of( node: n, at_time:t ) : returning 3d points
Same as identitfied above. We need a mapping from a node at time 0 to a node at time t
This may mean storing the specific point or it may mean storing the menas to get that point
It may mean both; we will have t and R but for convenience probably want to cache P too.

### get_point_corresponding_to( node: n, at_time: t)
This is the support function for the above method. For a given point/node at t_0 we should be able to get the corresponding point/normal at time t


## Sequencing of Needs
### Time 0/First PointCloud
When we read this, we have all the data we need to construct a graph.

### Second or subsequent point cloud
Initially we need to find the correspondence between existing points and these points. Assuming we have an external method for doing this we need to record that correspondence.  

This mapping is needed both in the immediate term to compute transformations as well as in the longer term to access them. The mapped structure should contain the from node as key and the location, normal, xform as value triplet

After we do correspondence, we will only have the location and normal.

Then we compute the xform and update the correspondence map

## Pseudo Code

### add_pointcloud
FieldElement[] t0_elements = get_t0_elements()
corr = find_correspondences( t0_elements, pointcloud, out: map<FE, pair< point_normal, xform> )
for fe in t0_elements
    N = get_spatial_neighbours_of( node: n, at time: 0 )
    compute_xform( fe, N, corr )
end
corr[timepoint] = corr



## Obvious optimisations
We compute the current time point orientation of each future timepoint element multiple times; it would seem prudent to cache the value or precomute it once


## Outstanding Questions
Simplifying the graph (constructing the hierarchy) requires us to merge FEs. How do we find correspondences in this model for future time points ?
