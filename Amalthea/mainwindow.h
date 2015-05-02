#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#define _AFXDLL

#include <QMainWindow>
#include <winsock2.h>
#include <wlanapi.h>
#include <windows.h>
#include <iphlpapi.h>
#include <Shlobj.h>
#include <NetCon.h>
#include <Objbase.h>
#include <comutil.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Wlanapi.lib")
#pragma comment(lib, "IPHLPAPI.lib")
#pragma comment(lib, "Ole32.lib")
#pragma comment(lib, "oleaut32.lib")
#pragma comment(lib, "strmiids.lib")
#pragma comment(lib, "comsuppw.lib")

namespace Ui {
class MainWindow;
}

#define BUFF_SIZE   1024

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    int checkWlanHosteed();

    QString getErrorMsg(int ErrCode);
    BOOL IsAdmin() const;

    //int setAllow();

    bool no;

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
