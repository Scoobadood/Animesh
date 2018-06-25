#pragma once

#include <Eigen/Core>
#include "Element/Element.h"
#include "HierarchicalGraph.h"

struct FieldElement {
	Eigen::Vector3f		m_location;
	Eigen::Vector3f		m_normal;
	Eigen::Vector3f		m_tangent;

	FieldElement( Eigen::Vector3f location, 
				  Eigen::Vector3f normal,
				  Eigen::Vector3f tangent ) : m_location{ location }, m_normal{ normal }, m_tangent{ tangent } {};
	/**
 	 * Useful method for merging FieldElements
	 */
	static FieldElement * mergeFieldElements ( const FieldElement* fe1, const FieldElement* fe2 ) {
		FieldElement *fe = new FieldElement(
			(fe1->m_location + fe2->m_location) / 2.0,
			(fe1->m_normal + fe2->m_normal).normalized(),
			Eigen::Vector3f::Zero());

		// randomize tangent
		Eigen::Vector3f random = Eigen::VectorXf::Random(3);
		fe->m_tangent = random.cross( fe->m_normal ).normalized( );
		return fe;
	}

	/**
 	 * Propagate field element changes down graph hierarch
	 */
	static FieldElement * propagateFieldElements ( FieldElement* parent, FieldElement* child ) {
		// Take the tangent from the parent and reproject into the child's tangent space
		// and normalise
		Eigen::Vector3f error = parent->m_tangent.dot( child->m_normal ) * child->m_normal;
		Eigen::Vector3f child_tangent = parent->m_tangent - error;
		child->m_tangent = child_tangent.normalized();
		return child;
	}
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
	virtual animesh::Graph<FieldElement *, EdgeData> * build_graph_for_elements( const std::vector<Element>& elements ) const = 0;
};