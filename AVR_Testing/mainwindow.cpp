#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QIntValidator>
#include <QFileDialog>
#include <QFile>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    //Init of AVR Client
    ui->setupUi(this);  //Init Qt UI
    ui->actionDisconnect->setEnabled(false);    //Disallow user to disconnect (We are already disconnected)
    OnSetAVRControlsEnabled(false);             //Disabling controls
    qRegisterMetaType<AVR::MessageType>("AVR::MessageType");    //Register message enum type
    ui->inputSteps->setValidator(new QIntValidator(-100000, 100000, this)); //Set bounds for step input edit
    client = new AVR::Client(0);    //Creating client entity

    //Connecting our signals and slots
    QObject::connect(this, &MainWindow::SendData, client, &AVR::Client::slotSendToServer);
    QObject::connect(client, &AVR::Client::SetAVRControlsEnabled, this, &MainWindow::OnSetAVRControlsEnabled);
    QObject::connect(client, &AVR::Client::SetConnectItemEnabled, ui->actionConnect, &QAction::setEnabled);
    QObject::connect(client, &AVR::Client::SetDisconnectItemEnabled, ui->actionDisconnect, &QAction::setEnabled);
    QObject::connect(client, &AVR::Client::WriteLineToLog, ui->outputText, &QTextEdit::append);
}

MainWindow::~MainWindow()
{
    //Clean-up on close window
    delete ui;
    delete client;
}

void MainWindow::on_MoveToPos_clicked() //When clicked Move button (move for certain quantity of steps)
{
    if(!client->IsConnected())  //Safety check. Return if no connection.
    {
        ui->outputText->append("Client Error: No connection. Please, connect to AVR.");
        return;
    }

    int steps = ui->inputSteps->text().toInt(); //Get steps from input
    QString info;
    info.sprintf("Ordering AVR move for %i steps...", steps);
    ui->outputText->append(info);
    emit SendData(AVR::MessageType::MoveForNSteps, steps);  //Emit client to send MoveForNSteps message
}

void MainWindow::on_MoveToZero_clicked()    //Says AVR move it's position to zero.
{
    if(!client->IsConnected())  //Safety check. Return if no connection.
    {
        ui->outputText->append("Client Error: No connection. Please, connect to AVR.");
        return;
    }

    ui->outputText->append("Ordering AVR move to zero position...");
    emit SendData(AVR::MessageType::MoveToZero);    //Emit client to send MoveToZero message
}

void MainWindow::on_AskPosition_clicked()
{
    if(!client->IsConnected())  //Safety check. Return if no connection.
    {
        ui->outputText->append("Client Error: No connection. Please, connect to AVR.");
        return;
    }

    ui->outputText->append("Asking AVR for its position...");
    emit SendData(AVR::MessageType::GetPosition);    //Emit client to send GetPosition message
}

void MainWindow::on_actionConnect_triggered()   //Says client to connect. Host and port are taken from Connection data tab inputs.
{
    client->Connect(ui->serverHost->text(), ui->serverPort->text().toInt());
}

void MainWindow::on_actionDisconnect_triggered()    //Says client to disconnect
{
    client->Disconnect();
}

void MainWindow::on_actionClear_triggered()     //Clear our log
{
    ui->outputText->clear();
}

void MainWindow::on_actionSave_to_file_triggered()  //Saving our log to file
{
    //Calls file save dialog and return file name with it's full path
    //when file selected and Save button was clicked.
    QString fileName = QFileDialog::getSaveFileName(this, "Save output log ", "", "Text log file (*.log);;All Files (*)");
    QFile file(fileName);   //Create file interface for working with selected file
    if (file.open(QIODevice::ReadWrite))    //Write to file if it was successfuly opened.
    {
        QTextStream stream(&file);  //Connect file with text stream
        stream << ui->outputText->toPlainText() << endl;    //Write to the file through stream.
    }
    else    //Report about error if file.Open() was failed.
        QMessageBox::critical(0,"Save file error","Unable to save file.");
}

void MainWindow::on_actionAbout_triggered() //Shows information about application
{
    QMessageBox aboutBox;
    aboutBox.setText("June 2018, by Georgiy (fullmetal.a)");
    aboutBox.setWindowTitle("About AVR Emulator & Tester");
    aboutBox.setIcon(QMessageBox::Icon::Information);
    aboutBox.exec();
}

void MainWindow::on_actionAbout_Qt_triggered()  //Shows information about Qt
{
    qApp->aboutQt();
}

void MainWindow::OnSetAVRControlsEnabled(bool isEnabled)    //Enables or disables AVR controls on main form.
{
    ui->MoveToPos->setEnabled(isEnabled);
    ui->MoveToZero->setEnabled(isEnabled);
    ui->AskPosition->setEnabled(isEnabled);
    ui->inputSteps->setEnabled(isEnabled);
}
