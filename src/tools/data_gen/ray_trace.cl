
__constant float PI = 3.14159265359f;

// A ray in 3D space
typedef struct Ray {
	float3 origin;
	float3 direction;
} Ray;

typedef struct Camera{
	float3 position;		
	float3 view;			
	float3 up;			
	float2 resolution;	
	float2 fov;		
	float focalDistance;
} Camera;

Ray create_cam_ray(const int x_coord, const int y_coord, const int width, const int height, __global Camera * cam);
bool intersect_ray_with_face(const Ray ray,
                          const float3 v0,
                          const float3 v1,
                          const float3 v2,
                          const float3 face_normal,
                          float * p_distance );
bool intersect_ray_with_mesh( const Ray ray,					
                           const int numVertices,
                           __global float3 * vertices,
                           const int numFaces,
                           __global int3 * faces,
                           __global float3 * faceNormals,
                           float * distance,
                           int3 * intersectedFace );
int closest_vertex(const int3 face, __global float3 * vertices, const float3 intersection );
void construct_camera_coordinate_system(__global Camera * cam, float3 * origin, float3 * n, float3 * u, float3 * v);
void construct_image_plane_origin(const float2 fov, 
								  const float focal_length,
								  const float3 camera_origin, 
								  const float3 n, 
								  const float3 u, 
								  const float3 v, 
								  float3 * image_plane_origin,
								  float2 * image_plane_dimensions);



/**
 * Construct an eye coordinate system
 * Input: camera position, center of interest, view-up vector
 * Returns: new origin and three basis vectors
 *
 *               /|
 *              / |
 *             /  |
 *            /   |
 *           / ^  |
 *          / v|  |
 *          |  |-----> n
 *          | /  /
 *          |Lu /
 *          |  /
 *          | /
 *          |/
 *
 */
void construct_camera_coordinate_system(__global Camera * cam, float3 * origin, float3 * n, float3 * u, float3 * v) {
	// n is normal to image plane, points in opposite direction of view point
	float3 N = cam->position - cam->view;
	*n = normalize(N);

	// u is a vector that is perpendicular to the plane spanned by
	// N and view up vector (cam->up)
	float3 U = cross(cam->up, *n);
	*u = normalize(U);

	// v is a vector perpendicular to N and U
	*v = cross(*n, *u);

	// origin is cam centre
	*origin = cam->position;
}

void construct_image_plane_origin(const float2 fov, 
								  const float  focal_length,
								  const float3 camera_origin, 
								  const float3 n, 
								  const float3 u, 
								  const float3 v, 
								  float3 * image_plane_origin,
								  float2 * image_plane_dimensions) {
	float image_plane_height = tan(fov.y * 0.5f * (PI / 180)) * 2.0f * focal_length;
	float image_plane_width  = tan(fov.x * 0.5f * (PI / 180)) * 2.0f * focal_length;

	float3 image_plane_centre = camera_origin - ( n * focal_length );
	*image_plane_origin = image_plane_centre - (u * image_plane_width * 0.5f) - (v * image_plane_height * 0.5f);
	image_plane_dimensions->x = image_plane_width;
	image_plane_dimensions->y = image_plane_height;
}

/**
 * x_coord in image plane
 * y_coord   --"--
 * width, height (of image)
 * 
 */
Ray create_cam_ray(const int x_coord, const int y_coord, const int width, const int height, __global Camera * cam) {
	/* create a local coordinate frame for the camera */
	float3 origin;
	float3 u;
	float3 v;
	float3 n;
	construct_camera_coordinate_system(cam, &origin, &n, &u, &v);

	float3 image_plane_origin;
	float2 image_plane_dimensions;
	construct_image_plane_origin(cam->fov, cam->focalDistance, origin, n, u, v, &image_plane_origin, &image_plane_dimensions);

	float pixel_width  = image_plane_dimensions.x / width;
	float pixel_height = image_plane_dimensions.y / height;
	float3 pixelCamCoordinate = image_plane_origin + (x_coord * pixel_width * u) + (y_coord * pixel_height * v);

	/* create camera ray*/
	Ray ray;
	ray.origin = origin;
	ray.direction = normalize(pixelCamCoordinate - origin);

	return ray;
}



/**
 * Determine if a ray intersects a particular triangle.
 * @param ray The ray
 * @param v0 The vertices of the triangle
 * @param v1 The vertices of the triangle
 * @param v2 The vertices of the triangle
 * @param face_normal The normal to the face
 * @param p_distance Pointer to the distance from ray origin of intersection.
 * @return true if the ray intersects the triangle. Otherwise false.
 */
