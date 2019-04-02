
// WeatherWSDlg.cpp : implementation file
//

#include "stdafx.h"
#include "WeatherWS.h"
#include "WeatherWSDlg.h"
#include "afxdialogex.h"
#include "WS/soapWeatherWebServiceSoapProxy.h"
#include "WS/WeatherWebServiceSoap.nsmap"
#include <iostream>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CWeatherWSDlg dialog



CWeatherWSDlg::CWeatherWSDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_WEATHERWS_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CWeatherWSDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CWeatherWSDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDC_BUTTON_UPDATE, &CWeatherWSDlg::OnBnClickedButtonUpdate)
END_MESSAGE_MAP()


// CWeatherWSDlg message handlers

BOOL CWeatherWSDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
    AllocConsole();
    FILE *fp;
    freopen_s(&fp, "CONIN$", "r", stdin);
    freopen_s(&fp, "CONOUT$", "w", stdout);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CWeatherWSDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
        if (!icon1.IsNull()) {
            icon1.Draw(GetDlgItem(IDC_STATIC_ICON1)->GetDC()->m_hDC, rect1);
            std::cout << "draw" << std::endl;
        }
        if (!icon2.IsNull()) {
            icon2.Draw(GetDlgItem(IDC_STATIC_ICON2)->GetDC()->m_hDC, rect2);
        }
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CWeatherWSDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

int CWeatherWSDlg::GetInternetIP(std::string &strIP)
{
    const WCHAR *pTmpFile = L".\\ip.tmp";
    int ret = URLDownloadToFile(NULL, _T("http://ip.catr.cn/"), pTmpFile, BINDF_GETNEWESTVERSION, NULL);
    if (ret == S_FALSE)
        return 0;
    FILE *fp;
    if (_wfopen_s(&fp, pTmpFile, _T("rb")) != 0) {
        return 0;
    }
    fseek(fp, 0, SEEK_END);
    int ilength = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if (ilength > 0){
        char *tmpBuf = new char[ilength + 1];
        fread(tmpBuf, sizeof(char), ilength, fp);
        fclose(fp);
        //DeleteFile(_T("ip.tmp"));

        try{
            std::string subString = strstr(tmpBuf, "keyword=");
            strIP = subString.substr(sizeof("keyword=") - 1, subString.find('\"') - sizeof("keyword=") + 1);
        }
        catch (std::exception ex) {
            MessageBoxA(nullptr, ex.what(), "Error", 0);
        }
        delete[] tmpBuf;
        return 0;
    }

    fclose(fp);
    return -1;
}

int CWeatherWSDlg::GetCityFromIP(WCHAR *IP, std::wstring &city)
{
    WeatherWebServiceSoapProxy wsIP;
    _WS1__getCountryCityByIp request;
    request.soap = &wsIP;
    request.theIpAddress = const_cast<WCHAR*>(IP);
    _WS1__getCountryCityByIpResponse response;
    wsIP.getCountryCityByIp(&request, response);
    int cnts = response.getCountryCityByIpResult ? response.getCountryCityByIpResult->string.size() : 0;
    if (cnts > 1) {
        city = response.getCountryCityByIpResult->string[1];
        return 0;
    }
    return -1;
}

int CWeatherWSDlg::GetAllSupportCities(std::vector<std::wstring> &cities)
{
    WeatherWebServiceSoapProxy wsCity;
    _WS1__getSupportCity request;
    request.soap = &wsCity;
    request.byProvinceName = L"";
    _WS1__getSupportCityResponse response;
    wsCity.getSupportCity(&request, response);
    int cnts = response.getSupportCityResult ? response.getSupportCityResult->string.size() : 0;
    for (int idx = 0; idx < cnts; ++idx) {
        std::wstring city(response.getSupportCityResult->string[idx]);
        city = city.substr(0, city.find(L" ("));
        cities.push_back(city);
    }
    return 0;
}

