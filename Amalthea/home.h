#ifndef HOME_H
#define HOME_H

#include <QDialog>
#include <winsock2.h>
#include <wlanapi.h>
#include <windows.h>
#include <iphlpapi.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Wlanapi.lib")
#pragma comment(lib, "IPHLPAPI.lib")
#pragma comment(lib, "Advapi32.lib")


namespace Ui {
class Home;
}

class Home : public QDialog
{
    Q_OBJECT

public:
    explicit Home(QWidget *parent = 0);
    ~Home();

    int checkWlanHosteed();
    QString getErrorMsg(int ErrCode);

private:
    Ui::Home *ui;
};

#endif // HOME_H
