#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "client.h"

namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_MoveToPos_clicked();
    void on_MoveToZero_clicked();
    void on_AskPosition_clicked();
    void on_actionConnect_triggered();
    void on_actionDisconnect_triggered();
    void on_actionClear_triggered();
    void on_actionSave_to_file_triggered();
    void on_actionAbout_triggered();
    void on_actionAbout_Qt_triggered();

    void OnSetAVRControlsEnabled(bool isEnabled);   //Enables AVR controls on main form when client signals

signals:
    void SendData(AVR::MessageType msg, int steps = 0); //Signals client to send data

private:
    Ui::MainWindow *ui;
    AVR::Client* client;    //Pointer to client entity
};

#endif // MAINWINDOW_H
