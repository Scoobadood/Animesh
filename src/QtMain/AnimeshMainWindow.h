#ifndef ANIMESHMAINWINDOW_H
#define ANIMESHMAINWINDOW_H

#include <QMainWindow>
#include <Field/FieldOptimiser.h>
#include <Field/ObjFileParser.h>

#include "vtkObject.h"
#include "vtkPolyData.h"
#include "vtkRenderer.h"
#include "vtkSmartPointer.h"


namespace Ui {
class AnimeshMainWindow;
}


class AnimeshMainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit AnimeshMainWindow(QWidget *parent = nullptr);

    ~AnimeshMainWindow();

private:
    Ui::AnimeshMainWindow *ui;

    /**
     * Load from one or more files or directory.
     * If file_names has more than one entry, they must all be files and we load them all
     * If it has one entry, it could be a directory or a file.
     */
    void
    load_from( const QStringList& file_names );

    /**
     * Load multiple files
     */
    void
    load_multiple_files( const std::vector<std::string>& file_names );

    /**
     * Load from directory.
     */
    void
    load_from_directory( const std::string& directory_name );

    /**
     * Load from file.
     */
    void
    load_from_file( const std::string& file_name );

    /**
     * Reset all controls once a file has been loaded.
    */
    void
    file_loaded( );


    void
    set_current_frame( size_t new_frame_idx );

    void
    set_current_tier( size_t new_tier_idx );

    void
    disable_graph_level();

    void
    disable_frame_selector();

    void
    clear_metrics();

    void
    update_metrics();

    void
    update_graph_tier_selector( );

    void
    init_include_checkbox( );

    void
    update_include_checkbox( );

    void
    update_view_layers();

    void
    disable_buttons();

    void
    enable_buttons( );

    void
    disable_display_checkboxes();

    void
    enable_display_checkboxes();

    void
    disable_singularities_checkbox();

    void
    enable_singularities_checkbox();

    void
    init_display_checkboxes();

    void
    init_UI();

    void
    update_frame_selector_range( );

    void
    update_frame_selector_value( );

    /**
     * Init the normal polydata/mapper/actor
     */
    void
    init_normals_layer( vtkSmartPointer<vtkRenderer> renderer );

    /**
     * Update the normals layer
     */
    void
    update_normals_layer( );

    /**
     * Init the main tangent layer
     */
    void
    init_cross_field_layer(vtkSmartPointer<vtkRenderer> renderer );

    /**
     * Init the main tangent vector layer
     */
    void
    init_neighbours_layer( vtkSmartPointer<vtkRenderer> renderer );

    /**
     * Init the singularities layer
     */
    void
    init_singularities_layer( vtkSmartPointer<vtkRenderer> renderer );

    /**
     * init the mesh layer
     */
    void
    init_mesh_layer( vtkSmartPointer<vtkRenderer> renderer );


    /**
     * Update the singularities layer
     */
    void
    update_singularities_layer();

    void
    maybe_update_singularities( );

    /**
     * Utility to initialise any layer
     */
    void
    init_layer( vtkSmartPointer<vtkRenderer> renderer, vtkSmartPointer<vtkPolyData>& poly_data, vtkSmartPointer<vtkActor>& actor );

    /**
     * Update the main tangent layer
     */
    void
    update_cross_field_layer( );

    /**
     * Update the secondary tangents layer
     */
    void
    update_neighbours_layer( );

    /**
     * Update the mesh layer
     */
    void
    update_mesh_layer( );

    void
    update_frame_range( );

    /**
     * Reconstruct all polydata from the field
     */
    void
    update_poly_data( );

    vtkSmartPointer<vtkRenderer>
    set_up_renderer();

    void
    update_inspector();

    void
    update_view();

    void
    compute_scale();

    const std::pair<std::vector<Eigen::Vector3f>, std::vector<std::vector<std::size_t>>>
    get_mesh_for_frame(std::size_t frame_id) const;

    // Optimiser
    animesh::FieldOptimiser *m_field_optimiser;

    // Poly data
    vtkSmartPointer<vtkPolyData> m_polydata_cross_field;
    vtkSmartPointer<vtkPolyData> m_polydata_normals;
    vtkSmartPointer<vtkPolyData> m_polydata_neighbours;
    vtkSmartPointer<vtkPolyData> m_polydata_singularities;
    vtkSmartPointer<vtkPolyData> m_polydata_mesh;

    // Actors
    vtkSmartPointer<vtkActor> m_cross_field_actor;
    vtkSmartPointer<vtkActor> m_normals_actor;
    vtkSmartPointer<vtkActor> m_neighbours_actor;
    vtkSmartPointer<vtkActor> m_singularities_actor;
    vtkSmartPointer<vtkActor> m_mesh_actor;

    // Number of tiers
    size_t m_num_tiers;

    // Currently displayed level of the field graph hierarchy
    size_t m_current_tier;

    // NUmber of frames
    size_t m_num_frames;

    // Currently displayed frame
    size_t m_current_frame;

    // Length to use for tangents
    float tan_scale_factor = 1.0f;

    // Singularity counts
    size_t m_num_red_singularities;
    size_t m_num_blue_singularities;
    size_t m_num_green_singularities;
    std::vector<std::tuple<Eigen::Vector3f, Eigen::Vector3f, int>> m_singularities;

    // Mesh data
    std::vector<std::pair<std::vector<Eigen::Vector3f>, std::vector<std::vector<std::size_t>>>> m_meshes;

    // Things to draw
    bool m_draw_cross_field;
    bool m_highlight_main_tangent;
    bool m_draw_normals;
    bool m_draw_neighbours;
    bool m_draw_singularities;
    bool m_draw_mesh;

private slots:
    void on_action_open_triggered();
    void on_action_exit_triggered();
    void on_btnSmoothCompletely_clicked();
    void on_btnSmoothOnce_clicked();
    void on_btnRandomise_clicked();
    void on_cbMainTangent_stateChanged(int arg1);
    void on_cbSecondaryTangents_stateChanged(int arg1);
    void on_cbNormals_stateChanged(int arg1);
    void on_cbNeighbours_stateChanged(int arg1);
    void on_cb_include_frame_stateChanged(int arg1);
    void on_hs_frame_selector_valueChanged(int value);
    void on_sbGraphLevel_valueChanged(int new_graph_level);
    void on_cbSingularities_stateChanged(int arg1);
    void on_cb_mesh_stateChanged(int arg1);
};

#endif // ANIMESHMAINWINDOW_H
