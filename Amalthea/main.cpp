#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    if(w.no){
        qApp->exit(-1);
        return -1;
    }
    w.show();

    return a.exec();
}
