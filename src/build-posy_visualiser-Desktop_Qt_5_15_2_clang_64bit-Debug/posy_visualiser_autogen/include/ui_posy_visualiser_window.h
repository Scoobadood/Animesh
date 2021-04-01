/********************************************************************************
** Form generated from reading UI file 'posy_visualiser_window.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_POSY_VISUALISER_WINDOW_H
#define UI_POSY_VISUALISER_WINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QOpenGLWidget>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_posy_visualiser_window
{
public:
    QWidget *centralwidget;
    QOpenGLWidget *openGLWidget;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *posy_visualiser_window)
    {
        if (posy_visualiser_window->objectName().isEmpty())
            posy_visualiser_window->setObjectName(QString::fromUtf8("posy_visualiser_window"));
        posy_visualiser_window->resize(800, 600);
        centralwidget = new QWidget(posy_visualiser_window);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        openGLWidget = new QOpenGLWidget(centralwidget);
        openGLWidget->setObjectName(QString::fromUtf8("openGLWidget"));
        openGLWidget->setGeometry(QRect(230, 130, 300, 200));
        posy_visualiser_window->setCentralWidget(centralwidget);
        menubar = new QMenuBar(posy_visualiser_window);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 800, 22));
        posy_visualiser_window->setMenuBar(menubar);
        statusbar = new QStatusBar(posy_visualiser_window);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        posy_visualiser_window->setStatusBar(statusbar);

        retranslateUi(posy_visualiser_window);

        QMetaObject::connectSlotsByName(posy_visualiser_window);
    } // setupUi

    void retranslateUi(QMainWindow *posy_visualiser_window)
    {
        posy_visualiser_window->setWindowTitle(QCoreApplication::translate("posy_visualiser_window", "posy_visualiser_window", nullptr));
    } // retranslateUi

};

namespace Ui {
    class posy_visualiser_window: public Ui_posy_visualiser_window {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_POSY_VISUALISER_WINDOW_H
