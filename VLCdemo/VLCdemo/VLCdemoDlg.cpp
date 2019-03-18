
// VLCdemoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "VLCdemo.h"
#include "VLCdemoDlg.h"
#include "afxdialogex.h"

#pragma comment(lib, "vlc/libvlc.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

const int TIMER_ID_PROGRESS = 60000;
const int TIMER_ESCAPE = 1 * 1000;

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CVLCdemoDlg dialog



CVLCdemoDlg::CVLCdemoDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_VLCDEMO_DIALOG, pParent)
	, m_pVlcInstance(nullptr)
	, m_pVlcPlayer(nullptr)
	, m_pVlcMedia(nullptr)
	, m_bPause(FALSE)
	, m_bFullscreen(FALSE)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CVLCdemoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SLIDER_VOLUME, m_sliderVolume);
	DDX_Control(pDX, IDC_SLIDER_VIDEO, m_sliderVideo);
}

BEGIN_MESSAGE_MAP(CVLCdemoDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_OPEN, &CVLCdemoDlg::OnBnClickedButtonOpen)
	ON_BN_CLICKED(IDC_BUTTON_FULLSCREEN, &CVLCdemoDlg::OnBnClickedButtonFullscreen)
	ON_BN_CLICKED(IDC_BUTTON_PAUSE, &CVLCdemoDlg::OnBnClickedButtonPause)
	ON_BN_CLICKED(IDC_BUTTON_STOP, &CVLCdemoDlg::OnBnClickedButtonStop)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_VOLUME, &CVLCdemoDlg::OnReleasedcaptureSliderVolume)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_VIDEO, &CVLCdemoDlg::OnReleasedcaptureSliderVideo)
	ON_WM_HSCROLL()
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CVLCdemoDlg message handlers

BOOL CVLCdemoDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// Initialize ctls
	m_sliderVolume.SetRangeMin(0);
	m_sliderVolume.SetRangeMax(100);
	m_sliderVideo.SetRangeMin(0);

	// Initialize vlc
	const char *args = "--video-on-top";
	m_pVlcInstance = libvlc_new(1, &args);
	if (m_pVlcInstance) {
		m_pVlcPlayer = libvlc_media_player_new(m_pVlcInstance);
		libvlc_media_player_set_hwnd(m_pVlcPlayer, GetDlgItem(IDC_STATIC_SCREEN)->m_hWnd);
	}

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CVLCdemoDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CVLCdemoDlg::OnPaint()
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
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CVLCdemoDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CVLCdemoDlg::OnClose()
{
	if (m_pVlcInstance) {
		libvlc_release(m_pVlcInstance);
	}
	if (m_pVlcPlayer) {
		libvlc_media_player_release(m_pVlcPlayer);
	}

	CDialogEx::OnClose();
}


void CVLCdemoDlg::OnBnClickedButtonOpen()
{
	if (!m_pVlcInstance || !m_pVlcPlayer) {
		return;
	}

	CFileDialog dlgFile(TRUE, L"Select a video", nullptr, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		L"Videos|*.avi;*.mp4;*.wmv;*.mkv");
	if (IDOK == dlgFile.DoModal()) {
		if (m_pVlcMedia) {
			libvlc_media_release(m_pVlcMedia);
			m_pVlcMedia = nullptr;
		}
		CT2A videoPath(dlgFile.GetPathName(), CP_UTF8);
		m_pVlcMedia = libvlc_media_new_path(m_pVlcInstance, videoPath);

		libvlc_media_player_set_media(m_pVlcPlayer, m_pVlcMedia);
		libvlc_media_player_play(m_pVlcPlayer);

		m_sliderVolume.SetPos(libvlc_audio_get_volume(m_pVlcPlayer));
		libvlc_media_parse(m_pVlcMedia);
		m_nVideoLen = libvlc_media_player_get_length(m_pVlcPlayer) / 1000;
		m_sliderVideo.SetRangeMax(static_cast<int>(m_nVideoLen));
		m_sliderVideo.SetTicFreq(1);
		m_sliderVideo.SetPageSize(5);

		CString content;
		UINT h = static_cast<UINT>(m_nVideoLen / 3600);
		UINT m = static_cast<UINT>((m_nVideoLen - h * 3600) / 60);
		UINT s = static_cast<UINT>(m_nVideoLen - h * 3600 - m * 60);
		content.Format(L"%02d:%02d:%02d ]", h, m, s);
		SetDlgItemText(IDC_STATIC_PROGRESS2, content);

		SetTimer(TIMER_ID_PROGRESS, TIMER_ESCAPE, nullptr);
	}
}


