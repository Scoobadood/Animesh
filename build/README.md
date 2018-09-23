# SING-2
Fix up the singularities by:
* Rendering the colours correctly
* Displaying a count of valency 3, 5 and extreme (2 or 6)
* Ensuring all graph loops are clockwise

# Next
Purpose of this branch is to enable the use of faces as the location of orientation fields rather than vertices.

We will do this by modifying ObjFileParser
* Add a parameter to the parse() methods to include a 'vertex oriented' or 'face oriented' flag
* Add a new parsing method to generate point normals for each face rather than per vertex.

# Later
We will load the mesh itself separately for rendering.