bool intersect_ray_with_face(const Ray ray,
                          const float3 v0,
                          const float3 v1,
                          const float3 v2,
                          const float3 face_normal,
                          float * p_distance ) {
    const float EPSILON = 0.0000001;

    // Eliminate faces which face away from POV
    float  angle = dot(face_normal, ray.direction);
    if( angle > 0) {
    	return false;
    }

    // Determine if there's an intersection
    float3 edge1 = v1 - v0;
    float3 edge2 = v2 - v0;

    // Check if ray is parallel to the face
    float3 pvec = cross(ray.direction,edge2);
    float det = dot(edge1, pvec);
    if (det > -EPSILON && det < EPSILON) {
        return false;
    }

    float inv_det = 1.0/det;
    float3 tvec = ray.origin - v0;
    float u = inv_det * (dot(tvec, pvec));
    if (u < 0.0 || u > 1.0) {
        return false;
    }

    float3 qvec = cross(tvec,edge1);
    float v = inv_det * dot(ray.direction, qvec);
    if (v < 0.0 || u + v > 1.0) {
        return false;
    }

    // Here we know there's an intersection
    // At this stage we can compute t to find out where the intersection point is on the line.
    float t = inv_det * dot(edge2, qvec);
    if (t > EPSILON) {
    	*p_distance = t;
        return true;
    }

 	// This means that there is a line intersection but not a ray intersection.
 	// i.e. the ray intersects 'behind' the ray origin
    else {
        return false;
    }
}


/**
 * Compute intersection of ray with mesh
 * @param ray The ray to intersect
 * @param numVertices The number of vertices in the mesh
 * @param vertices The vertex coordinates as float3
 * @param numFaces The number of faces in the mesh
 * @param faces The faces, represented as three indices into vertices
 * @param faceNormals The face normals
 * @param distance The range to any intersection found
 * @param intersectedFace The face that was intersected if there was one.
 * @return true If there was an intersection else false.
 */
bool intersect_ray_with_mesh( const Ray ray,					
                           const int numVertices,
                           __global float3 * vertices,
                           const int numFaces,
                           __global int3 * faces,
                           __global float3 * face_normals,
                           float * p_distance,
                           int3 * p_intersected_face ) {

	bool  did_intersect = false;
	float closest_intersection = INFINITY;
	int3  intersected_face;

	for (int i = 0; i < numFaces; i++ ) {
		int3 face = faces[i];
		float3 face_normal = face_normals[i];
		float distance;
		float3 v0, v1, v2;
		v0 = vertices[face.s0];
		v1 = vertices[face.s1];
		v2 = vertices[face.s2];
		if ( intersect_ray_with_face(ray, v0, v1, v2, face_normal, &distance)) {
			if( distance < closest_intersection ) {
				closest_intersection = distance;
				intersected_face = face;
				did_intersect = true;
			}
		}
	}

	*p_distance = closest_intersection;
	*p_intersected_face = intersected_face;
	return did_intersect;
}

int closest_vertex(const int3 face, __global float3 * vertices, const float3 intersection ) {
		float3 v0 = vertices[face.s0];
		float3 v1 = vertices[face.s1];
		float3 v2 = vertices[face.s2];
		float d0 = distance(intersection, v0);
		float d1 = distance(intersection, v1);
		float d2 = distance(intersection, v2);
		int idx = (( d0 < d1 ) && (d0 < d2)) 
			? face.s0 + 1
			: (( d1 < d0 ) && (d1 < d2)) 
				? face.s1 + 1
				: face.s2 + 1;
		return idx;
}
/**
 * Perform raytracing
 */
__kernel void ray_trace(
    int num_vertices,  __global float3 * vertices,	/* Vertex data for model */
    int num_faces, __global int3 * faces,			/* Face data for model */
    __global float3 * face_normals,
    __global Camera * camera,						/* Camera position and orientation */
    __global float* depth,							/* Output depth data */
    __global unsigned int* vertex,					/* Ouptut vertex data  */
    int width, int height) {						/* Dimensions of output data */

	const int work_item_id = get_global_id(0);
	const int x = work_item_id % width; /* x-coordinate of the pixel */
	const int y = work_item_id / width; /* y-coordinate of the pixel */

	// Create the ray from cam to view
	Ray ray = create_cam_ray(x, y, width, height, camera);

	// Intersect the ray with the mesh
	float range;
	int3 intersected_face;
	if( intersect_ray_with_mesh(ray, num_vertices, vertices, num_faces, faces, face_normals, &range, &intersected_face)) {
		// Compute the depth at this point
		depth[work_item_id] = range;

		// Compute _nearest_ vertex index
		float3 intersection = ray.origin + (range * ray.direction);
		int idx = closest_vertex(intersected_face, vertices, intersection);
		vertex[work_item_id] = idx;
	} else {
		// Ray mised mesh
		depth[work_item_id] = 0.0;
		vertex[work_item_id] = 0;
	}
}

