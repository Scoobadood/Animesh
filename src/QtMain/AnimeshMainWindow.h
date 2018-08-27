#ifndef ANIMESHMAINWINDOW_H
#define ANIMESHMAINWINDOW_H

#include <QMainWindow>
#include <Field/Field.h>
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

    void
    load_model_from_file(QString fileName);

    void
    load_new_frame(QString file_name);

    void
    setCurrentFile(const QString &fileName);

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
    init_main_tangent_vector_layer(vtkSmartPointer<vtkRenderer> renderer );

    /**
     * Update the main tangent layer
     */
    void
    update_main_tangent_vector_layer( );

    /**
     * Init the main tangent vector layer
     */
    void
    init_secondary_tangent_vector_layer( vtkSmartPointer<vtkRenderer> renderer );

    /**
     * Update the secondary tangents layer
     */
    void
    update_secondary_tangent_vector_layer( );

    /**
     * Init the main tangent vector layer
     */
    void
    init_neighbours_layer( vtkSmartPointer<vtkRenderer> renderer );

    /**
     * Update the secondary tangents layer
     */
    void
    update_neighbours_layer( );

    void
    update_frame_range( );

    /**
     * Reconstruct the given polydata from the field
     */
    void
    update_poly_data( );

    vtkSmartPointer<vtkRenderer> set_up_renderer();

    void
    set_field(animesh::Field *field);

    void
    update_inspector();

    void
    update_view();

    void
    compute_scale();


    // Field declaration
    animesh::Field *m_field;

    // Optimiser
    animesh::FieldOptimiser *m_field_optimiser;

    // Poly data
    vtkSmartPointer<vtkPolyData> m_polydata_main_tangents;
    vtkSmartPointer<vtkPolyData> m_polydata_other_tangents;
    vtkSmartPointer<vtkPolyData> m_polydata_normals;
    vtkSmartPointer<vtkPolyData> m_polydata_neighbours;

    // Actors
    vtkSmartPointer<vtkActor> m_main_tangents_actor;
    vtkSmartPointer<vtkActor> m_other_tangents_actor;
    vtkSmartPointer<vtkActor> m_normals_actor;
    vtkSmartPointer<vtkActor> m_neighbours_actor;

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

    // Things to draw
    bool m_draw_main_tangent;
    bool m_draw_other_tangents;
    bool m_draw_normals;
    bool m_draw_neighbours;

private slots:
    void on_action_open_triggered();
    void on_action_add_frame_triggered();
    void on_sbGraphLevel_valueChanged(int new_graph_level);
    void on_btnSmoothCompletely_clicked();
    void on_btnSmoothOnce_clicked();
    void on_cbMainTangent_stateChanged(int arg1);
    void on_cbSecondaryTangents_stateChanged(int arg1);
    void on_cbNormals_stateChanged(int arg1);
    void on_cbNeighbours_stateChanged(int arg1);
    void on_cb_include_frame_stateChanged(int arg1);
    void on_hs_frame_selector_valueChanged(int value);
    void on_btnRandomise_clicked();
};

#endif // ANIMESHMAINWINDOW_H
