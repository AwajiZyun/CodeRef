
// GdiPaintDlg.cpp : implementation file
//

#include "stdafx.h"
#include "GdiPaint.h"
#include "GdiPaintDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


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

};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


// CGdiPaintDlg dialog



CGdiPaintDlg::CGdiPaintDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_GDIPAINT_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CGdiPaintDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CGdiPaintDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_COMMAND(ID_LOAD_LOADIMAGE, &CGdiPaintDlg::OnLoadLoadimage)
    ON_COMMAND(ID_LOAD_CLEARSCREEN, &CGdiPaintDlg::OnLoadClearscreen)
    ON_WM_CLOSE()
    ON_COMMAND(ID_ABOUT_ABOUTA, &CGdiPaintDlg::OnAboutAbout)
    ON_WM_ERASEBKGND()
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONUP()
    ON_WM_MOUSEMOVE()
    ON_COMMAND(ID_SHAPE_DOT, &CGdiPaintDlg::OnShapeDot)
    ON_COMMAND(ID_SHAPE_LINE, &CGdiPaintDlg::OnShapeLine)
    ON_COMMAND(ID_SHAPE_RECTANGLE, &CGdiPaintDlg::OnShapeRectangle)
    ON_COMMAND(ID_SHAPE_ELIPSE, &CGdiPaintDlg::OnShapeElipse)
    ON_COMMAND(ID_SIZE_1, &CGdiPaintDlg::OnSize1)
    ON_COMMAND(ID_SIZE_2, &CGdiPaintDlg::OnSize2)
    ON_COMMAND(ID_SIZE_3, &CGdiPaintDlg::OnSize3)
    ON_COMMAND(ID_SIZE_4, &CGdiPaintDlg::OnSize4)
    ON_COMMAND(ID_SIZE_5, &CGdiPaintDlg::OnSize5)
    ON_COMMAND(ID_STYLE_SOLID, &CGdiPaintDlg::OnStyleSolid)
    ON_COMMAND(ID_STYLE_DASH, &CGdiPaintDlg::OnStyleDash)
    ON_COMMAND(ID_STYLE_DASHDOT, &CGdiPaintDlg::OnStyleDashdot)
    ON_COMMAND(ID_STYLE_DOT, &CGdiPaintDlg::OnStyleDot)
    ON_COMMAND(ID_COLOR_RED, &CGdiPaintDlg::OnColorRed)
    ON_COMMAND(ID_COLOR_GREEN, &CGdiPaintDlg::OnColorGreen)
    ON_COMMAND(ID_COLOR_BLUE, &CGdiPaintDlg::OnColorBlue)
END_MESSAGE_MAP()


// CGdiPaintDlg message handlers

BOOL CGdiPaintDlg::OnInitDialog()
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

	// Add menu
    m_menu.LoadMenuW(IDR_MENU1);
    SetMenu(&m_menu);

    m_bInitOK = TRUE;
    TRACE("Initdialog\n");
    m_memDC.CreateCompatibleDC(NULL);
    m_memDC.SetStretchBltMode(COLORONCOLOR);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CGdiPaintDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CGdiPaintDlg::OnPaint()
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
        if (m_bInitOK) {
            TRACE("OnPaint\n");
            CDC *pDC = GetDC();
            pDC->SetStretchBltMode(COLORONCOLOR);
            CRect rect;
            GetClientRect(&rect);
            pDC->StretchBlt(0, 0, rect.Width(), rect.Height(), &m_memDC, 0, 0, m_nImgWidth, m_nImgHeight, SRCCOPY);
        }
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CGdiPaintDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CGdiPaintDlg::OnLoadLoadimage()
{
    CFileDialog dlg(TRUE, 0, L"Open image", 6, L"Bitmaps|*.bmp|Jpeg|*.jpg|Jpg|*.jpg|PNG|*.png||");
    if (IDOK == dlg.DoModal()) {
        CString filePath = dlg.GetPathName();
        CRect rect;
        GetClientRect(&rect);
        m_nImgWidth = rect.Width();
        m_nImgHeight = rect.Height();
        CDC *pDC = GetDC();
        CBitmap MemBitmap;
        MemBitmap.CreateCompatibleBitmap(pDC, m_nImgWidth, m_nImgHeight);
        CBitmap *pOldBit = m_memDC.SelectObject(&MemBitmap);
        m_memDC.FillSolidRect(0, 0, m_nImgWidth, m_nImgHeight, RGB(255, 255, 255));
        CImage img;
        if (SUCCEEDED(img.Load(filePath))) {
            TRACE(L"Load img OK\n");
            img.Draw(m_memDC.m_hDC, 0, 0, m_nImgWidth, m_nImgHeight);
            img.Destroy();
            Invalidate();
        }
        else {
            TRACE(L"Load img failed\n");
        }
        MemBitmap.DeleteObject();
        ReleaseDC(pDC);
    }
}

void CGdiPaintDlg::OnLoadClearscreen()
{
    CRect rect;
    GetClientRect(&rect);
    m_nImgWidth = rect.Width();
    m_nImgHeight = rect.Height();
    CDC *pDC = GetDC();
    CBitmap MemBitmap;
    MemBitmap.CreateCompatibleBitmap(pDC, m_nImgWidth, m_nImgHeight);
    CBitmap *pOldBit = m_memDC.SelectObject(&MemBitmap);
    m_memDC.FillSolidRect(0, 0, m_nImgWidth, m_nImgHeight, RGB(255, 255, 255));
    MemBitmap.DeleteObject();
    Invalidate();
}

