#include <QApplication>
#include "AnimeshMainWindow.h"

int main(int argc, char *argv[]) {

	srandom( 1234L);
    QApplication animesh(argc, argv);
    QCoreApplication::setAttribute(Qt::AA_DontUseNativeMenuBar);
    
    AnimeshMainWindow	mainWin;
    mainWin.show();

    return animesh.exec();
}