#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    socket = new QTcpSocket(this);

    connect(socket, &QTcpSocket::readyRead, this, &MainWindow::Read);;
    connect(socket, &QTcpSocket::errorOccurred, this, &MainWindow::HandleError);
    connect(ui->pushButtonSend, &QPushButton::clicked, this, &MainWindow::Send);
    connect(ui->pushButtonConnect, &QPushButton::clicked, this, &MainWindow::Connect);

    connect(ui->lineEditIP, &QLineEdit::returnPressed, [this]() {ui->lineEditPort->setFocus(); });
    connect(ui->lineEditPort, &QLineEdit::returnPressed, this, &MainWindow::Connect);
    connect(ui->lineEditInput, &QLineEdit::returnPressed, this, &MainWindow::Send);
}

MainWindow::~MainWindow()
{
    delete socket;
    delete ui;
}

void MainWindow::Connect(){
    QString ip = ui->lineEditIP->text();
    short port = ui->lineEditPort->text().toInt();

    socket->connectToHost(ip, port);

    ui->TextEditOutput->append("Подключение к " + ip + ":" + QString::number(port) + ";");
}

void MainWindow::Read(){
    QByteArray data = socket->readAll();

    while(data.contains('\n')){
        int pos = data.indexOf('\n');
        QByteArray line = data.left(pos);
        ui->TextEditOutput->append(QString::fromUtf8(line));
        data.remove(0, pos + 1);
    }
}

void MainWindow::HandleError(QAbstractSocket::SocketError){
    ui->TextEditOutput->append("Ошибка: " + socket->errorString() + ";");
}

void MainWindow::Send(){
    QString query = ui->lineEditInput->text();
    if(query.isEmpty()) { return; }
    if(socket->state() != QAbstractSocket::ConnectedState){
        ui->TextEditOutput->append("Ошибка: нет подлючения к серверу;");
        return;
    }

    socket->write((query + "\n").toUtf8());
    ui->lineEditInput->clear();
}

