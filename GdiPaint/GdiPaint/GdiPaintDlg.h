
// GdiPaintDlg.h : header file
//

#pragma once


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
    CMenu m_menu;
    CDC m_memDC;
    UINT m_nImgWidth = 0;
    UINT m_nImgHeight = 0;
    CPen m_pen;
    CBrush m_brush;

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
};
