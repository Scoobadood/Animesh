# MLE002 Master

This branch is the main branch for Milestone 2; applying the orientation field to disparate point clouds.

The outline plan is as follows

1. Generate ground truth "cheat" data for a non-trivial blend shape

2. Assign storage for field
a. Load all frames data and establish point correspondences (which may be sloppy). Use this to determine the total number of unique surfels in the space. and allocate storage for them

b. For each surfel, store :
* Neighbouring patches in this frame
* The rotation matrix needed to orient the tangent correctly in frame space. 

3. Smooth field
Repeatedly select a random surfel and frame and apply neighbourhood smoothing until convergence.
Convergence is defined by the overall energy (residuals) being minimised.

We will need visualisation tools to show dense field orientation for a given frame. Nico's suggestion is to plot the error between neighbours which would give high values near singularities.

## Ground Truth Data
We want to have a blend shape which animates over time and where we capture depth data (like RGBD). The camera may move rigidly between frames too.  For each frame we need  depth image and also a pixel to vertex correspondence which maps each pixel back to the original vertex in the mesh from which the images were generated.

The depth data represents actual depth data.

The pixel to vertex mapping is a means of establishing a sloppy form of correspondence; pixels labelled with the same vertex can be assumed to be in correspondence between frames.

* Find an animated sequence for which we have known vertices and positions in object space
* Select a camera location, facing the object
* For each frame
  * generate a depth image
  * generate a mapping from each pixel to the nearest vertex in the model

### Model data
Ideally we want a series of vertices posed in particular locations.
We can use the horse gallop sequence from Sumner (Robert W. Sumner, Jovan Popovic. Deformation Transfer for Triangle Meshes. ACM Transactions on Graphics. 23, 3. August 2004. http://graphics.csail.mit.edu/~sumner/research/deftransfer/)
The frames are sparse (i.e. there are big changes in pose between them however this does not prevent our 'cheat' algorithm from working. It might present problems for detecting correspondences later in the end to end pipeline.

### Camera location
We can pick this arbitrarily. Perhaps our visualisation tool should let us change the position; e.g. rotate around the thing.

### Ray projection
We have CUDA ray tracing code in old code base for 3d recinstruction

Assuming a manual process, we will do the following algo:
* For each frame
  * For each pixel in the image
    * Project a ray from camera centre, through pixel of image plane
    	We have origin of camera = 0,0,0,
    	We have ray tracing
    * Intersect this ray with the mesh surface
    * Determine the distance
    * And the closest vertex
    * Update two images
  * Save images

### Intersection with model (CUDA? or OpenCL? or just OpenGL? or a fragment shader?)



## Assign storage
In this simple model, we'll end up with a number of surfels equal to the number of vertices in the original image?
Or if not then what? Many to many correspondences need to be eliminated somehow.

## Initialise random tangents 
In canonical space, we can generate a random tangent.

## Smooth the field
Repeatedly select a random surfel and frame
Find neighbours in that frame
Smooth in frame space
Transform back into canonical space.

## Other
### Depth map hierarchy
We can generate a hierarchical depth map by repeatedly subsampling the depth map.  Before subsampling we must remove pixels along the boundary of a depth discontinuity both before and after.

### Graph hierarchy
We can continue to simplify the 
