#include <QApplication>
#include "AnimeshMainWindow.h"

int main(int argc, char *argv[]) {

    QApplication animesh(argc, argv);
    QCoreApplication::setAttribute(Qt::AA_DontUseNativeMenuBar);
    
    animesh::AnimeshMainWindow	mainWin;
    mainWin.show();

    return animesh.exec();
}