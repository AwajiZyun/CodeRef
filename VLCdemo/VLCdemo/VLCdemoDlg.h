
// VLCdemoDlg.h : header file
//

#pragma once
#include "vlc/include/vlc/vlc.h"

// CVLCdemoDlg dialog
class CVLCdemoDlg : public CDialogEx
{
// Construction
public:
	CVLCdemoDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_VLCDEMO_DIALOG };
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

private:
	libvlc_instance_t *m_pVlcInstance;		// https://www.videolan.org/developers/vlc/doc/doxygen/html/group__libvlc__core.html
	libvlc_media_player_t *m_pVlcPlayer;	// https://www.videolan.org/developers/vlc/doc/doxygen/html/group__libvlc__media__player.html
	libvlc_media_t *m_pVlcMedia;			// https://www.videolan.org/developers/vlc/doc/doxygen/html/group__libvlc__media.html

	LONGLONG m_nVideoLen;		// in seconds
	BOOL m_bPause;
	BOOL m_bFullscreen;
	CRect m_rectCtl;
public:
	afx_msg void OnClose();
	afx_msg void OnBnClickedButtonOpen();
	afx_msg void OnBnClickedButtonFullscreen();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedButtonPause();
	afx_msg void OnBnClickedButtonStop();
	CSliderCtrl m_sliderVolume;
	CSliderCtrl m_sliderVideo;
	afx_msg void OnReleasedcaptureSliderVolume(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnReleasedcaptureSliderVideo(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
