#Design
##Separate Smoothing

###Rationale
Smoothing is an operation that is performed on a field and which changes it. There may be a number of operations that we want to perform on a field (compute location field for example) or multiple ways in which we want to perform them.

Having smoothing baked into the field class as a process means we are carrying a bunch of methods and fields that really are not to do with the field (ie. current smoothing iteration etc).

We should remove the smoothing function from the Field and make it an external algorithm that operates on a Field.

###Process

First we need to shortlist the elements of Field that are touched by smoothing and determine a strategy for accessing/updating them

Next identify all of the smoothing related fields and methods and move them to another class
- Make sure to call appropriate data accessors in Field's new public interface.

###Results
#### Methods related to smoothing
* void smooth( );
* void smooth_completely();
* float current_error( int tier );
* num_tiers( ) const;
* FieldGraph * graph_at_tier( size_t tier ) const;
* void start_smoothing( );
* bool smooth_tier_once( FieldGraph * tier );
* bool check_convergence( float new_error );
* Eigen::Vector3f calculate_smoothed_node( FieldGraph * tier, FieldGraphNode * const gn ) const;
* float error_for_node( FieldGraph * tier, FieldGraphNode * const gn ) const;
* void generate_hierarchy( size_t max_nodes );
* float calculate_error_for_node( FieldGraph * tier, FieldGraphNode * gn ) const;
* float calculate_error( FieldGraph * tier ) const;

#### Fields related to smoothing
* std::vector<FieldGraph *>		m_graph_hierarchy;
* std::vector<FieldGraphMapping>	m_mapping_hierarchy;
* bool 									m_tracing_enabled;
* bool									m_is_smoothing;
* bool									m_smoothing_started_new_tier;
* float									m_smoothing_last_error;
* int 									m_smoothing_iterations_this_tier;
* int 									m_smoothing_tier_index;
* animesh::Graph<FieldElement *, void*> *	m_smoothing_current_tier;

#### Field private elements touched by smoothing.






The Cross Field/Direction Field is a property of the mesh that we load. We compute the cross field.
We may use a hierarchy to do that but the hierarchy is NOT an inherent part of the field
But we still want to visualise it sometimes

BTW We still aren't working properly for the poly field

