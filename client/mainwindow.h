#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    QTcpSocket* socket;
    Ui::MainWindow *ui;

private:
    void HandleError(QAbstractSocket::SocketError);
    void Read();
    void Send();
    void Connect();
};
#endif // MAINWINDOW_H
