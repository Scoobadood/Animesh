#Leggings Tool
This tool generates a Surfel graph for n frames of animated leg data given two inputs
The first is the PLY file containing the raw mesh data
The second is an animation description TXT file which has n frames
of animation data with vertex position data for each vertex in the PLY file for each frame


Format of the TXT file is
```
N        // Integer num frames
x, y, z  // Float vertex 0 in frame 0
x, y, z  // Float vertex 1 in frame 0
...
x, y, z  // Float vertex M-1 in frame 0
x, y, z  // Float vertex 0 in frame 1
...
x, y, z  // Float vertex M-1 in frame 1
...
x, y, z  // Float vertex M-1 in frame N-1
```
The tool should be run as follows:
```
leg_tool -p ply_file_name 
         -a animation_file_name 
         [-f 1,2,3] 
         [-s surfel_file_name]
```
where -f is the frames to generate.
