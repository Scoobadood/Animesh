# Fix facewise load bug

Bug out when loading cloth facewise.

Bug is in update_neighbours_layer when transitioning from one frame to another.

With cloth2 sequence, the problem seems to be that adjacency and frame[0] pointnormals have 256 elements while frame 1 has 289.
This corresponds to 16x16 faces = 256 graph nodes but in frame 1, 17x17 vertices in frame 1
This suggests initialing future frames is buggy.

Bug was happening because we were loading 2nd and subsequent frames of a multi-file load using 'false' for facewise as we'd forgotten a parameter.
