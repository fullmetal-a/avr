#include "avrsystem.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>

MainWindow::MainWindow(const QHostAddress& host, int iPort, int chanceToLie, int maxPos, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this); //Init Qt UI
    //Registering our types for Qt signals
    qRegisterMetaType<AVR::Message>("AVR::Message");
    qRegisterMetaType<AVR::Message::Type>("Message::Type");
    //Creating AVR System unit
    avr = new AVR::AVRSystem(chanceToLie, maxPos);  //Passing chance to lie and maximum position values
    try
    {
        server = new AVR::Server(host, iPort); //Trying to create and host AVR server entity
    }
    catch(AVR::Server::Exeption svExeption) //If server init failed
    {
        if(svExeption == AVR::Server::Exeption::ListenFailed)
        {
            this->close();  //Exiting from applicationg.
            exit(0);
        }
    }
    QString sHost, sPort;   //String variables for host and port
    sHost = host.toString();
    sPort.sprintf("%i", iPort);
    if(sHost == "0.0.0.0")  //Set hostname to localhost if QHostName returns 0.0.0.0 (Which means "Any host").
        sHost = "localhost";
    QString hostInfo = "Host info: " + sHost + ":" + sPort;   //Creating host information sign
    ui->hostInfo->setText(hostInfo);
    avr->moveToThread(&backgroundThread);   //Moving AVR System to separate thread

    //Connecting all slots and events of AVR System and Server
    QObject::connect(&backgroundThread, &QThread::finished, avr, &QObject::deleteLater);
    QObject::connect(server, &AVR::Server::AVRMessage, avr, &AVR::AVRSystem::AVRSystem::ParseMsg, Qt::QueuedConnection);
    QObject::connect(server, &AVR::Server::ChangeConnectionLabel, this, &MainWindow::ChangeConnectionLabelToValue);
    QObject::connect(avr, &AVR::AVRSystem::WorkIsComplete, server, &AVR::Server::AVRWorkIsComplete, Qt::QueuedConnection);
    QObject::connect(avr, &AVR::AVRSystem::SendPosition, server, &AVR::Server::SendPosition, Qt::QueuedConnection);
    QObject::connect(avr, &AVR::AVRSystem::ErrorOccurred, server, &AVR::Server::OnAVRError);
    QObject::connect(avr, &AVR::AVRSystem::MessageReceived, server, &AVR::Server::OnMessageReceived);
    QObject::connect(avr, &AVR::AVRSystem::UpdateDisplay, this, &MainWindow::OnUpdateAVRDisplay, Qt::QueuedConnection);
    QObject::connect(avr, &AVR::AVRSystem::ClientInit, server, &AVR::Server::OnClientInit);
    QObject::connect(server, &AVR::Server::AskForClientInit, avr, &AVR::AVRSystem::OnClientInitRequest);

    //Launching AVR System's thread
    backgroundThread.start();
}

MainWindow::~MainWindow()
{
    //Destroying ui, AVR System and Server objects
    delete ui;
    delete avr;
    backgroundThread.quit();    //Also stopping the background thread
    backgroundThread.wait();
    delete server;
}

//Triggers when AVR System says UI to change position value on window
void MainWindow::OnUpdateAVRDisplay(int pos)
{
    ui->AVRPos->display(pos);   //Set LCD Display value to pos
}

//When someone connected to server (or disconnected)
//Servers sends signal to UI to change connection state lable sign
void MainWindow::ChangeConnectionLabelToValue(bool IsConnected)
{
    if(IsConnected)
        ui->connectionState->setText("<html><head/><body><p align=\"center\"><span style=\" font-weight:600; color:#00aa00;\">Client connected</span></p></body></html>");
    else
        ui->connectionState->setText("<html><head/><body><p align=\"center\"><span style=\" font-weight:600; color:#aa0000;\">No connection</span></p></body></html>");
}

