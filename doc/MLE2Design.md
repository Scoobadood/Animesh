# Data Structures

The input is a sequence of images with depth data and the vertex correspondence mapping.
The output needs to be storage for surfels for each discrete vertex as well as the correspondences between them

In this simple model , correspondences are provided by the vertex mappings and so are effectively 'given'. IN an actual solution, this wouldn't be the case and a discrete correspondence mapping component would be needed. We should model this abstractionbecause later we may have multiple correspondence algorithms.

Surfel storage should also include transformation matrices to the frames in thich the surfl appears along with neighbouring surfels

2. Assign storage for field
a. Load all frames data and establish point correspondences (which may be sloppy). Use this to determine the total number of unique surfels in the space. and allocate storage for them

b. For each surfel, store :
* Neighbouring patches in this frame
* The rotation matrix needed to orient the tangent correctly in frame space. 


## Data Structures

Surfel Data
int 			id // the index number of this surfel
FrameData[]		frame_data // data for each frame in which this Surfel appears.
int[]			neighbours // Neighbours of this surfel across all frames
vec3			tangent    // indicator for cross field

FrameData {
	int 	frame_id 						// The frame that it appears in
	mat33	Transformation from (0,1,0) for normal
}


## Processing
Each depth image we process contains points with stronger and weaker validity. For example, the depth of a point on a boundary of a depth change is more suspect than in th middle of a homogeneous area.  Image procssing should then include:

For each new image

	Remove 'unreliable' pixels
		These are pixels which :
			Have a depth greater than 'n' for n large.
			Are adjacent to a depth discontinuity greater than 'd'
			Have a surface normal at 'too great' an angle to the camera
				Backproject using camera properties
				Compute normal from 8-connected neighbours


	Establish pixel correspondences to previous frame(s)
		Load related vertex map image
		For each pixel in the vertex map
			If pixel has no associated vertex, continue.
			If the associated vertex id has already been seen
				Mark this frame/pixel as being in correspondence with every frame/pixel with the same vertex index
			Else
				Add this frame/pixel/vertex combo to the list.
		Next



	For each pixel in the image:
		If it's 'new' 
			Allocate space in the list for it and ref it's frame/pixel coords
		Else
			Make a note of 
		End
	End If
Next image