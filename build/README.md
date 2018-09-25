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


// to parse with adjacency:
For each face
  compute the centroid (using mean v) and normal (using mean vn)

A face is adjacent to another if they share an edge.
So we must compute a list of edges
  As we parse each face we have edges
  We need to be able to find an edge rapidly and dereference to a face index
  naive implementation is to have a map<pair<int,int>, pair<int,int>>

  first pair is an edge <from_vtx,to_vtx>
  second pair is the face pair they separate <face_1, face_2>

  then adjacency is given by walking the values of the map

  




# Later
We will load the mesh itself separately for rendering.
