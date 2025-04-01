
// CamAppDlg.h : header file
//

#pragma once

#include <opencv2/opencv.hpp>


// CCamAppDlg dialog
class CCamAppDlg : public CDialogEx
{
// Construction
public:
	CCamAppDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CAMAPP_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;
	cv::VideoCapture cap;
	cv::Mat m_currentFrame;
	afx_msg void OnTimer(UINT_PTR nIDEvent);

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CStatic m_camView;
	CButton m_btnStream;
	afx_msg void OnBnClickedButtonStream();
	BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	CButton m_btnZoomOut;
	CButton m_btnZoomIn;
	afx_msg void OnBnClickedButtonZoomout();
	afx_msg void OnBnClickedButtonZoomin();
};
