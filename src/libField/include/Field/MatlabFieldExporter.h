#pragma once

#include <ostream>

#include "FieldExporter.h"

namespace animesh {

class MatlabFieldExporter : public FieldExporter {
public:
	/** 
	 * Constructor 
	 */
	MatlabFieldExporter( std::ostream& out );


	/**
	 Destructure
	 */
	~MatlabFieldExporter( );

	/**
 	 * Export the field
 	 */
	void exportField( const Field& field ) const override;


private:
	void writeHeader( std::ostream& out ) const;
	void writeNormalData( std::ostream& out, std::vector<const FieldElement *>& elements ) const;
	void writeLocationData( std::ostream& out, std::vector<const FieldElement *>& elements ) const;
	void writeTangentData( std::ostream& out, std::vector<const FieldElement *>& elements ) const;
	void writeInt( std::ostream& out, int i ) const;
	void writeShort( std::ostream& out, short s ) const;
	void writeChar( std::ostream& out, char c) const;
	void writeVector3f( std::ostream& out, const Eigen::Vector3f& vector ) const;
	int writeVectorsHeader( std::ostream& out, const char * const name,  unsigned int numVectors ) const;

	/** Steram to write to */
	std::ostream& 		m_out;
};	

}