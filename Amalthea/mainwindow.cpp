#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QMessageBox>


#define NAT_PROTOCOL_TCP 6
#define NAT_PROTOCOL_UDP 17

#define ICS_Error_FailGetEvery -32
#define ICS_Error_FailGetNewEnum -33
#define ICS_Error_FailGetEnumVariant -34
#define ICS_Error_EnableSharing -35


BOOL MainWindow::IsAdmin() const
{
    return IsUserAnAdmin();
}

HRESULT DoTheWork (INetSharingManager * pNSM)
{   // add a port mapping to every firewalled or shared connection

    INetSharingEveryConnectionCollection * pNSECC = NULL;
    HRESULT hr = pNSM->get_EnumEveryConnection (&pNSECC);
    int LastErrorCode = 0;
    if (!pNSECC)
        return ICS_Error_FailGetEvery;
    else {

        // enumerate connections
        IEnumVARIANT * pEV = NULL;
        IUnknown * pUnk = NULL;
        hr = pNSECC->get__NewEnum (&pUnk);
        if (pUnk) {
            hr = pUnk->QueryInterface (__uuidof(IEnumVARIANT),
                                       (void**)&pEV);
            pUnk->Release();
        }else{
            return ICS_Error_FailGetNewEnum;
        }
        if (pEV) {
            VARIANT v;
            VariantInit (&v);
            while (S_OK == pEV->Next (1, &v, NULL)) {
                if (V_VT (&v) == VT_UNKNOWN) {
                    INetConnection * pNC = NULL;
                    V_UNKNOWN (&v)->QueryInterface
                            (__uuidof(INetConnection),
                             (void**)&pNC);
                    if (pNC) {
                        INetConnectionProps * pNCP = NULL;
                        pNSM->get_NetConnectionProps (pNC, &pNCP);
                        if (!pNCP)
                            wprintf (L"failed to get NetConnectionProps!\r\n");
                                     else {
                                         // check properties for firewalled or shared connection
                                         NETCON_MEDIATYPE MediaType;
                                         pNCP->get_MediaType(&MediaType);
                                         NETCON_STATUS Status;
                                         pNCP->get_Status(&Status);
                                         BSTR DevName;
                                         pNCP->get_DeviceName(&DevName);

                                         if (MediaType & (NCM_LAN | NCM_SHAREDACCESSHOST_LAN | NCM_PHONE)
                                                 && Status == NCS_CONNECTED
                                                 && QString(_com_util::ConvertBSTRToString(DevName)).indexOf("hosted network", 0, Qt::CaseInsensitive)==-1
                                                 && QString(_com_util::ConvertBSTRToString(DevName)).indexOf("virtual", 0, Qt::CaseInsensitive)==-1
                                                 && QString(_com_util::ConvertBSTRToString(DevName)).indexOf("teamviewer", 0, Qt::CaseInsensitive)==-1) {
                                                 // got a shared/firewalled connection
                                                 INetSharingConfiguration * pNSC = NULL;
                                                 hr = pNSM->get_INetSharingConfigurationForINetConnection (pNC, &pNSC);
                                                 if (!pNSC)
                                                 wprintf (L"can't make INetSharingConfiguration object!\r\n");
                                                 else {
                                                     hr = pNSC->EnableSharing(ICSSHARINGTYPE_PRIVATE);
                                                     if(hr!=S_OK){
                                                         LastErrorCode = ICS_Error_EnableSharing;
                                                     }else{
                                                         BSTR Name;
                                                         pNCP->get_Name(&Name);
                                                         QMessageBox msg;
                                                         msg.setText(QString("Network: %1 %2 %3").arg(_com_util::ConvertBSTRToString(Name)).arg(_com_util::ConvertBSTRToString(DevName)).arg(Status));
                                                         msg.exec();
                                                         return 0;
                                                     }
                                                     pNSC->Release();
                                                 }
                                         }
                                         pNCP->Release();
                                     }
                                     pNC->Release();
                    }
                }
                VariantClear (&v);
            }
            pEV->Release();
        }else{
            return ICS_Error_FailGetEnumVariant;
        }
        pNSECC->Release();
    }
    if(LastErrorCode!=0) return LastErrorCode;
    return hr;
}


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    no(false)
{
   if(!IsAdmin()){
       this->hide();
       QMessageBox msg(this);
       msg.setText("Для коректної роботи програмі необхідні права адміністратора!");
       msg.exec();
       this->no = true;
       ShellExecute(NULL,
           L"runas",
           qApp->applicationFilePath().toStdWString().c_str(),
           L"",
           NULL,                        // default dir
           SW_SHOWNORMAL
       );
       return;
   }

   ui->setupUi(this);

   int res = checkWlanHosteed();
   QString msg = QString("%2\r\n%1").arg(getErrorMsg(res)).arg((!IsAdmin())?"is not Admin :(":"is Admin :)");
   ui->textBrowser->setPlainText(msg);
   qDebug() << msg;

}

