# SING-2
Fix up the singularities by:
* Rendering the colours correctly
* Displaying a count of valency 3, 5 and extreme (2 or 6)
* Ensuring all graph loops are clockwise

# Next
ObjFileParser seems to be doing a poor job on Sphere
There are no tests for it.
Make some.

Problem was we were adding adjacency twice for each face, i.e. 1 adj 2 and 2 adj 1
For ObjFileParser, this should be implicit since edge relationships are undirected.
Removing this makes local tests pass and we should handle any interpretation of
adjacency in other layers of the stack.
