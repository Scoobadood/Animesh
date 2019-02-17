# MLE002 Master

This branch is the main branch for Milestone 2; applying the orientation field to disparate point clouds.

The outline plan is as follows:


1. Assign storage for field
a. Load all frames data and establish point correspondences (which may be sloppy). Use this to determine the total number of unique surfels in the space. and allocate storage for them

b. For each surfel, store :
* Neighbouring patches in this frame
* The rotation matrix needed to orient the tangent correctly in frame space. 

2. Smooth field
Repeatedly select a random surfel and frame and apply neighbourhood smoothing until convergence.
Convergence is defined by the overall energy (residuals) being minimised.

Also; we will need visualisation tools to show dense field orientation for a given frame. Nico's suggestion is to plot the error between neighbours which would give high values near singularities.
