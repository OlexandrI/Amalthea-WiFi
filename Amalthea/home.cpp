#include "home.h"
#include "ui_home.h"

Home::Home(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Home)
{
    ui->setupUi(this);

    int res = checkWlanHosteed();
    QString msg = getErrorMsg(res);
    ui->textBrowser->setPlainText(msg);
    qDebug() << msg;
}

Home::~Home()
{
    delete ui;
}

QString Home::getErrorMsg(int ErrCode){
    switch(ErrCode){
    case 0:
        return "All is ok :)";
    case -1:
        return "Застаріла версія інтерфейсу.";
    case ERROR_ACCESS_DENIED:
        return "The caller does not have sufficient permissions. This error is also returned if the OpCode parameter was wlan_hosted_network_opcode_enable and the wireless Hosted Network is disabled by group policy on a domain.";
    case ERROR_BAD_PROFILE:
        return "The network connection profile used by the wireless Hosted Network is corrupted.";
    case ERROR_INVALID_HANDLE:
        return "A handle is invalid. This error is returned if the handle specified in the hClientHandle parameter was not found in the handle table.";
    case ERROR_INVALID_PARAMETER:
        return "A parameter is incorrect.";
    case ERROR_INVALID_STATE:
        return "The resource is not in the correct state to perform the requested operation. This can occur if the wireless Hosted Network was in the process of shutting down.";
    case ERROR_NOT_SUPPORTED:
        return "The request is not supported. This error is returned if the application calls the WlanHostedNetworkSetProperty function with the OpCode parameter set to wlan_hosted_network_opcode_station_profile or wlan_hosted_network_opcode_security_settings.";
    case ERROR_SERVICE_NOT_ACTIVE:
        return "The service has not been started. This error is returned if the WLAN AutoConfig Service is not running.";
    }

    return "Unknown error";
}

int Home::checkWlanHosteed(){
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
        return dwError;
    }

    // check service version
    if (WLAN_API_VERSION_MAJOR(dwServiceVersion) < WLAN_API_VERSION_MAJOR(WLAN_API_VERSION_2_0))
    {
        WlanCloseHandle(hClient, NULL);
        return -1;
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
        return dwReturnValue;
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
            return dwFailedReason;
        }
        return dwReturnValue;
    }

    return 0;
}
