
struct Ray {
	float3 origin;
	float3 direction;
};

struct Ray createCamRay(const int x_coord, const int y_coord, const int width, const int height);
bool intersectRayWithFace(const struct Ray ray, __global float3 * vertices, const int3 face, float3 * intersection );
int intersectRayWithMesh( const struct Ray ray, const int numVertices, __global float3 * vertices, const int numFaces, __global int3 * faces, float3 * intersection );

struct Ray createCamRay(const int x_coord, const int y_coord, const int width, const int height) {

	float fx = (float)x_coord / (float)width;  /* convert int in range [0 - width] to float in range [0-1] */
	float fy = (float)y_coord / (float)height; /* convert int in range [0 - height] to float in range [0-1] */

	/* calculate aspect ratio */
	float aspect_ratio = (float)(width) / (float)(height);
	float fx2 = (fx - 0.5f) * aspect_ratio;
	float fy2 = fy - 0.5f;

	/* determine position of pixel on screen */
	float3 pixel_pos = (float3)(fx2, -fy2, 0.0f);

	/* create camera ray*/
	struct Ray ray;
	ray.origin = (float3)(0.0f, 0.0f, 50.0f); /* fixed camera position */
	ray.direction = normalize(pixel_pos - ray.origin);

	return ray;
}

bool intersectRayWithFace(const struct Ray ray,
                          __global float3 * vertices,
                          const int3 face,
                          float3 * intersection ) {
    const float EPSILON = 0.0000001;

    float3 vertex0 = vertices[face.x];
    float3 vertex1 = vertices[face.y];
    float3 vertex2 = vertices[face.z];

    float3 edge1, edge2, h, s, q;
    float a,f,u,v;
    edge1 = vertex1 - vertex0;
    edge2 = vertex2 - vertex0;

    h = cross(ray.direction,edge2);
    a = dot(edge1, h);
    if (a > -EPSILON && a < EPSILON)
        return false;    // This ray is parallel to this triangle.

    f = 1.0/a;
    s = ray.origin - vertex0;
    u = f * (dot(s, h));
    if (u < 0.0 || u > 1.0)
        return false;

    q = cross(s,edge1);
    v = f * dot(ray.direction, q);
    if (v < 0.0 || u + v > 1.0)
        return false;

    // At this stage we can compute t to find out where the intersection point is on the line.
    float t = f * dot(edge2,q);
    // ray intersection
    if (t > EPSILON) {
        *intersection = ray.origin + ray.direction * t;
        return true;
    }
    else // This means that there is a line intersection but not a ray intersection.
        return false;
}

int intersectRayWithMesh( const struct Ray ray,
                          const int numVertices,
                          __global float3 * vertices,
                          const int numFaces,
                          __global int3 * faces,
                          float3 * intersection ) {
	for (int f = 0; f < numFaces; f++ ) {
		if ( intersectRayWithFace(ray, vertices, faces[f], intersection)) {
			return f;
		}
	}
	return -1;
}

/**
 * Perform raytracing
 */
__kernel void ray_trace(
    int numVertices,
    __global float3 * vertices,
    int numFaces,
    __global int3 * faces,
    __global float* depth,
    __global unsigned int* vertex,
    int width, int height) {

	// Get x,y coord of pixel
	const int work_item_id = get_global_id(0);
	int x = work_item_id % width; /* x-coordinate of the pixel */
	int y = work_item_id / width; /* y-coordinate of the pixel */

	// Create the ray
	struct Ray ray = createCamRay(x, y, width, height);

	// Intersect the ray with the mesh
	float3 intersection;
	int face = intersectRayWithMesh(ray, numVertices, vertices, numFaces, faces, &intersection);

	// Compute the depth at this point
	if( face >= 0 ) {
		int3 f = faces[face];
		depth[work_item_id] = intersection.z;
		float3 v0 = vertices[f.x];
		float d0 = length(intersection - v0);

		float3 v1 = vertices[f.y];
		float d1 = length(intersection - v1);

		float3 v2 = vertices[f.z];
		float d2 = length(intersection - v2);

		vertex[work_item_id] = ((d0 > d1) && (d0 > d2)) 
			? f.x 
			: ((d1 > d0) && (d1 > d2)) 
				? f.y 
				: f.z;
	} else {
		depth[work_item_id] = 0.0;
		vertex[work_item_id] = 99;
	}
}