QString MainWindow::getErrorMsg(int ErrCode){
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
    case ICS_Error_FailGetEvery:
        return "failed to get EveryConnectionCollection!";
    case ICS_Error_FailGetNewEnum:
        return "ICSError failed to get NewEnum";
    case ICS_Error_FailGetEnumVariant:
        return "ICSError failed to get EnumVariant";
    case ICS_Error_EnableSharing:
        return "ICSError failed to enable sharring";
    }

    return "Unknown error";
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



    /*PIP_ADAPTER_INFO pAdapterInfo;
      pAdapterInfo = (IP_ADAPTER_INFO *) malloc(sizeof(IP_ADAPTER_INFO));
      ULONG buflen = sizeof(IP_ADAPTER_INFO);

      if(GetAdaptersInfo(pAdapterInfo, &buflen) == ERROR_BUFFER_OVERFLOW) {
        free(pAdapterInfo);
        pAdapterInfo = (IP_ADAPTER_INFO *) malloc(buflen);
      }

      if(GetAdaptersInfo(pAdapterInfo, &buflen) == NO_ERROR) {
        PIP_ADAPTER_INFO pAdapter = pAdapterInfo;
        while (pAdapter) {
          QString str = QString("\tAdapter Name: \t%1\n\
\tAdapter Desc: \t%2\n\
\tIP Address: \t%3\n\
\tGateway: \t%4\n").arg(pAdapter->AdapterName)
                  .arg(pAdapter->Description)
                  //.arg((byte*)pAdapter->Address)
                  .arg(pAdapter->IpAddressList.IpAddress.String)
                  .arg(pAdapter->GatewayList.IpAddress.String);
          pAdapter = pAdapter->Next;
          QMessageBox msg(this);
          msg.setText(str);
          msg.exec();
        }
      } else {
        printf("Call to GetAdaptersInfo failed.\n");
      }*/



    CoInitialize (NULL);

    // init security to enum RAS connections
    CoInitializeSecurity (NULL, -1, NULL, NULL,
                          RPC_C_AUTHN_LEVEL_PKT,
                          RPC_C_IMP_LEVEL_IMPERSONATE,
                          NULL, EOAC_NONE, NULL);

    INetSharingManager * pNSM = NULL;
    HRESULT hr = ::CoCreateInstance (__uuidof(NetSharingManager),
                                     NULL,
                                     CLSCTX_ALL,
                                     __uuidof(INetSharingManager),
                                     (void**)&pNSM);
    if(hr == S_OK){
            // in case it exists already
            //DeletePortMapping (pNSM, NAT_PROTOCOL_TCP, 555);

            // add a port mapping to every shared or firewalled connection.
                    hr = DoTheWork (pNSM);
                   pNSM->Release();
            if (hr!= S_OK){
                return hr;
            }

    }else return hr;

    return 0;
}



MainWindow::~MainWindow()
{
    delete ui;
}
