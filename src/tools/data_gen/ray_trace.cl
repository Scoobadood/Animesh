
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

Ray createCamRay(const int x_coord, const int y_coord, const int width, const int height, __global Camera * cam);
bool intersectRayWithFace(const Ray ray,
                          const float3 v0,
                          const float3 v1,
                          const float3 v2,
                          const float3 face_normal,
                          float * p_distance );
bool intersectRayWithMesh( const Ray ray,					
                           const int numVertices,
                           __global float3 * vertices,
                           const int numFaces,
                           __global int3 * faces,
                           __global float3 * faceNormals,
                           float * distance,
                           int3 * intersectedFace );


Ray createCamRay(const int x_coord, const int y_coord, const int width, const int height, __global Camera * cam) {
	/* create a local coordinate frame for the camera */
	float3 rendercamview = normalize(cam->view); 
	float3 rendercamup = normalize(cam->up); 
	float3 horizontalAxis = normalize(cross(rendercamview, rendercamup));
	float3 verticalAxis = normalize(cross(horizontalAxis, rendercamview));

	float3 middle = cam->position + rendercamview;
	float3 horizontal = horizontalAxis * tan(cam->fov.x * 0.5f * (PI / 180)); 
	float3 vertical   =  verticalAxis * tan(cam->fov.y * -0.5f * (PI / 180)); 

	int pixelx = x_coord; 
	int pixely = height - y_coord - 1;

	float sx = (float)pixelx / (width - 1.0f);
	float sy = (float)pixely / (height - 1.0f);
	
	float3 pointOnPlaneOneUnitAwayFromEye = middle + (horizontal * ((2 * sx) - 1)) + (vertical * ((2 * sy) - 1));
	float3 pointOnImagePlane = cam->position + ((pointOnPlaneOneUnitAwayFromEye - cam->position) * cam->focalDistance); /* cam->focalDistance */

	float3 aperturePoint = cam->position;
	float3 apertureToImagePlane = pointOnImagePlane - aperturePoint; apertureToImagePlane = normalize(apertureToImagePlane); 

	/* create camera ray*/
	Ray ray;
	ray.origin = aperturePoint;
	ray.direction = apertureToImagePlane; 

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
bool intersectRayWithFace(const Ray ray,
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
    float3 h = cross(ray.direction,edge2);
    float a = dot(edge1, h);
    if (a > -EPSILON && a < EPSILON) {
        return false;
    }

    float f = 1.0/a;
    float3 s = ray.origin - v0;
    float u = f * (dot(s, h));
    if (u < 0.0 || u > 1.0) {
        return false;
    }

    float3 q = cross(s,edge1);
    float v = f * dot(ray.direction, q);
    if (v < 0.0 || u + v > 1.0) {
        return false;
    }

    // Here we know there's an intersection
    // At this stage we can compute t to find out where the intersection point is on the line.
    float t = f * dot(edge2,q);
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
bool intersectRayWithMesh( const Ray ray,					
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
		if ( intersectRayWithFace(ray, v0, v1, v2, face_normal, &distance)) {
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

/**
 * Perform raytracing
 */
__kernel void ray_trace(
    int numVertices,  __global float3 * vertices,	/* Vertex data for model */
    int numFaces, __global int3 * faces,			/* Face data for model */
    __global float3 * faceNormals,
    __global Camera * camera,						/* Camera position and orientation */
    __global float* depth,							/* Output depth data */
    __global unsigned int* vertex,					/* Ouptut vertex data  */
    int width, int height) {						/* Dimensions of output data */

	const int work_item_id = get_global_id(0);
	const int x = work_item_id % width; /* x-coordinate of the pixel */
	const int y = work_item_id / width; /* y-coordinate of the pixel */

	// Create the ray from cam to view
	Ray ray = createCamRay(x, y, width, height, camera);

	// Intersect the ray with the mesh
	float range;
	int3 intersectedFace;
	if( intersectRayWithMesh(ray, numVertices, vertices, numFaces, faces, faceNormals, &range, &intersectedFace)) {
		// Compute the depth at this point
		depth[work_item_id] = range;

		// Compute _nearest_ vertex index
		float3 intersection = ray.origin + (range * ray.direction);
		float3 v0 = vertices[intersectedFace.s0];
		float3 v1 = vertices[intersectedFace.s1];
		float3 v2 = vertices[intersectedFace.s2];
		float d0 = distance(intersection, v0);
		float d1 = distance(intersection, v1);
		float d2 = distance(intersection, v2);
		int idx;
		if( d0 < d1 ) {
			if( d0 < d2) {
				idx = intersectedFace.s0 + 1;
			} else {
				idx = intersectedFace.s2 + 1;
			}
		} else {
			if( d1 < d2 ) {
				idx = intersectedFace.s1 + 1;
			} else {
				idx = intersectedFace.s2 + 1;
			}
		}
		vertex[work_item_id] = idx;
	} else {
		// Ray mised mesh
		depth[work_item_id] = 0.0;
		vertex[work_item_id] = 0;
	}
}

