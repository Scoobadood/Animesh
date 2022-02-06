/********************************************************************************
** Form generated from reading UI file 'field_visualiser_window.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FIELD_VISUALISER_WINDOW_H
#define UI_FIELD_VISUALISER_WINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <quad_gl_widget.h>
#include "posy_gl_widget.h"
#include "rosy_gl_widget.h"

QT_BEGIN_NAMESPACE

class Ui_field_visualiser_window
{
public:
    QAction *actionOpen;
    QWidget *centralwidget;
    QHBoxLayout *horizontalLayout;
    QFrame *controlPanel;
    QVBoxLayout *verticalLayout_2;
    QGroupBox *gbFrameSelector;
    QHBoxLayout *horizontalLayout_2;
    QSlider *frameSelector;
    QLabel *lblFrame;
    QGroupBox *gbRoSy;
    QVBoxLayout *verticalLayout_4;
    QCheckBox *cbNormals;
    QCheckBox *cbMainTangent;
    QCheckBox *cbOtherTangents;
    QCheckBox *cbErrorColours;
    QCheckBox *cbShowSplats;
    QCheckBox *cbShowPath;
    QSlider *slNormalLength;
    QGroupBox *gbPosy;
    QVBoxLayout *verticalLayout;
    QFrame *gbSplatSizeSelector;
    QHBoxLayout *horizontalLayout_3;
    QSlider *splatSizeSelector;
    QLabel *lblSplatSize;
    QCheckBox *cbShowTriangleFans;
    QCheckBox *cbShowQuads;
    QCheckBox *cbShowTextures;
    QGroupBox *gbQuads;
    QVBoxLayout *verticalLayout_5;
    QCheckBox *cbRed;
    QCheckBox *cbBlue;
    QCheckBox *cbAffinities;
    QGroupBox *gbEdge;
    QFormLayout *formLayout;
    QLabel *lblId;
    QLabel *lblTan;
    QLabel *lblTij;
    QLabel *lblTji;
    QLabel *lblSurfelId;
    QLabel *lblSurfelTanX;
    QLabel *lblSurfelTanZ;
    QLabel *lblKij;
    QLabel *lblSurfelCorrection;
    QLabel *lblKji;
    QLabel *lblSurfelSmoothness;
    QLabel *lblSurfelTanY;
    QPushButton *btnDecimate;
    QTabWidget *viewPanel;
    QWidget *rosyTab;
    QVBoxLayout *verticalLayout_3;
    rosy_gl_widget *rosyGLWidget;
    QWidget *posyTab;
    QVBoxLayout *verticalLayout_6;
    posy_gl_widget *posyGLWidget;
    QWidget *quadTab;
    QVBoxLayout *verticalLayout_7;
    quad_gl_widget *quadGLWidget;
    QMenuBar *menuBar;
    QMenu *menuFile;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *field_visualiser_window)
    {
        if (field_visualiser_window->objectName().isEmpty())
            field_visualiser_window->setObjectName(QString::fromUtf8("field_visualiser_window"));
        field_visualiser_window->resize(909, 803);
        QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        sizePolicy.setHorizontalStretch(1);
        sizePolicy.setVerticalStretch(1);
        sizePolicy.setHeightForWidth(field_visualiser_window->sizePolicy().hasHeightForWidth());
        field_visualiser_window->setSizePolicy(sizePolicy);
        actionOpen = new QAction(field_visualiser_window);
        actionOpen->setObjectName(QString::fromUtf8("actionOpen"));
        centralwidget = new QWidget(field_visualiser_window);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        sizePolicy.setHeightForWidth(centralwidget->sizePolicy().hasHeightForWidth());
        centralwidget->setSizePolicy(sizePolicy);
        centralwidget->setMaximumSize(QSize(16777215, 16777215));
        horizontalLayout = new QHBoxLayout(centralwidget);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        controlPanel = new QFrame(centralwidget);
        controlPanel->setObjectName(QString::fromUtf8("controlPanel"));
        QSizePolicy sizePolicy1(QSizePolicy::Minimum, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(20);
        sizePolicy1.setVerticalStretch(100);
        sizePolicy1.setHeightForWidth(controlPanel->sizePolicy().hasHeightForWidth());
        controlPanel->setSizePolicy(sizePolicy1);
        controlPanel->setMinimumSize(QSize(10, 0));
        controlPanel->setFrameShape(QFrame::StyledPanel);
        controlPanel->setFrameShadow(QFrame::Raised);
        verticalLayout_2 = new QVBoxLayout(controlPanel);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        gbFrameSelector = new QGroupBox(controlPanel);
        gbFrameSelector->setObjectName(QString::fromUtf8("gbFrameSelector"));
        QSizePolicy sizePolicy2(QSizePolicy::Maximum, QSizePolicy::Maximum);
        sizePolicy2.setHorizontalStretch(1);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(gbFrameSelector->sizePolicy().hasHeightForWidth());
        gbFrameSelector->setSizePolicy(sizePolicy2);
        horizontalLayout_2 = new QHBoxLayout(gbFrameSelector);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        frameSelector = new QSlider(gbFrameSelector);
        frameSelector->setObjectName(QString::fromUtf8("frameSelector"));
        QSizePolicy sizePolicy3(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(frameSelector->sizePolicy().hasHeightForWidth());
        frameSelector->setSizePolicy(sizePolicy3);
        frameSelector->setMinimum(0);
        frameSelector->setMaximum(0);
        frameSelector->setOrientation(Qt::Horizontal);

        horizontalLayout_2->addWidget(frameSelector);

        lblFrame = new QLabel(gbFrameSelector);
        lblFrame->setObjectName(QString::fromUtf8("lblFrame"));
        QSizePolicy sizePolicy4(QSizePolicy::Minimum, QSizePolicy::Preferred);
        sizePolicy4.setHorizontalStretch(0);
        sizePolicy4.setVerticalStretch(0);
        sizePolicy4.setHeightForWidth(lblFrame->sizePolicy().hasHeightForWidth());
        lblFrame->setSizePolicy(sizePolicy4);
        lblFrame->setFrameShape(QFrame::Panel);
        lblFrame->setAlignment(Qt::AlignCenter);
        lblFrame->setIndent(-1);

        horizontalLayout_2->addWidget(lblFrame);


        verticalLayout_2->addWidget(gbFrameSelector);

        gbRoSy = new QGroupBox(controlPanel);
        gbRoSy->setObjectName(QString::fromUtf8("gbRoSy"));
        QSizePolicy sizePolicy5(QSizePolicy::Preferred, QSizePolicy::Maximum);
        sizePolicy5.setHorizontalStretch(1);
        sizePolicy5.setVerticalStretch(0);
        sizePolicy5.setHeightForWidth(gbRoSy->sizePolicy().hasHeightForWidth());
        gbRoSy->setSizePolicy(sizePolicy5);
        verticalLayout_4 = new QVBoxLayout(gbRoSy);
        verticalLayout_4->setObjectName(QString::fromUtf8("verticalLayout_4"));
        cbNormals = new QCheckBox(gbRoSy);
        cbNormals->setObjectName(QString::fromUtf8("cbNormals"));
        cbNormals->setChecked(true);

        verticalLayout_4->addWidget(cbNormals);

        cbMainTangent = new QCheckBox(gbRoSy);
        cbMainTangent->setObjectName(QString::fromUtf8("cbMainTangent"));
        cbMainTangent->setChecked(true);

        verticalLayout_4->addWidget(cbMainTangent);

        cbOtherTangents = new QCheckBox(gbRoSy);
        cbOtherTangents->setObjectName(QString::fromUtf8("cbOtherTangents"));
        cbOtherTangents->setChecked(true);

        verticalLayout_4->addWidget(cbOtherTangents);

        cbErrorColours = new QCheckBox(gbRoSy);
        cbErrorColours->setObjectName(QString::fromUtf8("cbErrorColours"));
        cbErrorColours->setChecked(true);

        verticalLayout_4->addWidget(cbErrorColours);

        cbShowSplats = new QCheckBox(gbRoSy);
        cbShowSplats->setObjectName(QString::fromUtf8("cbShowSplats"));
        cbShowSplats->setChecked(true);

        verticalLayout_4->addWidget(cbShowSplats);

        cbShowPath = new QCheckBox(gbRoSy);
        cbShowPath->setObjectName(QString::fromUtf8("cbShowPath"));
        cbShowPath->setChecked(true);

        verticalLayout_4->addWidget(cbShowPath);

        slNormalLength = new QSlider(gbRoSy);
        slNormalLength->setObjectName(QString::fromUtf8("slNormalLength"));
        slNormalLength->setMinimum(1);
        slNormalLength->setMaximum(10);
        slNormalLength->setPageStep(1);
        slNormalLength->setOrientation(Qt::Horizontal);

        verticalLayout_4->addWidget(slNormalLength);


        verticalLayout_2->addWidget(gbRoSy);

        gbPosy = new QGroupBox(controlPanel);
        gbPosy->setObjectName(QString::fromUtf8("gbPosy"));
        sizePolicy5.setHeightForWidth(gbPosy->sizePolicy().hasHeightForWidth());
        gbPosy->setSizePolicy(sizePolicy5);
        verticalLayout = new QVBoxLayout(gbPosy);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        gbSplatSizeSelector = new QFrame(gbPosy);
        gbSplatSizeSelector->setObjectName(QString::fromUtf8("gbSplatSizeSelector"));
        QSizePolicy sizePolicy6(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy6.setHorizontalStretch(100);
        sizePolicy6.setVerticalStretch(0);
        sizePolicy6.setHeightForWidth(gbSplatSizeSelector->sizePolicy().hasHeightForWidth());
        gbSplatSizeSelector->setSizePolicy(sizePolicy6);
        horizontalLayout_3 = new QHBoxLayout(gbSplatSizeSelector);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        horizontalLayout_3->setContentsMargins(1, 1, 1, 1);
        splatSizeSelector = new QSlider(gbSplatSizeSelector);
        splatSizeSelector->setObjectName(QString::fromUtf8("splatSizeSelector"));
        QSizePolicy sizePolicy7(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy7.setHorizontalStretch(1);
        sizePolicy7.setVerticalStretch(0);
        sizePolicy7.setHeightForWidth(splatSizeSelector->sizePolicy().hasHeightForWidth());
        splatSizeSelector->setSizePolicy(sizePolicy7);
        splatSizeSelector->setMinimum(0);
        splatSizeSelector->setMaximum(10);
        splatSizeSelector->setSliderPosition(5);
        splatSizeSelector->setOrientation(Qt::Horizontal);
        splatSizeSelector->setTickPosition(QSlider::TicksBelow);
        splatSizeSelector->setTickInterval(1);

        horizontalLayout_3->addWidget(splatSizeSelector, 0, Qt::AlignLeft);

        lblSplatSize = new QLabel(gbSplatSizeSelector);
        lblSplatSize->setObjectName(QString::fromUtf8("lblSplatSize"));
        sizePolicy3.setHeightForWidth(lblSplatSize->sizePolicy().hasHeightForWidth());
        lblSplatSize->setSizePolicy(sizePolicy3);
        lblSplatSize->setFrameShape(QFrame::Panel);

        horizontalLayout_3->addWidget(lblSplatSize, 0, Qt::AlignRight);

        horizontalLayout_3->setStretch(0, 1);
        horizontalLayout_3->setStretch(1, 1);

        verticalLayout->addWidget(gbSplatSizeSelector);

        cbShowTriangleFans = new QCheckBox(gbPosy);
        cbShowTriangleFans->setObjectName(QString::fromUtf8("cbShowTriangleFans"));
        QSizePolicy sizePolicy8(QSizePolicy::Minimum, QSizePolicy::Preferred);
        sizePolicy8.setHorizontalStretch(1);
        sizePolicy8.setVerticalStretch(1);
        sizePolicy8.setHeightForWidth(cbShowTriangleFans->sizePolicy().hasHeightForWidth());
        cbShowTriangleFans->setSizePolicy(sizePolicy8);

        verticalLayout->addWidget(cbShowTriangleFans);

        cbShowQuads = new QCheckBox(gbPosy);
        cbShowQuads->setObjectName(QString::fromUtf8("cbShowQuads"));
        sizePolicy8.setHeightForWidth(cbShowQuads->sizePolicy().hasHeightForWidth());
        cbShowQuads->setSizePolicy(sizePolicy8);
        cbShowQuads->setChecked(true);

        verticalLayout->addWidget(cbShowQuads);

        cbShowTextures = new QCheckBox(gbPosy);
        cbShowTextures->setObjectName(QString::fromUtf8("cbShowTextures"));
        sizePolicy8.setHeightForWidth(cbShowTextures->sizePolicy().hasHeightForWidth());
        cbShowTextures->setSizePolicy(sizePolicy8);

        verticalLayout->addWidget(cbShowTextures);


        verticalLayout_2->addWidget(gbPosy);

        gbQuads = new QGroupBox(controlPanel);
        gbQuads->setObjectName(QString::fromUtf8("gbQuads"));
        sizePolicy5.setHeightForWidth(gbQuads->sizePolicy().hasHeightForWidth());
        gbQuads->setSizePolicy(sizePolicy5);
        verticalLayout_5 = new QVBoxLayout(gbQuads);
        verticalLayout_5->setObjectName(QString::fromUtf8("verticalLayout_5"));
        cbRed = new QCheckBox(gbQuads);
        cbRed->setObjectName(QString::fromUtf8("cbRed"));
        cbRed->setChecked(true);

        verticalLayout_5->addWidget(cbRed);

        cbBlue = new QCheckBox(gbQuads);
        cbBlue->setObjectName(QString::fromUtf8("cbBlue"));
        cbBlue->setChecked(true);

        verticalLayout_5->addWidget(cbBlue);

        cbAffinities = new QCheckBox(gbQuads);
        cbAffinities->setObjectName(QString::fromUtf8("cbAffinities"));

        verticalLayout_5->addWidget(cbAffinities);


        verticalLayout_2->addWidget(gbQuads);

        gbEdge = new QGroupBox(controlPanel);
        gbEdge->setObjectName(QString::fromUtf8("gbEdge"));
        formLayout = new QFormLayout(gbEdge);
        formLayout->setObjectName(QString::fromUtf8("formLayout"));
        formLayout->setContentsMargins(6, 6, 6, 6);
        lblId = new QLabel(gbEdge);
        lblId->setObjectName(QString::fromUtf8("lblId"));
        lblId->setScaledContents(false);

        formLayout->setWidget(0, QFormLayout::LabelRole, lblId);

        lblTan = new QLabel(gbEdge);
        lblTan->setObjectName(QString::fromUtf8("lblTan"));
        lblTan->setScaledContents(false);

        formLayout->setWidget(1, QFormLayout::LabelRole, lblTan);

        lblTij = new QLabel(gbEdge);
        lblTij->setObjectName(QString::fromUtf8("lblTij"));
        lblTij->setScaledContents(false);

        formLayout->setWidget(2, QFormLayout::LabelRole, lblTij);

        lblTji = new QLabel(gbEdge);
        lblTji->setObjectName(QString::fromUtf8("lblTji"));
        lblTji->setScaledContents(false);

        formLayout->setWidget(3, QFormLayout::LabelRole, lblTji);

        lblSurfelId = new QLabel(gbEdge);
        lblSurfelId->setObjectName(QString::fromUtf8("lblSurfelId"));
        lblSurfelId->setScaledContents(false);

        formLayout->setWidget(0, QFormLayout::FieldRole, lblSurfelId);

        lblSurfelTanX = new QLabel(gbEdge);
        lblSurfelTanX->setObjectName(QString::fromUtf8("lblSurfelTanX"));
        lblSurfelTanX->setScaledContents(false);

        formLayout->setWidget(1, QFormLayout::FieldRole, lblSurfelTanX);

        lblSurfelTanZ = new QLabel(gbEdge);
        lblSurfelTanZ->setObjectName(QString::fromUtf8("lblSurfelTanZ"));
        lblSurfelTanZ->setScaledContents(false);

        formLayout->setWidget(3, QFormLayout::FieldRole, lblSurfelTanZ);

        lblKij = new QLabel(gbEdge);
        lblKij->setObjectName(QString::fromUtf8("lblKij"));
        lblKij->setScaledContents(false);

        formLayout->setWidget(4, QFormLayout::LabelRole, lblKij);

        lblSurfelCorrection = new QLabel(gbEdge);
        lblSurfelCorrection->setObjectName(QString::fromUtf8("lblSurfelCorrection"));
        lblSurfelCorrection->setScaledContents(false);

        formLayout->setWidget(4, QFormLayout::FieldRole, lblSurfelCorrection);

        lblKji = new QLabel(gbEdge);
        lblKji->setObjectName(QString::fromUtf8("lblKji"));
        lblKji->setScaledContents(false);

        formLayout->setWidget(5, QFormLayout::LabelRole, lblKji);

        lblSurfelSmoothness = new QLabel(gbEdge);
        lblSurfelSmoothness->setObjectName(QString::fromUtf8("lblSurfelSmoothness"));
        lblSurfelSmoothness->setScaledContents(false);

        formLayout->setWidget(5, QFormLayout::FieldRole, lblSurfelSmoothness);

        lblSurfelTanY = new QLabel(gbEdge);
        lblSurfelTanY->setObjectName(QString::fromUtf8("lblSurfelTanY"));
        lblSurfelTanY->setScaledContents(false);

        formLayout->setWidget(2, QFormLayout::FieldRole, lblSurfelTanY);


        verticalLayout_2->addWidget(gbEdge);

        btnDecimate = new QPushButton(controlPanel);
        btnDecimate->setObjectName(QString::fromUtf8("btnDecimate"));

        verticalLayout_2->addWidget(btnDecimate);


        horizontalLayout->addWidget(controlPanel);

        viewPanel = new QTabWidget(centralwidget);
        viewPanel->setObjectName(QString::fromUtf8("viewPanel"));
        QSizePolicy sizePolicy9(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy9.setHorizontalStretch(80);
        sizePolicy9.setVerticalStretch(100);
        sizePolicy9.setHeightForWidth(viewPanel->sizePolicy().hasHeightForWidth());
        viewPanel->setSizePolicy(sizePolicy9);
        viewPanel->setMinimumSize(QSize(0, 0));
        rosyTab = new QWidget();
        rosyTab->setObjectName(QString::fromUtf8("rosyTab"));
        verticalLayout_3 = new QVBoxLayout(rosyTab);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        rosyGLWidget = new rosy_gl_widget(rosyTab);
        rosyGLWidget->setObjectName(QString::fromUtf8("rosyGLWidget"));
        QSizePolicy sizePolicy10(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy10.setHorizontalStretch(100);
        sizePolicy10.setVerticalStretch(50);
        sizePolicy10.setHeightForWidth(rosyGLWidget->sizePolicy().hasHeightForWidth());
        rosyGLWidget->setSizePolicy(sizePolicy10);
        rosyGLWidget->setMinimumSize(QSize(20, 0));

        verticalLayout_3->addWidget(rosyGLWidget);

        viewPanel->addTab(rosyTab, QString());
        posyTab = new QWidget();
        posyTab->setObjectName(QString::fromUtf8("posyTab"));
        verticalLayout_6 = new QVBoxLayout(posyTab);
        verticalLayout_6->setObjectName(QString::fromUtf8("verticalLayout_6"));
        posyGLWidget = new posy_gl_widget(posyTab);
        posyGLWidget->setObjectName(QString::fromUtf8("posyGLWidget"));
        sizePolicy10.setHeightForWidth(posyGLWidget->sizePolicy().hasHeightForWidth());
        posyGLWidget->setSizePolicy(sizePolicy10);
        posyGLWidget->setMinimumSize(QSize(0, 10));
        posyGLWidget->setMaximumSize(QSize(16777215, 16777215));

        verticalLayout_6->addWidget(posyGLWidget);

        viewPanel->addTab(posyTab, QString());
        quadTab = new QWidget();
        quadTab->setObjectName(QString::fromUtf8("quadTab"));
        verticalLayout_7 = new QVBoxLayout(quadTab);
        verticalLayout_7->setObjectName(QString::fromUtf8("verticalLayout_7"));
        quadGLWidget = new quad_gl_widget(quadTab);
        quadGLWidget->setObjectName(QString::fromUtf8("quadGLWidget"));
        sizePolicy10.setHeightForWidth(quadGLWidget->sizePolicy().hasHeightForWidth());
        quadGLWidget->setSizePolicy(sizePolicy10);

        verticalLayout_7->addWidget(quadGLWidget);

        viewPanel->addTab(quadTab, QString());

        horizontalLayout->addWidget(viewPanel);

        field_visualiser_window->setCentralWidget(centralwidget);
        menuBar = new QMenuBar(field_visualiser_window);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 909, 22));
        menuFile = new QMenu(menuBar);
        menuFile->setObjectName(QString::fromUtf8("menuFile"));
        field_visualiser_window->setMenuBar(menuBar);
        statusbar = new QStatusBar(field_visualiser_window);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        field_visualiser_window->setStatusBar(statusbar);

        menuBar->addAction(menuFile->menuAction());
        menuFile->addAction(actionOpen);

        retranslateUi(field_visualiser_window);

        viewPanel->setCurrentIndex(2);


        QMetaObject::connectSlotsByName(field_visualiser_window);
    } // setupUi

    void retranslateUi(QMainWindow *field_visualiser_window)
    {
        field_visualiser_window->setWindowTitle(QCoreApplication::translate("field_visualiser_window", "posy_visualiser_window", nullptr));
        actionOpen->setText(QCoreApplication::translate("field_visualiser_window", "Open", nullptr));
        gbFrameSelector->setTitle(QCoreApplication::translate("field_visualiser_window", "Frames", nullptr));
        lblFrame->setText(QCoreApplication::translate("field_visualiser_window", "01", nullptr));
        gbRoSy->setTitle(QCoreApplication::translate("field_visualiser_window", "RoSy", nullptr));
        cbNormals->setText(QCoreApplication::translate("field_visualiser_window", "Normals", nullptr));
        cbMainTangent->setText(QCoreApplication::translate("field_visualiser_window", "Tangents", nullptr));
        cbOtherTangents->setText(QCoreApplication::translate("field_visualiser_window", "Other Tangents", nullptr));
        cbErrorColours->setText(QCoreApplication::translate("field_visualiser_window", "Error Colours", nullptr));
        cbShowSplats->setText(QCoreApplication::translate("field_visualiser_window", "Show Splats", nullptr));
        cbShowPath->setText(QCoreApplication::translate("field_visualiser_window", "Show Path", nullptr));
        gbPosy->setTitle(QCoreApplication::translate("field_visualiser_window", "PoSy", nullptr));
        lblSplatSize->setText(QCoreApplication::translate("field_visualiser_window", "01", nullptr));
        cbShowTriangleFans->setText(QCoreApplication::translate("field_visualiser_window", "Show tri-fans", nullptr));
        cbShowQuads->setText(QCoreApplication::translate("field_visualiser_window", "Show quads", nullptr));
        cbShowTextures->setText(QCoreApplication::translate("field_visualiser_window", "Show textures", nullptr));
        gbQuads->setTitle(QCoreApplication::translate("field_visualiser_window", "Quads", nullptr));
        cbRed->setText(QCoreApplication::translate("field_visualiser_window", "Red", nullptr));
        cbBlue->setText(QCoreApplication::translate("field_visualiser_window", "Blue", nullptr));
        cbAffinities->setText(QCoreApplication::translate("field_visualiser_window", "Affinities", nullptr));
        gbEdge->setTitle(QCoreApplication::translate("field_visualiser_window", "Selected", nullptr));
        lblId->setText(QCoreApplication::translate("field_visualiser_window", "id", nullptr));
        lblTan->setText(QCoreApplication::translate("field_visualiser_window", "tan", nullptr));
        lblTij->setText(QString());
        lblTji->setText(QString());
        lblSurfelId->setText(QString());
        lblSurfelTanX->setText(QString());
        lblSurfelTanZ->setText(QString());
        lblKij->setText(QCoreApplication::translate("field_visualiser_window", "Corr.", nullptr));
        lblSurfelCorrection->setText(QString());
        lblKji->setText(QCoreApplication::translate("field_visualiser_window", "Smth.", nullptr));
        lblSurfelSmoothness->setText(QString());
        lblSurfelTanY->setText(QString());
        btnDecimate->setText(QCoreApplication::translate("field_visualiser_window", "Decimate", nullptr));
        viewPanel->setTabText(viewPanel->indexOf(rosyTab), QCoreApplication::translate("field_visualiser_window", "RoSy", nullptr));
        viewPanel->setTabText(viewPanel->indexOf(posyTab), QCoreApplication::translate("field_visualiser_window", "PoSy", nullptr));
        viewPanel->setTabText(viewPanel->indexOf(quadTab), QCoreApplication::translate("field_visualiser_window", "Quads", nullptr));
        menuFile->setTitle(QCoreApplication::translate("field_visualiser_window", "File", nullptr));
    } // retranslateUi

};

namespace Ui {
    class field_visualiser_window: public Ui_field_visualiser_window {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FIELD_VISUALISER_WINDOW_H
