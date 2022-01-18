## Correspondence Validation
### Background
#### Correspondences
We use CPD (Coherent Point Drift) to compute correspondences between points over time in our deforming point cloud
This algorithm is performed frame to frame across each pair of frames and from that we extract the coherent path 
of a single point across multiple frames.

A simplifying assumption that we make is that each pixel in a depth map (so each point in a cloud) corresponds to only one
point in the next frame (CPD does not impose this constraint). There's always the possibility that an ambiguous match between frames may occur and a path may consequently be incorrect.

#### Surfels
Each correspondence is used to generate a `Surfel` - a surface patch on the mesh which moves over time.
Surfel's store per-frame data  including position, normal orientation etc.
When we perform smoothing, we use pairs of neighbouring surfels to smooth. So we need to compute their neighbours.

#### Neighbours
We use 8 connected pixel adjacency to determine the neighbours of a partiular point. Thus in frame 1 for example, we may have a point that projects to the pixel (10,20) in the depth map. We start with the assumption that 
pixels at (9,20), (11,20), (9,19), (10,19) etc. are neighbours.  Not all of these pixels may be valid. We may have pruned some because they had
no valid depth. We may have pruned others becase of their adjacency to a depth discontinuity. But for those that do exist, we work out the surfel to which they belong and assert that this surfel is a neighbour of the surfel under consideration.

The flaw here is that two points may appear to be adjacent in one frame but turn out to be quite distant in another.
So we need to validate the neighbourhood relationships.

#### A threshold test
One way of doing this would be to examine the proximity of the 3D points represented by apparently 'adjacent' pixels across all frames
and then only commit to them being neighbours iff the distance between the points does not exceed some threshold.
The reasoning here is that proximity can often be illusory whereas distance is not. So if two points are ever far apart, we can rejct them as neighbours.
This is true but we run the risk of throwing away good neighbours due to ambiguous or outlier results. Consider the illustration below.
```
  1     2     3   4     5
A o=====o=====o===o=====o
  |     |     |   |     |
B o=====o=====o   |     |
                \ |     |
                  o=====o
```
In the above, between frames 3 and 4 the distance between the apparent neighbours grows beyond the threshold. We could determine that this means that the two points are not neighbours at all. Though this
is possible, it discounts the 'good' neighbour hood relationships in frames 1,2 and 3.  The problem here is that chosing to 'cut' a neighbourhood relationship in one frame, severs it in all frames
There is an alternative. We can cut the correspondence relationship instead thus preserving neighbour hood realtionships. See below:
```
  1     2     3   4     5
A o=====o=====o=X=o=====o
  |     |     |   |     |
B o=====o=====o   |     |
                \ |     |
                  o=====o
```
If we cut the correspondence between frames 3 and 4, we can preserve the good neighbourhood relationship for frames 1,2 and 3 while discarding that in frames 4 and 5
```
  1     2     3    4     5
A o=====o=====o  C o=====o
  |     |     |   
B o=====o=====o   
                \ 
                  o=====o
```
We could equally cut the correspondence for B between frames 3 and 4
```
  1     2     3   4     5
A o=====o=====o===o=====o
  |     |     |   
B o=====o=====o   
                  
                C o=====o
```
In this case we have three possible 'cuts' that can be made to resolve the problem. We need a mechanism for determining which is the 'best' in some sense.
There are some challenges here though. First is that we have only considered two neighbours whereas of course, there are many neighbours for each possible point so a global optiisation 
is probably required. The challenge is to find a local operation that will work here.  

Also we need to consider that there may need to be multiple cuts.
```
  1     2     3   4     5     6     7     8
A o=====o=====o===o=====o   
  |     |     |   |     | \
B o=====o=====o   |     |   o=====o=====o
                \ |     |   |     |     |
                  o=====o===o=====o=====o
```
In the above we could cut between 3 and 4 on upper or lower correspondence paths and also between 5 and 6. That gives four possible
options:
```
  1     2     3     4     5     6     7     8
A o=====o=====o   C o=====o   
  |     |     |           
B o=====o=====o           D o=====o=====o
                \           |     |     |
                  o=====o===o=====o=====o
```
```
  1     2     3     4     5     6     7     8
A o=====o=====o   C o=====o   
  |     |     |             \
B o=====o=====o               o=====o=====o
                \             |     |     |
                  o=====o   D o=====o=====o
```
```
  1     2     3   4     5     6     7     8
A o=====o=====o===o=====o   
  |     |     |            \
B o=====o=====o               o=====o=====o
                              |     |     |
                C o=====o   D o=====o=====o
```
```
  1     2     3   4     5     6     7     8
A o=====o=====o===o=====o   
  |     |     |            
B o=====o=====o           C o=====o=====o
                            |     |     |
                D o=====o===o=====o=====o
```
(as well as the trivial decision that these points should not be neighbours at all)

#### A local optimisation
We need to decide where to place a cut.
So, we need to compute some sort of metric which values a particular configuration of correspondences and neighbour relationships.
The metric should include:

**1. A penalty for the total/mean/median distance between neighbours**
: We prefer neighbouring points to be close together and so we penalise apparent neighbours who are far apart.

**2. A reward for long strings of neighbourness**: The above metric would reward the situation where we made every point have no neighbours. We wish to disincentivise cuts that would effectively result in points having no nieghbours
so we reward some combination of (num neighbours * duration of neighbourhoodness)

**3. A penalty for short paths**: Paths of length one or two are unhelpful when smoothing temporally so we penalise them.

We'd like to normalise these terms as that will make it easier to balance the coefficients.

1. We have a threshold. We can iterate over each frame in which these two points are neighbours, compute the distance based on point clouds and compare it to the threshold.
Some options for this would be:
* Percentage of frames out of range (This doesn't punish bigger exceptions more but is in range 0 - 1)
* sum(max(0, actual dist - threshold)^2)  : This is uncapped but grows rapidly with big values

### Implementation
#### Base Algorithm
```
for each pif in frame 0
  construct the set of all neighbouring paths across all frames
    - trace the path
    - find neighbouring pifs in each frame
    - trace their paths
  we now have a bundle of paths
    
 
We have a series of `correspondences` computed from the point clouds which are in turn computed from the depth maps and camera positions.

These are in the form of a series of lines of (frame,index) pairs where the index relates to the point in the pointclouds. Each correspondence represents the position of a single actual point over time.

We have traceability to PIF from the `pifs` files though these don't give depth so we'd need to load the depth maps too.

So we must load :
* depth_map_genned_Lnn_Fnn.pgm
* pifs_Lnn_Fnn.txt and
* paths_nn.txt

### Approach
