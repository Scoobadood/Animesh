#ifndef ANIMESHMAINWINDOW_H
#define ANIMESHMAINWINDOW_H

#include <QMainWindow>
#include <Field/Field.h>

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

	// Field declaration
	Field * m_field;


private slots:
    void on_actionOpen_triggered();

};

#endif // ANIMESHMAINWINDOW_H
