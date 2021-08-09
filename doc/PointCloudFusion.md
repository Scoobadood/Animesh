# Pointcloud Fusion

The current code base assumes that the various frames' data :
* Contain the same number of vertices
* With the same connectivity
* with known correspondence

These are helpful but not realistic assumptions when it comes to 3D reconstruction.
In thie latter case:
* We will have multiple point clouds rather than meshes
* For a given pair of point clouds, they may partially overlap but there will be points in each that have no correspondence with points in the other.
* The point clouds represent discrete samples of parts of the surface of an underlying shape which is deforming. Assumptions will need to be made about the nature of the deformation.
* Connectivity is not given and so will need to be deduced between points.

# Background
Without loss of generality, we can assume that the first frame's data represents a partial surface patch of the subject in its canonical pose.
For each other frame, the task becomes:
* determining which points are common to both frames and
* determining how the surface has deformed between the two frames
* augmenting the underlying surface model by adding any newly revealed points at appropriate locations.

These points are all inter-related.  In particular, to establish a deformation it is necessary to place points in the two frames into correspondence. However, to place points into correspondence we must have some notion of the deformation.

The unconstrained model, which assumes that any point in one frame could, in theory, correspond to any point in the second frame is an ill-posed problem since there are no constraints which can be used to solve it.  We must therefore introduce constraints to make the problem tractable.

 The 3D reconstruction field has matured over recent years to the extent that real time reconstruction of deforming 3D surfaces is possible. Latest works include:

 **CITE**

 Li Hao?

Constraints include:
Starting with a template (usually a skeleton with a well understood set of joint positions and weighted bindings to a surface mesh).
Volumetric methods where deformation of an embedded object is a consequence of a spatial warping defined by e.g. an embedded deformation graph (sparse) or dense deformation field (where each voxel has its own 6DOF transform and these vary smoothly between voxels).

Other approaches include local optimisations for surface patches as well as global optimisations of non-rigid sfm approaches.

These approaches all solve simultaneously for correspondence and deformation. New geometry is extrapolated from existing geometry.

# Approach
One approach to this problem would be to run a preprocessing step in which we integrate all frames of point cloud data into a common model, then extract an initial surface mesh to supply the graph connectivity needed for generating the cross-field . We would also have individual 6DOF transformations for each vertex between frames.

This seems like a fruitful approach.
