
// GdiPaintDlg.h : header file
//

#pragma once

struct ST_PEN_STYLE {
    int style;
    int width;
    COLORREF color;
};

// CGdiPaintDlg dialog
class CGdiPaintDlg : public CDialogEx
{
// Construction
public:
	CGdiPaintDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_GDIPAINT_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
    BOOL m_bInitOK = FALSE;
    BOOL m_bDrawing = FALSE;
    BOOL m_bPropertyChanged = FALSE;

    CMenu m_menu;
    CDC m_memDC;
    UINT m_nImgWidth = 0;
    UINT m_nImgHeight = 0;

    int m_nCurMode = 0;
    CPen m_pen;
    CPen *m_pOldPen = nullptr;
    ST_PEN_STYLE m_penStyle;
    CBrush m_brush;
    CBrush *m_pOldBrush = nullptr;

    afx_msg void OnLoadLoadimage();
    afx_msg void OnLoadClearscreen();
    afx_msg void OnClose();
    afx_msg void OnAboutAbout();
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnShapeDot();
    afx_msg void OnShapeLine();
    afx_msg void OnShapeRectangle();
    afx_msg void OnShapeElipse();
    afx_msg void OnSize1();
    afx_msg void OnSize2();
    afx_msg void OnSize3();
    afx_msg void OnSize4();
    afx_msg void OnSize5();
    afx_msg void OnStyleSolid();
    afx_msg void OnStyleDash();
    afx_msg void OnStyleDashdot();
    afx_msg void OnStyleDot();
    afx_msg void OnColorRed();
    afx_msg void OnColorGreen();
    afx_msg void OnColorBlue();
};
