#pragma once 

#include <Graph/Graph.h>
#include <Eigen/Core>

#include <Field/FieldElement.h>

namespace animesh {

using FieldGraph = Graph<FieldElement *, void *>;
using FieldGraphNode = typename Graph<FieldElement *, void *>::GraphNode;

// Fwd decl For point comparison
struct cmpPointNormal;

/**
 * A Field is the collection of FieldElements across multuiple timepoints
 * The spatial relationship is modeled in a Graph and the temporal relationship
 * is modeled in a series of maps.
 */
class Field {

public:
	/**
	 * Construct a Field given vectors of vertices and their normals as well as adjacency.
	 */
	Field( const std::vector<Eigen::Vector3f>& vertices, const std::vector<Eigen::Vector3f>& normals, const std::vector<std::vector<size_t>>& adjacency );

	~Field( );

	/**
	 * @return the size of the field (number of FieldElements)
	 */
	std::size_t size() const;

	/**
	 * @return vector of elements in frame.
	 */
	const std::vector<FieldElement *>& 
	elements( size_t frame_idx ) const;


	/* ******************************************************************************************
	 * *
	 * *  Multi Frame Support
	 * * 
	 * ******************************************************************************************/

	/**
	 * Add a point cloud for another frame
	 * @param cloud The point cloud to be added.
	 */ 
	void
	add_new_frame(const std::vector<Eigen::Vector3f>& vertices, const std::vector<Eigen::Vector3f>& normals);

	/**
	 * Get the vector elements neighbouring the given node at the specific time point.
	 * @param fe The FieldElement who's neighbours should be returned
	 * @param time_point The neighbouring FieldElements
	 * @return vector of elements 
	 */
	std::vector<FieldElement *> get_neighbours_of( FieldElement * fe, size_t frame_idx ) const;

	/**
	 * @return the number of frames in this field
	 */
	inline size_t get_num_frames() const { return m_frame_data.size(); }


	/* ******************************************************************************************
	 * *
	 * *  IO
	 * * 
	 * ******************************************************************************************/
	void dump( ) const;

	void enable_tracing( bool enable_tracing ) { m_tracing_enabled = enable_tracing;}

	inline bool is_tracing_enabled( ) const { return m_tracing_enabled; }

	/** The graph of the nodes constructed over the first pointcloud */
	FieldGraph *							m_graph;

	/** Point data for each frame();								*/
	std::vector<std::vector<FieldElement *>> m_frame_data;

private:
	/**
	 *  Throw an exception if the frame is out of range
	 * @param frame
	 */
	void inline check_frame( size_t frame ) const {
        if (frame > get_num_frames()) {
        	std::string err = "Frame out of range : " + std::to_string(frame);
            throw std::invalid_argument(err);
        }
    }

    /** Flag to determine if we should trace field moothing */
	bool 											m_tracing_enabled;

	/** Maintain a mapping between FieldElements and GraphNodes */
	std::map<FieldElement *, FieldGraphNode *>		m_graphnode_for_field_element;
};
/* ******************************************************************************************
 * *
 * *  Utility Functions
 * * 
 * ******************************************************************************************/

std::ostream& operator<<( std::ostream& os, const Eigen::Vector3f& fe);
}