#pragma once

#include <Eigen/Core>
#include "Graph.h"

struct FieldElement {
	Eigen::Vector3f		m_location;
	Eigen::Vector3f		m_normal;
	Eigen::Vector3f		m_tangent;

	FieldElement( Eigen::Vector3f location, 
				  Eigen::Vector3f normal,
				  Eigen::Vector3f tangent ) : m_location{ location }, m_normal{ normal }, m_tangent{ tangent } {};
};


/*
 * GraphBuilder is responsible for building a graph given a set of input element data
 * The input data should comprise a set of Elements each of which supports methods for
 * obtaining location and normal.  The builder will output a graph based on it's own
 * criteria.
 */
template <class EdgeData>
class GraphBuilder {
public:
	/**
	 * Virtual destructur
	 */
	virtual ~GraphBuilder( ) {};

	/**
	 * Build a graph given the input vector of Elements
	 */
	virtual Graph<FieldElement *, EdgeData> * build_graph_for_elements( const std::vector<Element>& elements ) const = 0;
};