int CWeatherWSDlg::GetCityWeather(const WCHAR *city, ST_WEATHER_INFO &weatherInfo)
{
    WeatherWebServiceSoapProxy weather;
    _WS1__getWeatherbyCityName weatherbyCityName;

    weatherbyCityName.theCityName = const_cast<WCHAR*>(city);
    weatherbyCityName.soap = &weather;
    _WS1__getWeatherbyCityNameResponse weatherbyCityNameResponse;

    weather.getWeatherbyCityName(&weatherbyCityName, weatherbyCityNameResponse);
    int cnt = weatherbyCityNameResponse.getWeatherbyCityNameResult ? 
        weatherbyCityNameResponse.getWeatherbyCityNameResult->string.size() : 0;
    for (int idx = 0; idx < cnt; ++idx){
        if (5 == idx) {
            weatherInfo.temperature = weatherbyCityNameResponse.getWeatherbyCityNameResult->string[idx];
        }
        else if (6 == idx) {
            weatherInfo.weather = weatherbyCityNameResponse.getWeatherbyCityNameResult->string[idx];
        }
        else if (7 == idx) {
            weatherInfo.wind = weatherbyCityNameResponse.getWeatherbyCityNameResult->string[idx];
        }
        else if(8 == idx) {
            weatherInfo.picStart = weatherbyCityNameResponse.getWeatherbyCityNameResult->string[idx];
        }
        else if (9 == idx) {
            weatherInfo.picEnd = weatherbyCityNameResponse.getWeatherbyCityNameResult->string[idx];
        }
        else if (10 == idx) {
            weatherInfo.detail = weatherbyCityNameResponse.getWeatherbyCityNameResult->string[idx];
            break;
        }
    }
    return 0;
}

void CWeatherWSDlg::CWeatherWSDlg::OnBnClickedButtonUpdate()
{
    // Get internet IP
    std::string strCity;
    std::string strIP;
    GetInternetIP(strIP);
    GetDlgItem(IDC_EDIT_IPADDR)->SetWindowTextW(CString(strIP.data()));

    // Get current city
    std::wstring city;
    WCHAR* pWIP = MultiByte2WideChar(strIP.data());
    GetCityFromIP(pWIP, city);
    GetDlgItem(IDC_EDIT_CITY)->SetWindowTextW(city.data());
    delete[] pWIP;

    // Get all supported cities
    std::vector<std::wstring> supportedCities;
    GetAllSupportCities(supportedCities);

    // Get weather
    std::wstring queryCityName;
    for (int idx = 0; idx < static_cast<int>(supportedCities.size()); ++idx) {
        if (-1 != city.find(supportedCities[idx])) {
            queryCityName = supportedCities[idx];
            break;
        }
    }
    ST_WEATHER_INFO info;
    GetCityWeather(queryCityName.data(), info);
    GetDlgItem(IDC_EDIT_TEMPERATURE)->SetWindowTextW(info.temperature.data());
    GetDlgItem(IDC_EDIT_WEATHER)->SetWindowTextW(info.weather.data());
    GetDlgItem(IDC_EDIT_WIND)->SetWindowTextW(info.wind.data());
    GetDlgItem(IDC_EDIT_DETAIL)->SetWindowTextW(info.detail.data());

    // Draw icons
    icon1.Destroy();
    icon2.Destroy();
    if (SUCCEEDED(icon1.Load((std::wstring(L"a_") + info.picStart).data()))) {
        GetDlgItem(IDC_STATIC_ICON1)->GetClientRect(&rect1);
        icon1.Draw(GetDlgItem(IDC_STATIC_ICON1)->GetDC()->m_hDC, rect1);
    }
    if (SUCCEEDED(icon2.Load((std::wstring(L"a_") + info.picEnd).data()))) {
        GetDlgItem(IDC_STATIC_ICON2)->GetClientRect(&rect2);
        icon2.Draw(GetDlgItem(IDC_STATIC_ICON2)->GetDC()->m_hDC, rect2);
    }
}

WCHAR* CWeatherWSDlg::MultiByte2WideChar(const char* src)
{
    int len = strlen(src);
    int wcsLen = ::MultiByteToWideChar(CP_ACP, NULL, src, len, NULL, 0);
    wchar_t* wszString = new wchar_t[wcsLen + 1];
    len = ::MultiByteToWideChar(CP_ACP, NULL, src, strlen(src), wszString, wcsLen);
    wszString[wcsLen] = '\0';
    return wszString;
}

char* CWeatherWSDlg::WideChar2MultiByte(const WCHAR* src)
{
    int len = WideCharToMultiByte(CP_ACP, 0, src, -1, NULL, 0, NULL, NULL);
    char *szUtf8 = new char[len + 1];
    WideCharToMultiByte(CP_ACP, 0, src, -1, szUtf8, len, NULL, NULL);
    szUtf8[len] = '\0';
    return szUtf8;
}