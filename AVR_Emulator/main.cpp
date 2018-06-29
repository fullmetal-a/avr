#include "mainwindow.h"
#include <QApplication>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    //Dfault Qt init.
    QApplication a(argc, argv);
    QStringList args = a.arguments();

    //Default values of host, port, maximum position and chance to lie
    QHostAddress host = QHostAddress::Any;
    int port = 28338;
    int chanceToLie = 10;
    int maxPos = 15000;

    QString item;   //For iterated strings of arguments

    //Means that argument of next iteration will be host or port value
    bool nextIsHost = false, nextIsPort = false, nextIsMaxPos = false, nextIsChanceToLie = false;
    for (int i = 0; i < args.size(); i++)    //Arguments iteration
    {
        item = args.at(i);  //Get argument value

        if (item == "-host")   //If argument is -host
        {
            nextIsHost = true;  //Than next argument will be host value
            continue;
        }

        if (item == "-port")   //If argument is -port
        {
            nextIsPort = true;  //Than next argument will be port value
            continue;
        }

        if (item == "-ctl")   //If argument is -ctl
        {
            nextIsChanceToLie= true;  //Than next argument will be chance to lie value
            continue;
        }

        if (item == "-maxpos")   //If argument is -port
        {
            nextIsMaxPos = true;  //Than next argument will be maximum position value
            continue;
        }

        if (nextIsHost)
        {
            host = QHostAddress(item);    //Saving custom host value
            nextIsHost = false;
        }

        if (nextIsPort)
        {
            port = item.toInt();    //Saving custom port value
            nextIsPort = false;
        }

        if (nextIsChanceToLie)
        {
            chanceToLie = item.toInt();    //Saving custom port value
            if (chanceToLie < 0 || chanceToLie > 100)
            {
                QMessageBox::critical(0,"Init Error","Incorrect chance to lie has been passed. Correct value must be between 0 and 100.");
                return 0;   //Close application, incorrect chance to lie
            }
            nextIsChanceToLie = false;
        }

        if (nextIsMaxPos)
        {
            maxPos = item.toInt();    //Saving custom port value
            if (maxPos < 1)
            {
                QMessageBox::critical(0,"Init Error","Incorrect maximum position has been passed. This value cannot be lower than 1.");
                return 0;   //Close application, incorrect maximum position
            }

            nextIsMaxPos = false;
        }
    }

    MainWindow w(host, port, chanceToLie, maxPos);   //Passing all initial data to MainWindow ctor
    w.show();
    return a.exec();
}
