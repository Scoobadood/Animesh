#pragma once 

#include <Field/Field.h>
#include <Args/Args.h>

void write_matlab_file( Field * field, const std::string& file_name );

void write_matlab_file( Field * field, int index );

/**
 * Construct a field
 */
Field * load_field( const Args& args);

/**
 * Write the field to Matlab
 */
void save_field( const Args& args, Field * field );