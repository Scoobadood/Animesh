#include <Field/Field.h>
#include <Field/FieldElement.h>
#include <Geom/geom.h>

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <pcl/kdtree/kdtree_flann.h>

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
bool operator==(const pcl::PointNormal &point1, const pcl::PointNormal &point2) {
    return (point1.x == point2.x) &&
           (point1.y == point2.y) &&
           (point1.z == point2.z) &&
           (point1.normal_x == point2.normal_x) &&
           (point1.normal_y == point2.normal_y) &&
           (point1.normal_z == point2.normal_z);
}

/*
 * Comparator for pcl::PointNormal instances
 */
struct animesh::cmpPointNormal {
    bool operator()(const pcl::PointNormal &a, const pcl::PointNormal &b) const {
        if (a.x < b.x) return true;
        if (a.x > b.x) return false;
        if (a.y < b.y) return true;
        if (a.y > b.y) return false;
        if (a.z < b.z) return true;
        if (a.z > b.z) return false;

        if (a.normal_x < b.normal_x) return true;
        if (a.normal_x > b.normal_x) return false;
        if (a.normal_y < b.normal_y) return true;
        if (a.normal_y > b.normal_y) return false;
        if (a.normal_z < b.normal_z) return true;

        return false;
    }
};

/**
 * Construct a vector of FieldElements from a point cloud of PointNormals
 */
std::vector<FieldElement *>
make_field_elements_from_point_cloud(const pcl::PointCloud<pcl::PointNormal>::Ptr cloud) {
    std::vector<FieldElement *> elements;
    for (auto point : *cloud) {
        elements.push_back(FieldElement::from_point(point));
    }
    return elements;
}


/* ******************************************************************************************
 * **
 * **  Constructor
 * **
 * ******************************************************************************************/

/**
 * Add points to graph while maintaining a map from point to Graphnode
 * @return the mapping
 */
std::map<pcl::PointNormal, FieldGraphNode *, animesh::cmpPointNormal> *
Field::add_points_from_cloud(const pcl::PointCloud<pcl::PointNormal>::Ptr cloud) {
    using namespace std;
    using namespace pcl;

    // Convert points to FieldElements
    assert(m_frame_data.size() == 0 );
    m_frame_data.push_back( make_field_elements_from_point_cloud(cloud));

    // And add to graph, maintaining a mapping
    size_t fe_idx = 0;
    map<PointNormal, FieldGraphNode *, cmpPointNormal> *point_to_gn_map = new map<PointNormal, FieldGraphNode *, cmpPointNormal>();

    for (auto point : *cloud) {
        FieldGraphNode *gn = m_graph->add_node(m_frame_data[0][fe_idx]);

        // Keep a (long term) mapping from FE to GN
        m_graphnode_for_field_element[m_frame_data[0][fe_idx]] = gn;

        // Keep a mapping from point to gn
        (*point_to_gn_map)[point] = gn;
        fe_idx++;
    }
    if (m_tracing_enabled)
        cout << "Added " << point_to_gn_map->size() << " points" << endl;
    return point_to_gn_map;
}

/**
 * Construct a field given a point cloud
 */
Field::Field(const pcl::PointCloud<pcl::PointNormal>::Ptr cloud, int k, bool tracing_enabled) {
    using namespace pcl;
    using namespace std;

    m_tracing_enabled = tracing_enabled;

    // First make an empty FieldGraph
    m_graph = new FieldGraph();

    // Next add points, converting as necessary
    map<PointNormal, FieldGraphNode *, cmpPointNormal> *point_to_gn_map = add_points_from_cloud(cloud);

    // Set up for kNN
    KdTreeFLANN<PointNormal> kdtree;
    kdtree.setInputCloud(cloud);
    vector<int> knnNeighbourIndices(k);
    vector<float> knnNeighbourDistances(k);

    // Keep track of points visited
    set<PointNormal, cmpPointNormal> visited;

    // For each point, construct edges
    for (auto this_point : *cloud) {
        if (m_tracing_enabled)
            cout << "Processing point " << this_point << endl;

        // Look up GN
        FieldGraphNode *this_gn = point_to_gn_map->at(this_point);

        // Now find neighbours
        knnNeighbourIndices.clear();
        knnNeighbourDistances.clear();
        if (kdtree.nearestKSearch(this_point, k, knnNeighbourIndices, knnNeighbourDistances) > 0) {

            if (m_tracing_enabled) {
                cout << "  Found " << knnNeighbourIndices.size() << " neighbours :" << endl;
                size_t point_index = 0;
                for (size_t i : knnNeighbourIndices) {
                    PointNormal neighbour_point = cloud->points[i];
                    cout << "    Point: " << neighbour_point << ";  GN "
                         << point_to_gn_map->at(neighbour_point) << endl;
                }
                cout << "  Looking at neighbours" << endl;
            }

            // ... and add edges to them
            for (int i : knnNeighbourIndices) {
                PointNormal neighbour_point = cloud->points[i];
                if (m_tracing_enabled)
                    cout << "    next point is " << neighbour_point << endl;

                // Provided the eighbour isn't this point
                if (!(neighbour_point == this_point)) {
                    // If the neighbour GN is not found... throw
                    FieldGraphNode *neighbour_gn = point_to_gn_map->at(neighbour_point);

                    if (m_tracing_enabled)
                        cout << "    Adding edge to " << neighbour_point << endl;

                    // Construct edge
                    m_graph->add_edge(this_gn, neighbour_gn, 1.0f, nullptr);
                } else {
                    if (m_tracing_enabled)
                        cout << "    Ignoring me as my own neighbour" << endl;
                }
            }
        } else {
            throw std::runtime_error("No neighbours found for a point. This shouldn't happen");
        }
    }

    // FieldGraph is built
    cout << "Done" << endl;
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
 *
 * @param cloud The point cloud to be added for the next time point.
 */
void
Field::add_new_frame(const pcl::PointCloud<pcl::PointNormal>::Ptr cloud) {
    // Construct a vector of FieldElements from points in the cloud
    std::vector<FieldElement *> new_elements = make_field_elements_from_point_cloud(cloud);
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