void CGdiPaintDlg::OnClose()
{
    m_memDC.DeleteDC();
    CDialogEx::OnClose();
}


void CGdiPaintDlg::OnAboutAbout()
{
    CAboutDlg dlg;
    dlg.DoModal();
}


BOOL CGdiPaintDlg::OnEraseBkgnd(CDC* pDC)
{
    // TODO: Add your message handler code here and/or call default
    return TRUE;
    //return CDialogEx::OnEraseBkgnd(pDC);
}


void CGdiPaintDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
    m_bDrawing = TRUE;
    if (m_bPropertyChanged) {
        m_bPropertyChanged = FALSE;
        m_pOldPen = m_memDC.SelectObject(&m_pen);
    }
    if (0 == m_nCurMode) {
        POINT point;
        GetCursorPos(&point);
        m_memDC.MoveTo(point);
        m_memDC.LineTo(point);
    }
    

    CDialogEx::OnLButtonDown(nFlags, point);
}


void CGdiPaintDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
    m_bDrawing = FALSE;

    CDialogEx::OnLButtonUp(nFlags, point);
}


void CGdiPaintDlg::OnMouseMove(UINT nFlags, CPoint point)
{
    if (0 == m_nCurMode && m_bDrawing) {
        m_memDC.MoveTo(point);
        m_memDC.LineTo(point);
        Invalidate();
    }

    CDialogEx::OnMouseMove(nFlags, point);
}


void CGdiPaintDlg::OnShapeDot()
{
    // TODO: Add your command handler code here
}


void CGdiPaintDlg::OnShapeLine()
{
    // TODO: Add your command handler code here
}


void CGdiPaintDlg::OnShapeRectangle()
{
    // TODO: Add your command handler code here
}


void CGdiPaintDlg::OnShapeElipse()
{
    // TODO: Add your command handler code here
}


void CGdiPaintDlg::OnSize1()
{
    m_bPropertyChanged = TRUE;
    m_penStyle.width = 1;
    m_pen.DeleteObject();
    m_pen.CreatePen(m_penStyle.style, m_penStyle.width, m_penStyle.color);
}


void CGdiPaintDlg::OnSize2()
{
    m_bPropertyChanged = TRUE;
    m_penStyle.width = 2;
    m_pen.DeleteObject();
    m_pen.CreatePen(m_penStyle.style, m_penStyle.width, m_penStyle.color);
}


void CGdiPaintDlg::OnSize3()
{
    m_bPropertyChanged = TRUE;
    m_penStyle.width = 3;
    m_pen.DeleteObject();
    m_pen.CreatePen(m_penStyle.style, m_penStyle.width, m_penStyle.color);
}


void CGdiPaintDlg::OnSize4()
{
    m_bPropertyChanged = TRUE;
    m_penStyle.width = 4;
    m_pen.DeleteObject();
    m_pen.CreatePen(m_penStyle.style, m_penStyle.width, m_penStyle.color);
}


void CGdiPaintDlg::OnSize5()
{
    m_bPropertyChanged = TRUE;
    m_penStyle.width = 5;
    m_pen.DeleteObject();
    m_pen.CreatePen(m_penStyle.style, m_penStyle.width, m_penStyle.color);
}


void CGdiPaintDlg::OnStyleSolid()
{
    m_bPropertyChanged = TRUE;
    m_penStyle.style = PS_SOLID;
    m_pen.DeleteObject();
    m_pen.CreatePen(m_penStyle.style, m_penStyle.width, m_penStyle.color);
}


void CGdiPaintDlg::OnStyleDash()
{
    m_bPropertyChanged = TRUE;
    m_penStyle.style = PS_DASH;
    m_pen.DeleteObject();
    m_pen.CreatePen(m_penStyle.style, m_penStyle.width, m_penStyle.color);
}


void CGdiPaintDlg::OnStyleDashdot()
{
    m_bPropertyChanged = TRUE;
    m_penStyle.style = PS_DASHDOT;
    m_pen.DeleteObject();
    m_pen.CreatePen(m_penStyle.style, m_penStyle.width, m_penStyle.color);
}


void CGdiPaintDlg::OnStyleDot()
{
    m_bPropertyChanged = TRUE;
    m_penStyle.style = PS_DOT;
    m_pen.DeleteObject();
    m_pen.CreatePen(m_penStyle.style, m_penStyle.width, m_penStyle.color);
}


void CGdiPaintDlg::OnColorRed()
{
    m_bPropertyChanged = TRUE;
    m_penStyle.color = RGB(255, 0, 0);
    m_pen.DeleteObject();
    m_pen.CreatePen(m_penStyle.style, m_penStyle.width, m_penStyle.color);
}


void CGdiPaintDlg::OnColorGreen()
{
    m_bPropertyChanged = TRUE;
    m_penStyle.color = RGB(0, 255, 0);
    m_pen.DeleteObject();
    m_pen.CreatePen(m_penStyle.style, m_penStyle.width, m_penStyle.color);
}


void CGdiPaintDlg::OnColorBlue()
{
    m_bPropertyChanged = TRUE;
    m_penStyle.color = RGB(0, 0, 255);
    m_pen.DeleteObject();
    m_pen.CreatePen(m_penStyle.style, m_penStyle.width, m_penStyle.color);
}
