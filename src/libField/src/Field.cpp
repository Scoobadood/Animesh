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
 * Find correspondences between FieldElements in first set and second set.
 */
void animesh::find_correspondences(
        const std::vector<FieldElement *> &first,
        const std::vector<FieldElement *> &second,
        Field::Correspondence &corr) {

    if (first.size() == 0)
        throw std::invalid_argument("find_correspondences expects first vector to have non-zero size");

    if (second.size() == 0)
        throw std::invalid_argument("find_correspondences expects second vector to have non-zero size");

    // Naive implementation assumes a one-to-one, ordered correspondence.
    if (first.size() != second.size())
        throw std::invalid_argument("find_correspondences expects vectors to be the same size");

    for (size_t i = 0; i < first.size(); ++i) {
        corr.insert(std::make_pair(first[i], std::make_pair(second[i], Eigen::Matrix3f::Identity())));
    }
}

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


/**
 * Compute the transformation for a given node
 * @param fe The source FieldElement
 * @param time_point The time point from which to recover the corresponding point
 */
void animesh::compute_temporal_transform(FieldElement *node,
                                         const std::vector<FieldElement *> &neighbours,
                                         Field::Correspondence &corr) {
    using namespace std;
    using namespace Eigen;

    Vector3f point1 = node->location();
    Vector3f normal1 = node->normal();

    const FieldElement *const node_at_t1 = corr.at(node).first;
    Vector3f point2 = node_at_t1->location();
    Vector3f normal2 = node_at_t1->normal();

    // Find points for neighbours at both time intervals
    vector<Vector3f> neighbours1;
    vector<Vector3f> neighbours2;
    for (auto fe : neighbours) {
        neighbours1.push_back(fe->location());
        neighbours2.push_back(corr.at(fe).first->location());
    }

    Eigen::Matrix3f m = rotation_between(point1, normal1, neighbours1,
                                         point2, normal2, neighbours2);
    corr.at(node).second = m;
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
    m_elements = make_field_elements_from_point_cloud(cloud);

    // And add to graph, maintaining a mapping
    size_t fe_idx = 0;
    map<PointNormal, FieldGraphNode *, cmpPointNormal> *point_to_gn_map = new map<PointNormal, FieldGraphNode *, cmpPointNormal>();

    for (auto point : *cloud) {
        FieldGraphNode *gn = m_graph->add_node(m_elements[fe_idx]);

        // Keep a (long term) mapping from FE to GN
        m_graphnode_for_field_element[m_elements[fe_idx]] = gn;

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
std::size_t Field::size() const {
    return m_graph->num_nodes();
}

/**
 * Return vector of elements
 */
const std::vector<const FieldElement *> Field::elements() const {
    std::vector<const FieldElement *> elements;

    for (auto node : m_graph->nodes()) {
        elements.push_back(node->data());
    }
    return elements;
}


/* ******************************************************************************************
 * *
 * *  Multi Frame Support
 * *
 * ******************************************************************************************/

/**
 * Add a point cloud for the next point in time.
 * @param cloud The point cloud to be added for the next time point.
 */
void Field::add_new_timepoint(const pcl::PointCloud<pcl::PointNormal>::Ptr cloud) {
    // Construct a vector of FieldElements from points in the cloud
    std::vector<FieldElement *> new_elements = make_field_elements_from_point_cloud(cloud);

    // Find correspondences between t_0 and this time point
    Correspondence corr;
    find_correspondences(m_elements, new_elements, corr);

    // For each FE in the t_0 cloud, compute the xform to the FE in the t_n cloud
    for (auto gn : m_graph->nodes()) {
        // 1. Get the neighbours of this FE according to graph
        FieldElement *fe = gn->data();
        std::vector<FieldElement *> neighbours_0 = m_graph->neighbours_data(gn);
        compute_temporal_transform(fe, neighbours_0, corr);
    }

    m_correspondences.push_back(corr);
}

/**
 * Get the vector elements neighbouring the given node at the specific time point.
 * @param fe The FieldElement who's neighbours should be returned
 * @param time_point The neighbouring FieldElements
 * @return vector of elements
 */
std::vector<FieldElement *> Field::get_neighbours_of(FieldElement *fe, int time_point) const {
    using namespace std;

    check_time_point(time_point);
    FieldGraphNode *gn = m_graphnode_for_field_element.at(fe);
    vector<FieldElement *> t0_neighbours = m_graph->neighbours_data(gn);
    if (time_point == 0) {
        return t0_neighbours;
    }

    if (time_point >= m_correspondences.size())
        throw std::invalid_argument("Time point out of range");

    Correspondence c = m_correspondences[time_point];
    vector<FieldElement *> future_neighbours;
    for (auto nfe : t0_neighbours) {
        FutureFieldElementAndRotation f = c[nfe];
        future_neighbours.push_back(f.first);
    }

    return future_neighbours;
}

/**
 * Get the point corresponding to this one at a given time point
 * @param fe The source FieldElement
 * @param time_point The time point from which to recover the corresponding point. should be 1
 * @return The corresponding element
 */
FieldElement *const Field::get_point_corresponding_to(FieldElement * const fe, int time_point) const {
    check_time_point(time_point);
    if( time_point == 0 ) return fe;

    Correspondence c = m_correspondences[time_point];
    FutureFieldElementAndRotation f = c.at(const_cast<FieldElement *>(fe));

    return f.first;
}

std::vector<FieldElement *> Field::get_all_n_at_t0() const {
    std::vector<FieldElement *> v{};
    return v;
}

// Total number of time points. Current time point is 0. If there is one additional timepoint, we return 2
size_t Field::get_num_timepoints() const {
    return m_correspondences.size();
}

/**
 * Get the Matrix which transforms a given FE into it's position at time t
 * @param fe The FieldElement to map
 * @param t The timepoint (0 for now, >0 for future
 * @return The mtransformation matrix
 */
Eigen::Matrix3f Field::get_fwd_xform_for(const FieldElement *const fe, int time_point) const {
    check_time_point(time_point);
    if( time_point == 0 ) return Eigen::Matrix3f::Identity();

    Correspondence c = m_correspondences[time_point - 1];
    FutureFieldElementAndRotation f = c.at(const_cast<FieldElement *>(fe));

    return f.second;
}
