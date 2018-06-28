#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    //Nothing to comment, just default Qt init code.
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
