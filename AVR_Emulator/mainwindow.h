#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include "avrsystem.h"
#include "avrserver.h"

namespace Ui 
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    Ui::MainWindow *ui;         //UI interface
    QThread backgroundThread;   //Separate thread for AVR system
    AVR::AVRSystem* avr;        //AVR main interface
    AVR::Server* server;        //AVR Server entity

public:
    explicit MainWindow(QWidget *parent = 0);
    MainWindow(const QHostAddress& host, int iPort, int chanceToLie, int maxPos, QWidget *parent = 0);
    ~MainWindow();

private slots:
    void OnUpdateAVRDisplay(int pos);   //Triggers when AVR system asks to update UI position
    void ChangeConnectionLabelToValue(bool IsConnected);    //Changes UI label text of connection state
    
};

#endif // MAINWINDOW_H
