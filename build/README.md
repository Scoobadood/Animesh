# Load and Render Mesh

Right now we construct a field for each frame which is extracted from a graph extracted from a mesh.

We also ant to render the mesh for comparison.

Here are the steps:

## New layer
We need a new layer in the renderer which shows only the mesh.
It should have its own toggle.
It should (possibly) be a PolyLine - though it has faces.
It will never change and so does not need to be updated.

## Loading
We currently load single and multiple files using the ObjFileParser.  This gives us vertices or faces and adjacency back.
We also need to load specific face data for the mesh sufficicnet to construct the overlay. This may require the ObjFileParser to be broken into chunks.

## Storage
Since there will be a separate mesh per layer, we will need to redraw the layer when the frame changes.
So we will need an update method for the mesh layer.

## Options
We may render edges, points or faces of the mesh so we will need checkbox/drop down for this.

## Process
Construct a PolyLine which can store the mesh and render it.
Add toggle on and off.
Populate with simple cube data to test.

# Next Step
Build a PolyLine layer for mesh.

Implement init_mesh_layer and update_mesh_layer.
