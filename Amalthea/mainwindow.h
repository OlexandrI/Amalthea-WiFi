#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#define _AFXDLL

#include <QMainWindow>
#include <winsock2.h>
#include <wlanapi.h>
#include <windows.h>
#include <iphlpapi.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Wlanapi.lib")
#pragma comment(lib, "IPHLPAPI.lib")

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    int checkWlanHosteed();

    //int setAllow();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
