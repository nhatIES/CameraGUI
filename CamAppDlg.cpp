
// CamAppDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "CamApp.h"
#include "CamAppDlg.h"
#include "afxdialogex.h"
#include <iostream> 

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


bool m_isStreaming = false;

float zoomSpeed = 0.0025f;
float m_zoomLevel = 1.0f;
const float m_zoomMin = 1.0f;
const float m_zoomMax = 3.0f;
cv::Point2f m_zoomCenter = cv::Point2f(0.5f, 0.5f);

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
public:
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


// CCamAppDlg dialog


CCamAppDlg::CCamAppDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CAMAPP_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CCamAppDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CAM_VIEW, m_camView);
	DDX_Control(pDX, IDC_BUTTON_STREAM, m_btnStream);
	DDX_Control(pDX, IDC_BUTTON_ZOOMOUT, m_btnZoomOut);
	DDX_Control(pDX, IDC_BUTTON_ZOOMIN, m_btnZoomIn);
}

BEGIN_MESSAGE_MAP(CCamAppDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_WM_MOUSEWHEEL()
	ON_BN_CLICKED(IDC_BUTTON_STREAM, &CCamAppDlg::OnBnClickedButtonStream)
	ON_BN_CLICKED(IDC_BUTTON_ZOOMOUT, &CCamAppDlg::OnBnClickedButtonZoomout)
	ON_BN_CLICKED(IDC_BUTTON_ZOOMIN, &CCamAppDlg::OnBnClickedButtonZoomin)
END_MESSAGE_MAP()


// CCamAppDlg message handlers

BOOL CCamAppDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	/* Change type of ICD_CAM_VIEW object */
	CStatic* pStatic = (CStatic*)GetDlgItem(IDC_CAM_VIEW);
	pStatic->ModifyStyle(0xF, SS_BITMAP);
	pStatic->ModifyStyle(0xF, SS_BLACKFRAME);


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

	// TODO: Add extra initialization here

	return TRUE;  // return TRUE  unless you set the focus to a control
}


void DrawMatToControl(cv::Mat& frame, CStatic& control)
{
	if (frame.empty()) return;

	cv::Mat rgb;
	cv::cvtColor(frame, rgb, cv::COLOR_BGR2RGB);
	CString str;
	str.Format(_T("[INFO] Camera frame size: %d x %d\n"), frame.cols, frame.rows);
	OutputDebugString(str);

	CRect rect;
	control.GetClientRect(&rect);
	int w = rect.Width();
	int h = rect.Height();

	BITMAPINFO bmi;
	ZeroMemory(&bmi, sizeof(BITMAPINFO));
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = rgb.cols;
	bmi.bmiHeader.biHeight = -rgb.rows; 
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 24;
	bmi.bmiHeader.biCompression = BI_RGB;

	int border = 1;
	CDC* pDC = control.GetDC();
	if (pDC)
	{
		StretchDIBits(
			pDC->GetSafeHdc(),
			border, border, w - 2 * border, h - 2 * border,
			0, 0, rgb.cols, rgb.rows,
			rgb.data,
			&bmi,
			DIB_RGB_COLORS,
			SRCCOPY
		);
		control.ReleaseDC(pDC);
	}
}

cv::Mat ZoomFrame(const cv::Mat& src, float zoom, cv::Point2f zoomCenter)
{
	if (zoom <= 1.0f) return src;

	int w = src.cols;
	int h = src.rows;

	int newW = static_cast<int>(w / zoom);
	int newH = static_cast<int>(h / zoom);

	int centerX = static_cast<int>(zoomCenter.x * w);
	int centerY = static_cast<int>(zoomCenter.y * h);

	int x = centerX - newW / 2;
	int y = centerY - newH / 2;

	x = std::max(0, std::min(w - newW, x));
	y = std::max(0, std::min(h - newH, y));

	cv::Rect roi(x, y, newW, newH);
	cv::Mat cropped = src(roi);

	cv::Mat resized;
	cv::resize(cropped, resized, src.size());
	return resized;
}


void CCamAppDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == 1)
	{
		cv::Mat frame;
		cap >> frame;
		if (!frame.empty())
		{
			cv::Mat zoomed = ZoomFrame(frame, m_zoomLevel, m_zoomCenter);
			DrawMatToControl(zoomed, m_camView);
		}
	}

	CDialogEx::OnTimer(nIDEvent);
}



void CCamAppDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CCamAppDlg::OnPaint()
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
HCURSOR CCamAppDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CCamAppDlg::OnBnClickedButtonStream()
{
	if (!m_isStreaming)
	{
		/* Start streaming */
		cap.open(0);
		cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
		cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
		if (cap.isOpened())
		{
			SetTimer(1, 30, NULL);
			m_isStreaming = true;
			m_btnStream.SetWindowTextW(_T("STOP"));
		}
		else
		{
			AfxMessageBox(_T("Camera is not supported!"));
		}
	}
	else
	{
		/* Stop streaming */
		KillTimer(1);
		cap.release();
		m_isStreaming = false;
		m_btnStream.SetWindowTextW(_T("START"));

		CClientDC dc(&m_camView);
		CRect rect;
		m_camView.GetClientRect(&rect);
		int border = 1;
		rect.DeflateRect(border, border);
		dc.FillSolidRect(&rect, RGB(255, 255, 255));
	}
}


template <typename T>
T clamp(const T& val, const T& low, const T& high)
{
	return (val < low) ? low : (val > high) ? high : val;
}

BOOL CCamAppDlg::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	if (!m_isStreaming)
		return CDialogEx::OnMouseWheel(nFlags, zDelta, pt);

	ScreenToClient(&pt);

	CRect camRect;
	m_camView.GetWindowRect(&camRect);
	ScreenToClient(&camRect);

	if (!camRect.PtInRect(pt))
		return CDialogEx::OnMouseWheel(nFlags, zDelta, pt);

	int border = 1;
	int drawW = camRect.Width() - 2 * border;
	int drawH = camRect.Height() - 2 * border;

	float cursorRelX = (pt.x - camRect.left - border) / (float)drawW;
	float cursorRelY = (pt.y - camRect.top - border) / (float)drawH;

	cursorRelX = clamp(cursorRelX, 0.0f, 1.0f);
	cursorRelY = clamp(cursorRelY, 0.0f, 1.0f);

	float deltaZoom = zDelta * zoomSpeed;
	float newZoom = clamp(m_zoomLevel + deltaZoom, m_zoomMin, m_zoomMax);

	if (newZoom != m_zoomLevel)
	{
		float visibleW = 1.0f / m_zoomLevel;
		float visibleH = 1.0f / m_zoomLevel;

		float topLeftX = m_zoomCenter.x - visibleW / 2.0f;
		float topLeftY = m_zoomCenter.y - visibleH / 2.0f;

		topLeftX = clamp(topLeftX, 0.0f, 1.0f - visibleW);
		topLeftY = clamp(topLeftY, 0.0f, 1.0f - visibleH);

		float absoluteX = topLeftX + cursorRelX * visibleW;
		float absoluteY = topLeftY + cursorRelY * visibleH;

		m_zoomCenter = cv::Point2f(clamp(absoluteX, 0.0f, 1.0f), clamp(absoluteY, 0.0f, 1.0f));
		m_zoomLevel = newZoom;
	}

	return TRUE;
}

void CCamAppDlg::OnBnClickedButtonZoomout()
{
	if (!m_isStreaming) return;

	float step = 0.15f;
	float newZoom = clamp(m_zoomLevel - step, m_zoomMin, m_zoomMax);

	if (newZoom != m_zoomLevel)
	{
		m_zoomLevel = newZoom;
		m_zoomCenter = cv::Point2f(0.5f, 0.5f); 
	}
}

void CCamAppDlg::OnBnClickedButtonZoomin()
{
	if (!m_isStreaming) return;

	float step = 0.15f;
	float newZoom = clamp(m_zoomLevel + step, m_zoomMin, m_zoomMax);

	if (newZoom != m_zoomLevel)
	{
		m_zoomLevel = newZoom;
		m_zoomCenter = cv::Point2f(0.5f, 0.5f);
	}
}
