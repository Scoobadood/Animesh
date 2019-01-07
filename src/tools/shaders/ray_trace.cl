
struct Ray {
	float3 origin;
	float3 direction;
};

struct Ray createCamRay(const int x_coord, const int y_coord, const int width, const int height){

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
 ray.origin = (float3)(0.0f, 0.0f, 40.0f); /* fixed camera position */
 ray.direction = normalize(pixel_pos - ray.origin);

 return ray;
}

intersectRayWithMesh( const struct Ray ray, float3 * v1, float3 * v2, float3 * v3, float3 * intersection ) {
	return false;
}

/**
 * Perform raytracing
 */
__kernel void ray_trace(__global float* depth, __global unsigned int* vertex, int width, int height) {
	// Get x,y coord of pixel
 	const int work_item_id = get_global_id(0);
	int x = work_item_id % width; /* x-coordinate of the pixel */
 	int y = work_item_id / width; /* y-coordinate of the pixel */

 	// Create the ray
 	struct Ray ray = createCamRay(x, y, width, height);

 	// Intersect the ray with the mesh
 	float3 v1, v2, v3;
 	float3 intersection;
 	bool hit = intersectRayWithMesh(ray, &v1, &v2, &v3, &intersection);

 	// Compute the depth at this point
 	depth[work_item_id] = hit ? intersection.z : 0.0;

 	// Compute the nearest vertex at this point
 	vertex[work_item_id] = 1;
}

