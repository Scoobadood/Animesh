#ifndef ANIMESHMAINWINDOW_H
#define ANIMESHMAINWINDOW_H

#include <QMainWindow>
#include <Field/Field.h>
#include <Field/FieldOptimiser.h>


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

    void load_model_from_file(QString fileName);
    /**
     * Load a new file, setup all the stuff
     */
    void load_new_frame(QString file_name);

    void setCurrentFile(const QString &fileName);

    void disable_frame_counter( );

    void update_frame_counter( );

        // VTK Handling
    void update_poly_data();

    vtkSmartPointer<vtkRenderer> set_up_renderer();

    void set_field(animesh::Field *field);

    void update_inspector();

    void disable_inspector();

    void update_view();

    void view_changed();

    void compute_scale();


    // Field declaration
    animesh::Field *m_field;

    // Optimiser
    animesh::FieldOptimiser *m_field_optimiser;

    // Poly data
    vtkSmartPointer<vtkPolyData> m_polydata;

    // Currently displayed level of the field graph hierarchy
    int m_current_tier;

    // Currently displayed frame
    int m_current_frame;

    // Length to use for tangents
    float tan_scale_factor = 1.0f;

private slots:

    void on_action_open_triggered();

    void on_action_poly_triggered();

    void on_action_plane_triggered();

    void on_action_add_frame_triggered();

    void on_sbGraphLevel_valueChanged(int new_graph_level);

    void on_sbFrameNumber_valueChanged(int new_frame_number);

    void on_btnSmoothCompletely_clicked();

    void on_btnSmoothOnce_clicked();
};

#endif // ANIMESHMAINWINDOW_H