void CVLCdemoDlg::OnBnClickedButtonFullscreen()
{
	if (!m_pVlcPlayer) {
		return;
	}
	::SetWindowLong(GetSafeHwnd(), GWL_STYLE, GetWindowLong(m_hWnd, GWL_STYLE) - WS_CAPTION);
	GetDlgItem(IDC_STATIC_SCREEN)->GetClientRect(m_rectCtl);
	::ShowWindow(m_hWnd, SW_MAXIMIZE);
	CRect fullRect(0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
	GetDlgItem(IDC_STATIC_SCREEN)->MoveWindow(fullRect);

	m_bFullscreen = TRUE;
}


BOOL CVLCdemoDlg::PreTranslateMessage(MSG* pMsg)
{
	if (WM_KEYDOWN == pMsg->message && VK_ESCAPE == pMsg->wParam){
		if (m_bFullscreen) {
			::SetWindowLong(GetSafeHwnd(), GWL_STYLE, GetWindowLong(m_hWnd, GWL_STYLE) + WS_CAPTION);
			::ShowWindow(m_hWnd, SW_NORMAL);
			GetDlgItem(IDC_STATIC_SCREEN)->MoveWindow(m_rectCtl);
			m_bFullscreen = FALSE;
		}
		return TRUE;
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}


void CVLCdemoDlg::OnBnClickedButtonPause()
{
	if (!m_pVlcPlayer) {
		return;
	}

	if (m_bPause) {
		GetDlgItem(IDC_BUTTON_PAUSE)->SetWindowText(L"Pause");
		libvlc_media_player_set_pause(m_pVlcPlayer, FALSE);
		m_bPause = FALSE;
	}
	else {
		GetDlgItem(IDC_BUTTON_PAUSE)->SetWindowText(L"Resume");
		libvlc_media_player_set_pause(m_pVlcPlayer, TRUE);
		m_bPause = TRUE;
	}
}


void CVLCdemoDlg::OnBnClickedButtonStop()
{
	if (!m_pVlcPlayer) {
		return;
	}
	libvlc_media_player_stop(m_pVlcPlayer);
}


void CVLCdemoDlg::OnReleasedcaptureSliderVolume(NMHDR *pNMHDR, LRESULT *pResult)
{
	

	*pResult = 0;
}


void CVLCdemoDlg::OnReleasedcaptureSliderVideo(NMHDR *pNMHDR, LRESULT *pResult)
{


	*pResult = 0;
}


void CVLCdemoDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (reinterpret_cast<CScrollBar*>(&m_sliderVolume) == pScrollBar) {
		libvlc_audio_set_volume(m_pVlcPlayer, m_sliderVolume.GetPos());
	}
	else if (reinterpret_cast<CScrollBar*>(&m_sliderVideo) == pScrollBar) {
		int pos = m_sliderVideo.GetPos();
		libvlc_media_player_set_time(m_pVlcPlayer, pos * 1000);

		CString content;
		UINT h = pos / 3600;
		UINT m = (pos - h * 3600) / 60;
		UINT s = pos - h * 3600 - m * 60;
		content.Format(L"[ %02d:%02d:%02d /", h, m, s);
		SetDlgItemText(IDC_STATIC_PROGRESS1, content);
	}

	CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
}


void CVLCdemoDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (TIMER_ID_PROGRESS == nIDEvent) {
		int pos = static_cast<int>(libvlc_media_player_get_time(m_pVlcPlayer) / 1000);
		m_sliderVideo.SetPos(pos);

		CString content;
		UINT h = pos / 3600;
		UINT m = (pos - h * 3600) / 60;
		UINT s = pos - h * 3600 - m * 60;
		content.Format(L"[ %02d:%02d:%02d /", h, m, s);
		SetDlgItemText(IDC_STATIC_PROGRESS1, content);

		if (pos == m_nVideoLen) {
			KillTimer(TIMER_ID_PROGRESS);
		}
	}

	CDialogEx::OnTimer(nIDEvent);
}
