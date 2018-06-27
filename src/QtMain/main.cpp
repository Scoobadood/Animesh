#include <QApplication>
#include "AnimeshMainWindow.h"

int main(int argc, char *argv[]) {

    QApplication animesh(argc, argv);
    
    AnimeshMainWindow	mainWin;
    mainWin.show();

    return animesh.exec();
}