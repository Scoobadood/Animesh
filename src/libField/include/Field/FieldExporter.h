#pragma once

#include <Field/Field.h>

namespace animesh {

class FieldExporter {
public:
	/** 
	 * Pure virtual destructor
	 */
	virtual ~FieldExporter( ) {};

public:	
	/**
	 * Export the field
	 */
	virtual void exportField( Field& field ) const =0;
};
}