#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

   int res = checkWlanHosteed();
    if(res == 0){
        ui->label->setText("All is ok!");
        qDebug() << "All is ok!";
    }else{
        ui->label->setText(QString("No :( %1").arg(res));
        qDebug() << "No :(";
    }

}

int MainWindow::checkWlanHosteed(){
    DWORD dwError = 0;
    DWORD dwServiceVersion = 0;
    HANDLE hClient = NULL;

    if (ERROR_SUCCESS != (dwError = WlanOpenHandle(
                        WLAN_API_VERSION,
                        NULL,               // reserved
                        &dwServiceVersion,
                        &hClient
                        )))
    {
        return 1;
    }

    // check service version
    if (WLAN_API_VERSION_MAJOR(dwServiceVersion) < WLAN_API_VERSION_MAJOR(WLAN_API_VERSION_2_0))
    {
        WlanCloseHandle(hClient, NULL);
        return 2;
    }


    std::string strSSID = "test";
    std::string strSecondaryKey = "123456780";

    // Set the network mode to allow
    BOOL bIsAllow = TRUE;
    WLAN_HOSTED_NETWORK_REASON dwFailedReason;
    DWORD dwReturnValue = WlanHostedNetworkSetProperty(hClient,
                                                       wlan_hosted_network_opcode_enable,
                                                       sizeof(BOOL),
                                                       &bIsAllow,
                                                       &dwFailedReason,
                                                       NULL);

    if(ERROR_SUCCESS != dwReturnValue)
    {
        return -1;
    }

    // Set the network SSID and the maximum number of connections
    WLAN_HOSTED_NETWORK_CONNECTION_SETTINGS wlanConnectionSetting;
    memset(&wlanConnectionSetting, 0, sizeof(WLAN_HOSTED_NETWORK_CONNECTION_SETTINGS));

    // The SSID field in WLAN_HOSTED_NETWORK_CONNECTION_SETTINGS must be of type ANSI, so if the program uses the Unicode, you need to do the conversion.
#ifdef _UNICODE
    WideCharToMultiByte(CP_ACP,
                        0,
                        strSSID.c_str(),   // Save SSID CString types
                        strSSID.length(),    // SSID the length of a string
                        (LPSTR)wlanConnectionSetting.hostedNetworkSSID.ucSSID,
                        32,
                        NULL,
                        NULL);
#else
    memcpy(wlanConnectionSetting.hostedNetworkSSID.ucSSID, strSSID.c_str(), strlen(strSSID.c_str()));
#endif

    wlanConnectionSetting.hostedNetworkSSID.uSSIDLength = strlen((const char*)wlanConnectionSetting.hostedNetworkSSID.ucSSID);
    wlanConnectionSetting.dwMaxNumberOfPeers = 100;

    dwReturnValue = WlanHostedNetworkSetProperty(hClient,
                                                 wlan_hosted_network_opcode_connection_settings,
                                                 sizeof(WLAN_HOSTED_NETWORK_CONNECTION_SETTINGS),
                                                 &wlanConnectionSetting,
                                                 &dwFailedReason,
                                                 NULL);
    if(ERROR_SUCCESS != dwReturnValue)
    {
        return dwReturnValue;
    }

    UCHAR keyBuf[100] = {0};
#ifdef _UNICODE
    WideCharToMultiByte(CP_ACP,
                        0,
                        strSecondaryKey.c_str(),
                        strSecondaryKey.length(),
                        (LPSTR)keyBuf,
                        100,
                        NULL,
                        NULL);
#else
    memcpy(keyBuf, strSecondaryKey.c_str(), strSecondaryKey.length());
#endif
    dwReturnValue = WlanHostedNetworkSetSecondaryKey(hClient,
                                                     strlen((const char*)keyBuf) + 1,
                                                     keyBuf,
                                                     TRUE,
                                                     FALSE,
                                                     &dwFailedReason,
                                                     NULL);
    if(ERROR_SUCCESS != dwReturnValue)
    {
        return dwReturnValue;
    }

    dwReturnValue = WlanHostedNetworkStartUsing(hClient, &dwFailedReason, NULL);
    if(ERROR_SUCCESS != dwReturnValue)
    {
        if (wlan_hosted_network_reason_interface_unavailable == dwFailedReason)
        {
            return 0;
        }
        return ERROR_SUCCESS;
    }

    return 0;
}

MainWindow::~MainWindow()
{
    delete ui;
}
