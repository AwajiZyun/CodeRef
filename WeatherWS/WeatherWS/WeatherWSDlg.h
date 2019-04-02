
// WeatherWSDlg.h : header file
//

#pragma once
#include <string>
#include <vector>
struct ST_WEATHER_INFO
{
    std::wstring temperature;
    std::wstring weather;
    std::wstring wind;
    std::wstring detail;
    std::wstring picStart;
    std::wstring picEnd;
};

// CWeatherWSDlg dialog
class CWeatherWSDlg : public CDialogEx
{
// Construction
public:
	CWeatherWSDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_WEATHERWS_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
    CRect rect1;
    CRect rect2;
    CImage icon1;
    CImage icon2;
    afx_msg void OnBnClickedButtonUpdate();
    WCHAR* MultiByte2WideChar(const char* src);
    char* WideChar2MultiByte(const WCHAR* src);
    int GetCityWeather(const WCHAR *city, ST_WEATHER_INFO &weatherInfo);
    int GetAllSupportCities(std::vector<std::wstring> &cities);
    int GetCityFromIP(WCHAR *IP, std::wstring &city);
    int GetInternetIP(std::string &strIP);
};
