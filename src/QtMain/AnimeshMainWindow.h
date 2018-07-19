#ifndef ANIMESHMAINWINDOW_H
#define ANIMESHMAINWINDOW_H

#include <QMainWindow>
#include <Field/Field.h>


#include "vtkObject.h"
#include "vtkPolyData.h"
#include "vtkRenderer.h"
#include "vtkSmartPointer.h"


namespace animesh {

namespace Ui {
class AnimeshMainWindow;
}



class AnimeshMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit AnimeshMainWindow(QWidget *parent = nullptr);
    ~AnimeshMainWindow();

private:
    Ui::AnimeshMainWindow *ui;
    void loadFile( QString fileName );
    void setCurrentFile(const QString &fileName);

    // VTK Handling
	void update_poly_data( );
	vtkSmartPointer<vtkRenderer> set_up_renderer( );

    void set_field( Field * field );
    void update_inspector();
    void disable_inspector();
	void update_view();
    void view_changed();


	// Field declaration
	Field *    m_field;

	// Poly data
	vtkSmartPointer<vtkPolyData> m_polydata;

    // Currently displayed level of the field graph hierarchy
    int         m_current_tier;


private slots:
    void on_action_open_triggered();
    void on_action_poly_triggered();
    void on_sbGraphLevel_valueChanged(int new_graph_level);
    void on_btnSmoothCompletely_clicked();
    void on_btnSmoothOnce_clicked();
};
}
#endif // ANIMESHMAINWINDOW_H
