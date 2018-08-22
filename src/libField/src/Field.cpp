#include <Field/Field.h>
#include <Field/FieldElement.h>
#include <Geom/geom.h>

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <iostream>
#include <queue>
#include <set>
#include <vector>

using animesh::Field;
using animesh::FieldElement;


using FieldGraph = animesh::Graph<FieldElement *, void *>;
using FieldGraphNode = typename animesh::Graph<FieldElement *, void *>::GraphNode;

/* ******************************************************************************************
 * **
 * **  Utility functions
 * **
 * ******************************************************************************************/
/**
 * Construct a vector of FieldElements from vectors of vertices and normals.
 */
std::vector<FieldElement *>
make_field_elements_from_point_data(const std::vector<Eigen::Vector3f>& vertices, const std::vector<Eigen::Vector3f>& normals ) {
    assert( vertices.size() != 0);
    assert( normals.size() == vertices.size());

    std::vector<FieldElement *> elements;

    for (size_t i=0; i<vertices.size(); ++i) {
        elements.push_back(new FieldElement( vertices[i], normals[i]));
    }
    return elements;
}


/* ******************************************************************************************
 * **
 * **  Constructor
 * **
 * ******************************************************************************************/

/**
 * Construct a Field given vectors of vertices and their normals as well as adjacency.
 */
Field::Field( const std::vector<Eigen::Vector3f>& vertices, const std::vector<Eigen::Vector3f>& normals, const std::vector<std::vector<size_t>>& adjacency ) {
    using namespace std;
    using namespace Eigen;

    // Preconditions
    assert( vertices.size() != 0);
    assert( normals.size() == vertices.size());
    assert( adjacency.size() == vertices.size());


    // First make an empty FieldGraph
    m_graph = new FieldGraph();

    // Convert data to FEs
    vector<FieldElement *> fes = make_field_elements_from_point_data(vertices, normals );
    m_frame_data.push_back(fes);

    for( auto fe : fes) {
        FieldGraphNode *gn = m_graph->add_node(fe);

        // Keep a (long term) mapping from FE to GN
        m_graphnode_for_field_element[fe] = gn;
    }

    // Add edges
    for(size_t src_node_idx=0; src_node_idx<vertices.size(); ++src_node_idx) {
        FieldElement * src_fe = fes[src_node_idx];
        FieldGraphNode * src_node = m_graphnode_for_field_element.at(src_fe);

        for(size_t adj_idx=0; adj_idx < adjacency[src_node_idx].size(); ++adj_idx) {
            size_t dest_node_idx = adjacency[src_node_idx][adj_idx];
            FieldElement * dest_fe = fes[dest_node_idx];
            FieldGraphNode * dest_node = m_graphnode_for_field_element.at(dest_fe);

            // Construct edge
            if( ! m_graph->has_edge(src_node, dest_node) ) {
                m_graph->add_edge(src_node, dest_node, 1.0f, nullptr);
            }
        }
    }
}


/**
 * Destructor for fields
 */
Field::~Field() {
    delete m_graph;
}

/**
 * @return the size of the ifled
 */
std::size_t
Field::size() const {
    return m_frame_data[0].size();
}

/**
 * Return vector of elements
 */
const std::vector<FieldElement *>&
Field::elements( size_t frame_idx) const {
    check_frame(frame_idx);
    return m_frame_data[frame_idx];
}


/* ******************************************************************************************
 * *
 * *  Multi Frame Support
 * *
 * ******************************************************************************************/

/**
 * Add a point cloud for the next point in time.
 * For now we assume that the second point cloud is the same size as the current one and that
 * points in the cloud are in correspondence with the current list of FieldElements
 */
void
Field::add_new_frame(const std::vector<Eigen::Vector3f>& vertices, const std::vector<Eigen::Vector3f>& normals) {
    std::vector<FieldElement *> new_elements = make_field_elements_from_point_data(vertices, normals);
    assert(new_elements.size() == m_frame_data[0].size() );
    m_frame_data.push_back(new_elements);
}

/**
 * Get the vector elements neighbouring the given node at the specific time point.
 * @param fe The FieldElement who's neighbours should be returned
 * @param frame_idx The neighbouring FieldElements
 * @return vector of elements
 */
std::vector<FieldElement *> 
Field::get_neighbours_of(FieldElement *fe, size_t frame_idx) const {
    using namespace std;

    check_frame(frame_idx);
    auto it = m_graphnode_for_field_element.find(fe);
    assert(it != m_graphnode_for_field_element.end());
    FieldGraphNode *gn = it->second;
    vector<FieldElement *> t0_neighbours = m_graph->neighbours_data(gn);
    if (frame_idx == 0) {
        return t0_neighbours;
    }

    vector<FieldElement *> future_neighbours;
    for (auto nfe : t0_neighbours) {
        auto it = std::find(m_frame_data[0].begin(), m_frame_data[0].end(), nfe);
        assert( it != m_frame_data[0].end());
        size_t idx = it - m_frame_data[0].begin();
        future_neighbours.push_back(m_frame_data[frame_idx][idx]);
    }

    return future_neighbours;
}