// ExperimentImgDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ExperimentImg.h"
#include "ExperimentImgDlg.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <CL/cl.h>
#include "afxdialogex.h"
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/highgui.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/calib3d/calib3d_c.h>
#include "ldb.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//extern "C" void add_host(int *host_a, int *host_b, int *host_c); //interface for kernel function
extern "C" void MedianFilter_host(int *pixel, int Width, int Height);


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
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


// CExperimentImgDlg 对话框


CExperimentImgDlg::CExperimentImgDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_EXPERIMENTIMG_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	//加载对话框的时候初始化
	m_pImgSrc = NULL;
	m_pImgCpy = NULL;
	m_nThreadNum = 1;
	m_pThreadParam = new ThreadParam[MAX_THREAD];
	srand(time(0));

	//m_filepath = "";             //文件路径
	//m_isplay = FALSE;            //判断是否播放中
	//m_framepos = 0;              //帧位置
	//m_totalframes = 0;           //帧总数
	//m_fps = 0;                   //帧率
	//m_speed = 20;                //播放速度,初始为20
}

void CExperimentImgDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	//	DDX_Control(pDX, IDC_EDIT_INFO, mEditInfo);
	DDX_Control(pDX, IDC_PICTURE, mPictureControl);
	DDX_Control(pDX, IDC_CHECK_100, m_CheckCirculation);
	DDX_Control(pDX, IDC_PICTURE1, mPictureControl1);
	DDX_Control(pDX, IDC_HORI_SCROLLBAR, m_horiScrollbar);
}

BEGIN_MESSAGE_MAP(CExperimentImgDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_OPEN, &CExperimentImgDlg::OnBnClickedButtonOpen)
//	ON_EN_CHANGE(IDC_EDIT1, &CExperimentImgDlg::OnEnChangeEdit1)
//	ON_EN_CHANGE(IDC_EDIT_INFO, &CExperimentImgDlg::OnEnChangeEditInfo)
ON_CBN_SELCHANGE(IDC_COMBO_FUNCTION, &CExperimentImgDlg::OnCbnSelchangeComboFunction)
ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_THREADNUM, &CExperimentImgDlg::OnNMCustomdrawSliderThreadnum)
ON_BN_CLICKED(IDC_BUTTON_PROCESS, &CExperimentImgDlg::OnBnClickedButtonProcess)
ON_MESSAGE(WM_NOISE, &CExperimentImgDlg::OnNoiseThreadMsgReceived)
ON_MESSAGE(WM_MEDIAN_FILTER, &CExperimentImgDlg::OnMedianFilterThreadMsgReceived)
ON_STN_CLICKED(IDC_PICTURE1, &CExperimentImgDlg::OnStnClickedPicture1)
ON_MESSAGE(WM_VIDEO, &CExperimentImgDlg::OnVideoViewThreadMsgReceived)
ON_BN_CLICKED(IDC_OPEN, &CExperimentImgDlg::OnBnClickedOpen)
ON_BN_CLICKED(IDC_PLAY, &CExperimentImgDlg::OnBnClickedPlay)
ON_BN_CLICKED(IDC_EXIT, &CExperimentImgDlg::OnBnClickedExit)
ON_WM_TIMER()
ON_NOTIFY(NM_THEMECHANGED, IDC_HORI_SCROLLBAR, &CExperimentImgDlg::OnNMThemeChangedHoriScrollbar)
ON_WM_HSCROLL()
END_MESSAGE_MAP()


// CExperimentImgDlg 消息处理程序

BOOL CExperimentImgDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);
	
	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
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

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// 设置水平滚动条的滚动范围为1到100
	m_horiScrollbar.SetScrollRange(1, 100);
	// 设置水平滚动条的初始位置为20
	m_horiScrollbar.SetScrollPos(1);
	// 在编辑框中显示20
	SetDlgItemInt(IDC_HSCROLL_EDIT, 1);

	cv::namedWindow("view", cv::WINDOW_AUTOSIZE);
	hWndDisplay = (HWND)cvGetWindowHandle("view");
	HWND hParent = ::GetParent(hWndDisplay);
	::SetParent(hWndDisplay, GetDlgItem(IDC_PICTURE)->m_hWnd);
	::ShowWindow(hParent, SW_HIDE);
	cv::namedWindow("view1", cv::WINDOW_AUTOSIZE);
	hWndDisplay1 = (HWND)cvGetWindowHandle("view1");
	HWND hParent1 = ::GetParent(hWndDisplay1);
	::SetParent(hWndDisplay1, GetDlgItem(IDC_PICTURE1)->m_hWnd);
	::ShowWindow(hParent1, SW_HIDE);

	// TODO: 在此添加额外的初始化代码
//	mEditInfo.SetWindowTextW(CString("File Path"));
	CComboBox * cmb_function = ((CComboBox*)GetDlgItem(IDC_COMBO_FUNCTION));
	cmb_function->AddString(_T("椒盐噪声"));
	cmb_function->AddString(_T("视频检测"));
	cmb_function->AddString(_T("中值滤波"));
	cmb_function->SetCurSel(0);

	CComboBox * cmb_thread = ((CComboBox*)GetDlgItem(IDC_COMBO_THREAD));
	cmb_thread->InsertString(0, _T("WIN多线程"));
	cmb_thread->InsertString(1, _T("OpenMP"));
	cmb_thread->InsertString(2, _T("CUDA"));
	cmb_thread->InsertString(3, _T("OpenCL"));
	cmb_thread->SetCurSel(0);

	CSliderCtrl * slider = ((CSliderCtrl*)GetDlgItem(IDC_SLIDER_THREADNUM));
	slider->SetRange(1, MAX_THREAD, TRUE);
	slider->SetPos(MAX_THREAD);

	//ShowWindow(SW_MAXIMIZE);
	AfxBeginThread((AFX_THREADPROC)&CExperimentImgDlg::Update, this);
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CExperimentImgDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CExperimentImgDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
		if (m_pImgSrc != NULL)
		{
			int height;
			int width;
			CRect rect;
			CRect rect1;
			height = m_pImgSrc->GetHeight();
			width = m_pImgSrc->GetWidth();

			mPictureControl.GetClientRect(&rect);
			CDC *pDC = mPictureControl.GetDC();
			SetStretchBltMode(pDC->m_hDC, STRETCH_HALFTONE);

			if (width <= rect.Width() && height <= rect.Width())
			{
				rect1 = CRect(rect.TopLeft(), CSize(width, height));
				m_pImgSrc->StretchBlt(pDC->m_hDC, rect1, SRCCOPY);
			}
			else
			{
				float xScale = (float)rect.Width() / (float)width;
				float yScale = (float)rect.Height() / (float)height;
				float ScaleIndex = (xScale <= yScale ? xScale : yScale);
				rect1 = CRect(rect.TopLeft(), CSize((int)width*ScaleIndex, (int)height*ScaleIndex));
				//将picture control调整到图像缩放后的大小
//				CWnd *pWnd;
//				pWnd = GetDlgItem(IDC_PICTURE);
//				pWnd->MoveWindow(CRect((int)rect.top, (int)rect.left, (int)width*ScaleIndex, (int)height*ScaleIndex));
				m_pImgSrc->StretchBlt(pDC->m_hDC, rect1, SRCCOPY);
			}
			ReleaseDC(pDC);
		}
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CExperimentImgDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

UINT CExperimentImgDlg::Update(void* p)
{
	while (1)
	{
		Sleep(200);
		CExperimentImgDlg* dlg = (CExperimentImgDlg*)p;
		if (dlg->m_pImgSrc != NULL)
		{
			int height;
			int width;
			CRect rect;
			CRect rect1;
			height = dlg->m_pImgSrc->GetHeight();
			width = dlg->m_pImgSrc->GetWidth();

			dlg->mPictureControl.GetClientRect(&rect);
			CDC *pDC = dlg->mPictureControl.GetDC();
			SetStretchBltMode(pDC->m_hDC, STRETCH_HALFTONE);

			if (width <= rect.Width() && height <= rect.Height())
			{
				rect1 = CRect(rect.TopLeft(), CSize(width, height));
				dlg->m_pImgSrc->StretchBlt(pDC->m_hDC, rect1, SRCCOPY);
			}
			else
			{
				float xScale = (float)rect.Width() / (float)width;
				float yScale = (float)rect.Height() / (float)height;
				float ScaleIndex = (xScale <= yScale ? xScale : yScale);
				rect1 = CRect(rect.TopLeft(), CSize((int)width*ScaleIndex, (int)height*ScaleIndex));
				dlg->m_pImgSrc->StretchBlt(pDC->m_hDC, rect1, SRCCOPY);
			}
			dlg->ReleaseDC(pDC);
		}
	}
	return 0;
}

UINT CExperimentImgDlg::UpdateCpy(void* p)
{
	while (1)
	{
		Sleep(200);
		CExperimentImgDlg* dlg = (CExperimentImgDlg*)p;

		if (dlg->m_pImgCpy != NULL)
		{
			int height;
			int width;
			CRect rect;
			CRect rect1;
			height = dlg->m_pImgCpy->GetHeight();
			width = dlg->m_pImgCpy->GetWidth();

			dlg->mPictureControl1.GetWindowRect(&rect);
			CDC *pDC = dlg->mPictureControl1.GetDC();
			SetStretchBltMode(pDC->m_hDC, STRETCH_HALFTONE);

			if (width <= rect.Width() && height <= rect.Height())
			{
				rect1 = CRect(rect.TopLeft(), CSize(width, height));
				dlg->m_pImgCpy->StretchBlt(pDC->m_hDC, rect1, SRCCOPY);
			}
			else
			{
				float xScale = (float)rect.Width() / (float)width;
				float yScale = (float)rect.Height() / (float)height;
				float ScaleIndex = (xScale <= yScale ? xScale : yScale);
				rect1 = CRect(rect.TopLeft(), CSize((int)width*ScaleIndex * 2, (int)height*ScaleIndex*2.5));
				dlg->m_pImgCpy->StretchBlt(pDC->m_hDC, rect1, SRCCOPY);
			}
			dlg->ReleaseDC(pDC);
		}
	}
	return 0;
}

void CExperimentImgDlg::ThreadDraw(DrawPara *p)
{
	CRect rect;
	GetClientRect(&rect);
	CDC     memDC;             // 用于缓冲绘图的内存画笔  
	CBitmap memBitmap;         // 用于缓冲绘图的内存画布
	memDC.CreateCompatibleDC(p->pDC);  // 创建与原画笔兼容的画笔
	memBitmap.CreateCompatibleBitmap(p->pDC, p->width, p->height);  // 创建与原位图兼容的内存画布
	memDC.SelectObject(&memBitmap);      // 创建画笔与画布的关联
	memDC.FillSolidRect(rect, p->pDC->GetBkColor());
	p->pDC->SetStretchBltMode(HALFTONE);
	// 将pImgSrc的内容缩放画到内存画布中
	p->pImgSrc->StretchBlt(memDC.m_hDC, 0, 0, p->width, p->height);

	// 将已画好的画布复制到真正的缓冲区中
	p->pDC->BitBlt(p->oriX, p->oriY, p->width, p->height, &memDC, 0, 0, SRCCOPY);
	memBitmap.DeleteObject();
	memDC.DeleteDC();
}

void CExperimentImgDlg::ImageCopy(CImage* pImgSrc, CImage* pImgDrt)
{
	int MaxColors = pImgSrc->GetMaxColorTableEntries();
	RGBQUAD* ColorTab;
	ColorTab = new RGBQUAD[MaxColors];

	CDC *pDCsrc, *pDCdrc;
	if (!pImgDrt->IsNull())
	{
		pImgDrt->Destroy();
	}
	pImgDrt->Create(pImgSrc->GetWidth(), pImgSrc->GetHeight(), pImgSrc->GetBPP(), 0);

	if (pImgSrc->IsIndexed())
	{
		pImgSrc->GetColorTable(0, MaxColors, ColorTab);
		pImgDrt->SetColorTable(0, MaxColors, ColorTab);
	}

	pDCsrc = CDC::FromHandle(pImgSrc->GetDC());
	pDCdrc = CDC::FromHandle(pImgDrt->GetDC());
	pDCdrc->BitBlt(0, 0, pImgSrc->GetWidth(), pImgSrc->GetHeight(), pDCsrc, 0, 0, SRCCOPY);
	pImgSrc->ReleaseDC();
	pImgDrt->ReleaseDC();
	delete ColorTab;

}

void CExperimentImgDlg::OnBnClickedButtonOpen()
{
	// TODO: 在此添加控件通知处理程序代码
	TCHAR szFilter[] = _T("JPEG(*jpg)|*.jpg|*.bmp|*.png|TIFF(*.tif)|*.tif|All Files （*.*）|*.*||");
	CString filePath("");
	
	CFileDialog fileOpenDialog(TRUE, NULL, NULL, OFN_HIDEREADONLY, szFilter);
	if (fileOpenDialog.DoModal() == IDOK)
	{
		VERIFY(filePath = fileOpenDialog.GetPathName());
		CString strFilePath(filePath);
//		mEditInfo.SetWindowTextW(strFilePath);	//在文本框中显示图像路径

		if (m_pImgSrc != NULL)
		{
			m_pImgSrc->Destroy();
			delete m_pImgSrc;
		}
		m_pImgSrc = new CImage();
		m_pImgSrc->Load(strFilePath);

		//源图像参数
		BYTE* srcPtr = (BYTE*)m_pImgSrc->GetBits();
		int srcBitsCount = m_pImgSrc->GetBPP();
		int srcWidth = m_pImgSrc->GetWidth();
		int srcHeight = m_pImgSrc->GetHeight();
		int srcPitch = m_pImgSrc->GetPitch();

		//销毁原有图像
		if (m_pImgCpy != NULL) {
			m_pImgCpy->Destroy();
			delete m_pImgCpy;
		}
		m_pImgCpy = new CImage();
		//创建CImage类新图像并分配内存
		if (srcBitsCount == 32)   //支持alpha通道
		{
			m_pImgCpy->Create(srcWidth, srcHeight, srcBitsCount, 1);
		}
		else
		{
			m_pImgCpy->Create(srcWidth, srcHeight, srcBitsCount, 0);
		}
		//加载调色板
		if (srcBitsCount <= 8 && m_pImgSrc->IsIndexed())//需要调色板
		{
			RGBQUAD pal[256];
			int nColors = m_pImgSrc->GetMaxColorTableEntries();
			if (nColors > 0)
			{
				m_pImgSrc->GetColorTable(0, nColors, pal);
				m_pImgCpy->SetColorTable(0, nColors, pal);//复制调色板程序
			}
		}

		this->Invalidate();
	}


}

void CExperimentImgDlg::OnCbnSelchangeComboFunction()
{
	// TODO: 在此添加控件通知处理程序代码
}

void CExperimentImgDlg::OnNMCustomdrawSliderThreadnum(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	CSliderCtrl *slider = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_THREADNUM);
	CString text("");
	m_nThreadNum = slider->GetPos();
	text.Format(_T("%d"), m_nThreadNum);
	GetDlgItem(IDC_STATIC_THREADNUM)->SetWindowText(text);
	*pResult = 0;
}

void CExperimentImgDlg::OnBnClickedButtonProcess()
{
	// TODO: 在此添加控件通知处理程序代码
	CComboBox* cmb_function = ((CComboBox*)GetDlgItem(IDC_COMBO_FUNCTION));
	int func = cmb_function->GetCurSel();
	switch (func)
	{
	case 0:  //椒盐噪声
		AddNoise();
		break;
	case 1:	//视频检测
		VideoView();
		break;
	case 2: //自适应中值滤波
		MedianFilter();
		break;
	default:
		break;
	}
}

void CExperimentImgDlg::AddNoise()
{
	CComboBox* cmb_thread = ((CComboBox*)GetDlgItem(IDC_COMBO_THREAD));
	int thread = cmb_thread->GetCurSel();
	CButton* clb_circulation = ((CButton*)GetDlgItem(IDC_CHECK_CIRCULATION));
	int circulation = clb_circulation->GetCheck() == 0 ? 1:100;
	startTime = CTime::GetTickCount();
	switch (thread)
	{
	case 0://win多线程
	{
		//int subLength = m_pImgSrc->GetWidth() * m_pImgSrc->GetHeight() / m_nThreadNum;
		AddNoise_WIN();
		//for (int i = 0; i < circulation; i++)
		//{
		//	for (int i = 0; i < m_nThreadNum; ++i)
		//	{
		//		m_pThreadParam[i].startIndex = i * subLength;
		//		m_pThreadParam[i].endIndex = i != m_nThreadNum - 1 ?
		//			(i + 1) * subLength - 1 : m_pImgSrc->GetWidth() * m_pImgSrc->GetHeight() - 1;
		//		m_pThreadParam[i].src = m_pImgSrc;
		//		AfxBeginThread((AFX_THREADPROC)&ImageProcess::addNoise, &m_pThreadParam[i]);
		//	}
		//}
		//CTime endTime = CTime::GetTickCount();
		//CString timeStr;
		//timeStr.Format(_T("处理%d次,耗时:%dms"), circulation, endTime - startTime);
		//AfxMessageBox(timeStr);
	}

	break;

	case 1://openmp
	{
		int subLength = m_pImgSrc->GetWidth() * m_pImgSrc->GetHeight() / m_nThreadNum;

		#pragma omp parallel for num_threads(m_nThreadNum)
			for (int i = 0; i < m_nThreadNum; ++i)
			{
				m_pThreadParam[i].startIndex = i * subLength;
				m_pThreadParam[i].endIndex = i != m_nThreadNum - 1 ?
					(i + 1) * subLength - 1 : m_pImgSrc->GetWidth() * m_pImgSrc->GetHeight() - 1;
				m_pThreadParam[i].src = m_pImgSrc;
				ImageProcess::addNoise(&m_pThreadParam[i]);
			}
	}

	break;

	case 2://cuda
		break;
	}
}

void CExperimentImgDlg::AddNoise_WIN()
{
	int subLength = m_pImgSrc->GetWidth() * m_pImgSrc->GetHeight() / m_nThreadNum;
	for (int i = 0; i < m_nThreadNum; ++i)
	{
		m_pThreadParam[i].startIndex = i * subLength;
		m_pThreadParam[i].endIndex = i != m_nThreadNum - 1 ?
			(i + 1) * subLength - 1 : m_pImgSrc->GetWidth() * m_pImgSrc->GetHeight() - 1;
		m_pThreadParam[i].src = m_pImgSrc;
		AfxBeginThread((AFX_THREADPROC)&ImageProcess::addNoise, &m_pThreadParam[i]);
	}
}

void CExperimentImgDlg::VideoView()
{
	CComboBox* cmb_thread = ((CComboBox*)GetDlgItem(IDC_COMBO_THREAD));
	int thread = cmb_thread->GetCurSel();
	CButton* clb_circulation = ((CButton*)GetDlgItem(IDC_CHECK_CIRCULATION));
	int circulation = clb_circulation->GetCheck() == 0 ? 1 : 100;
	startTime = CTime::GetTickCount();
	switch (thread)
	{
	case 0://win多线程
	{
		//int subLength = m_pImgSrc->GetWidth() * m_pImgSrc->GetHeight() / m_nThreadNum;
		VideoView_WIN();
		//for (int i = 0; i < circulation; i++)
		//{
		//	for (int i = 0; i < m_nThreadNum; ++i)
		//	{
		//		m_pThreadParam[i].startIndex = i * subLength;
		//		m_pThreadParam[i].endIndex = i != m_nThreadNum - 1 ?
		//			(i + 1) * subLength - 1 : m_pImgSrc->GetWidth() * m_pImgSrc->GetHeight() - 1;
		//		m_pThreadParam[i].src = m_pImgSrc;
		//		AfxBeginThread((AFX_THREADPROC)&ImageProcess::addNoise, &m_pThreadParam[i]);
		//	}
		//}
		//CTime endTime = CTime::GetTickCount();
		//CString timeStr;
		//timeStr.Format(_T("处理%d次,耗时:%dms"), circulation, endTime - startTime);
		//AfxMessageBox(timeStr);
	}

	break;

	case 1://openmp
	{
		int subLength = m_pImgSrc->GetWidth() * m_pImgSrc->GetHeight() / m_nThreadNum;

#pragma omp parallel for num_threads(m_nThreadNum)
		for (int i = 0; i < m_nThreadNum; ++i)
		{
			m_pThreadParam[i].startIndex = i * subLength;
			m_pThreadParam[i].endIndex = i != m_nThreadNum - 1 ?
				(i + 1) * subLength - 1 : m_pImgSrc->GetWidth() * m_pImgSrc->GetHeight() - 1;
			m_pThreadParam[i].src = m_pImgSrc;
			ImageProcess::videoView(&m_pThreadParam[i]);
		}
	}

	break;

	case 2://cuda
		break;
	}
}

void CExperimentImgDlg::VideoView_WIN()
{
	int subLength = m_pImgSrc->GetWidth() * m_pImgSrc->GetHeight() / m_nThreadNum;
	for (int i = 0; i < m_nThreadNum; ++i)
	{
		m_pThreadParam[i].startIndex = i * subLength;
		m_pThreadParam[i].endIndex = i != m_nThreadNum - 1 ?
			(i + 1) * subLength - 1 : m_pImgSrc->GetWidth() * m_pImgSrc->GetHeight() - 1;
		m_pThreadParam[i].src = m_pImgSrc;
		AfxBeginThread((AFX_THREADPROC)&ImageProcess::videoView, &m_pThreadParam[i]);
	}
}

void CExperimentImgDlg::MedianFilter()
{
	//	AfxMessageBox(_T("中值滤波"));
	CComboBox* cmb_thread = ((CComboBox*)GetDlgItem(IDC_COMBO_THREAD));
	int thread = cmb_thread->GetCurSel();
	CButton* clb_circulation = ((CButton*)GetDlgItem(IDC_CHECK_CIRCULATION));
	int circulation = clb_circulation->GetCheck() == 0 ? 1 : 4;

	startTime = CTime::GetTickCount();
	m_nThreadNum;
	switch (thread)
	{
	case 0://win多线程
	{
		MedianFilter_WIN();
	}

	break;

	case 1://openmp
	{
		int subLength = m_pImgSrc->GetWidth() * m_pImgSrc->GetHeight() / m_nThreadNum;

#pragma omp parallel for num_threads(m_nThreadNum)
		for (int i = 0; i < m_nThreadNum; ++i)
		{
			m_pThreadParam[i].startIndex = i * subLength;
			m_pThreadParam[i].endIndex = i != m_nThreadNum - 1 ?
				(i + 1) * subLength - 1 : m_pImgSrc->GetWidth() * m_pImgSrc->GetHeight() - 1;
			m_pThreadParam[i].maxSpan = MAX_SPAN;
			m_pThreadParam[i].src = m_pImgSrc;
			ImageProcess::medianFilter(&m_pThreadParam[i]);
		}
	}

	break;

	case 2://cuda
	{
		byte* pRealData = (byte*)m_pImgSrc->GetBits();
		int pit = m_pImgSrc->GetPitch();	//line offset 
		int bitCount = m_pImgSrc->GetBPP() / 8;
		int height = m_pImgSrc->GetHeight();
		int width = m_pImgSrc->GetWidth();
		int length = height * width;
		int *pixel = (int*)malloc(length * sizeof(int));
		int *pixelR = (int*)malloc(length * sizeof(int));
		int *pixelG = (int*)malloc(length * sizeof(int));
		int *pixelB = (int*)malloc(length * sizeof(int));
		int *pixelIndex = (int*)malloc(length * sizeof(int));
		int index = 0;

		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width; x++)
			{
				if (bitCount == 1)
				{
					pixel[index] = *(pRealData + pit * y + x * bitCount);
					index++;
				}
				else
				{
					pixelR[index] = *(pRealData + pit * y + x * bitCount + 2);
					pixelG[index] = *(pRealData + pit * y + x * bitCount + 1);
					pixelB[index] = *(pRealData + pit * y + x * bitCount);
					pixel[index++] = int(pixelB[index] * 0.299 + 0.587*pixelG[index] + pixelR[index] * 0.144);
				}
			}
		}
		if (bitCount == 1)
		{
			MedianFilter_host(pixel, width, height);
			index = 0;
			for (int y = 0; y < height; y++)
			{
				for (int x = 0; x < width; x++)
				{
					*(pRealData + pit*y + x*bitCount) = pixel[index];
					index++;
				}
			}
		}
		else
		{
			MedianFilter_host(pixelR, width, height);
			MedianFilter_host(pixelG, width, height);
			MedianFilter_host(pixelB, width, height);
			index = 0;
			for (int y = 0; y < height; y++)
			{
				for (int x = 0; x < width; x++)
				{			
					*(pRealData + pit*y + x*bitCount + 2) = pixelR[index];
					*(pRealData + pit*y + x*bitCount + 1) = pixelG[index];
					*(pRealData + pit*y + x*bitCount) = pixelB[index];
					index++;
				}
			}
		}
		AfxMessageBox(_T("finish!"));
	}
	break;

	case 3: //opencl
	{
		unsigned char* pRealData = (unsigned char*)m_pImgSrc->GetBits();
		int pit = m_pImgSrc->GetPitch();	//line offset 
		int bitCount = m_pImgSrc->GetBPP() / 8;
		int height = m_pImgSrc->GetHeight();
		int width = m_pImgSrc->GetWidth();
		int length = height * width;
		int *pixel = (int*)malloc(length * sizeof(int));
		int *pixelR = (int*)malloc(length * sizeof(int));
		int *pixelG = (int*)malloc(length * sizeof(int));
		int *pixelB = (int*)malloc(length * sizeof(int));
		int *pixelIndex = (int*)malloc(length * sizeof(int));
		int index = 0;

		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width; x++)
			{
				if (bitCount == 1)
				{
					pixel[index] = *(pRealData + pit * y + x * bitCount);
					index++;
				}
				else
				{
					pixelR[index] = *(pRealData + pit * y + x * bitCount + 2);
					pixelG[index] = *(pRealData + pit * y + x * bitCount + 1);
					pixelB[index] = *(pRealData + pit * y + x * bitCount);
					pixel[index] = int(pixelB[index] * 0.299 + 0.587*pixelG[index] + pixelR[index] * 0.144);
					index++;
				}
			}
		}

		if (bitCount == 1)
		{
			MedianFilter_CL(pixel, pixelIndex, width, height);
			index = 0;
			for (int y = 0; y < height; y++)
			{
				for (int x = 0; x < width; x++)
				{
					*(pRealData + pit*y + x*bitCount) = pixel[index];
					index++;
				}
			}
		}
		else
		{
			MedianFilter_CL(pixelR, pixelIndex, width, height);
			MedianFilter_CL(pixelG, pixelIndex, width, height);
			MedianFilter_CL(pixelB, pixelIndex, width, height);
			index = 0;
			for (int y = 0; y < height; y++)
			{
				for (int x = 0; x < width; x++)
				{
					*(pRealData + pit*y + x*bitCount + 2) = pixelR[index];
					*(pRealData + pit*y + x*bitCount + 1) = pixelG[index];
					*(pRealData + pit*y + x*bitCount) = pixelB[index];
					index++;
				}
			}
		}
		AfxMessageBox(_T("Finish!"));
	}
		break;
	}
}

void CExperimentImgDlg::MedianFilter_CL(int *pixel, int *pixelIndex, int width, int height)
{
	//step 1:get platform;
	cl_int ret;														//errcode;
	cl_uint num_platforms;											//用于保存获取到的platforms数量;
	ret = clGetPlatformIDs(0, NULL, &num_platforms);
	if ((CL_SUCCESS != ret) || (num_platforms < 1))
	{
		CString errMsg;
		errMsg.Format(_T("Error getting platform number : %d"), ret);
		AfxMessageBox(errMsg);
		return;
	}
	cl_platform_id platform_id = NULL;
	ret = clGetPlatformIDs(1, &platform_id, NULL);					//获取第一个platform的id;
	if (CL_SUCCESS != ret)
	{
		CString errMsg;
		errMsg.Format(_T("Error getting platform id:%d "), ret);
		AfxMessageBox(errMsg);
		return;
	}

	//step 2:get device ;
	cl_uint num_devices;
	ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 0, NULL, &num_devices);
	if ((CL_SUCCESS != ret) || (num_devices < 1))
	{
		CString errMsg;
		errMsg.Format(_T("Error getting GPU device number:%d "), ret);
		AfxMessageBox(errMsg);
		return;
	}
	cl_device_id device_id = NULL;
	ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 1, &device_id, NULL);
	if (CL_SUCCESS != ret)
	{
		CString errMsg;
		errMsg.Format(_T("Error getting GPU device id: %d "), ret);
		AfxMessageBox(errMsg);
		return;
	}

	//step 3:create context;
	cl_context_properties props[] =
	{
		CL_CONTEXT_PLATFORM, (cl_context_properties)platform_id, 0
	};
	cl_context context = NULL;
	context = clCreateContext(props, 1, &device_id, NULL, NULL, &ret);
	if ((CL_SUCCESS != ret) || (NULL == context))
	{
		CString errMsg;
		errMsg.Format(_T("Error createing context: %d "), ret);
		AfxMessageBox(errMsg);
		return;
	}

	//step 4:create command queue;						//一个device有多个queue，queue之间并行执行
	cl_command_queue command_queue = NULL;
	command_queue = clCreateCommandQueue(context, device_id, 0, &ret);
	if ((CL_SUCCESS != ret) || (NULL == command_queue))
	{
		CString errMsg;
		errMsg.Format(_T("Error createing command queue: %d "), ret);
		AfxMessageBox(errMsg);
		return;
	}

	//step 5:create memory object;						//缓存类型（buffer），图像类型（iamge）

	cl_mem mem_obj = NULL;
	cl_mem mem_objout = NULL;

	//create opencl memory object using host ptr
//	mem_obj = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, BUF_SIZE, host_buffer, &ret);
	mem_obj = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, width * height * sizeof(int), pixel, &ret);
	mem_objout = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, width * height * sizeof(int), pixelIndex, &ret);
	if ((CL_SUCCESS != ret) || (NULL == mem_obj))
	{
		CString errMsg;
		errMsg.Format(_T("Error creating memory object: %d "), ret);
		AfxMessageBox(errMsg);
		return;
	}

	//step 6:create program;
	size_t szKernelLength = 0;
//	const char* oclSourceFile = "add_vector.cl";
	const char* oclSourceFile = "medianFilter.cl";
	const char* kernelSource = LoadProgSource(oclSourceFile, "", &szKernelLength);
	if (kernelSource == NULL)
	{
		CString errMsg;
		errMsg.Format(_T("Error loading source file: %d "), ret);
		AfxMessageBox(errMsg);
		return;
	}

	//create program
	cl_program program = NULL;
	program = clCreateProgramWithSource(context, 1, (const char **)&kernelSource, NULL, &ret);
	if ((CL_SUCCESS != ret) || (NULL == program))
	{
		CString errMsg;
		errMsg.Format(_T("Error creating program: %d "), ret);
		AfxMessageBox(errMsg);
		return;
	}

	//build program 
	ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
	if (CL_SUCCESS != ret)
	{
		size_t len;
		char buffer[8 * 1024];
		clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
		CString errMsg = (CString)buffer;
		AfxMessageBox(errMsg);
		return;
	}

	//step 7:create kernel;
	cl_kernel kernel = NULL;
//	kernel = clCreateKernel(program, "test", &ret);
	kernel = clCreateKernel(program, "medianFilterCL", &ret);
	if ((CL_SUCCESS != ret) || (NULL == kernel))
	{
		CString errMsg;
		errMsg.Format(_T("Error creating kernel: %d "), ret);
		AfxMessageBox(errMsg);
		return;
	}

	//step 8:set kernel arguement;
	ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void*)& mem_obj);
	if (CL_SUCCESS != ret)
	{
		CString errMsg;
		errMsg.Format(_T("Error setting kernel arguement 0: %d "), ret);
		AfxMessageBox(errMsg);
		return;
	}
	ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void*)& mem_objout);
	if (CL_SUCCESS != ret)
	{
		CString errMsg;
		errMsg.Format(_T("Error setting kernel arguement 1: %d "), ret);
		AfxMessageBox(errMsg);
		return;
	}
	ret = clSetKernelArg(kernel, 2, sizeof(int), (void*)& width);
	if (CL_SUCCESS != ret)
	{
		CString errMsg;
		errMsg.Format(_T("Error setting kernel arguement 2: %d "), ret);
		AfxMessageBox(errMsg);
		return;
	}
	ret = clSetKernelArg(kernel, 3, sizeof(int), (void*)& height);
	if (CL_SUCCESS != ret)
	{
		CString errMsg;
		errMsg.Format(_T("Error setting kernel arguement 3: %d "), ret);
		AfxMessageBox(errMsg);
		return;
	}
	//step 9:set work group size;  							//<---->dimBlock\dimGrid
	cl_uint work_dim = 2;
	size_t local_work_size[2] = { 32, 32 };
	size_t global_work_size[2] = { RoundUp(local_work_size[0], width),
		RoundUp(local_work_size[1], height) };		//let opencl device determine how to break work items into work groups;

	//step 10:run kernel;				//put kernel and work-item arugement into queue and excute;
	ret = clEnqueueNDRangeKernel(command_queue, kernel, work_dim, NULL, global_work_size, local_work_size, 0, NULL, NULL);
	if (CL_SUCCESS != ret)
	{
		CString errMsg;
		errMsg.Format(_T("Error enqueue NDRange: %d "), ret);
		AfxMessageBox(errMsg);
		return;
	}

	//step 11:get result;
	int *device_buffer = (int *)clEnqueueMapBuffer(command_queue, mem_objout, CL_TRUE, CL_MAP_READ | CL_MAP_WRITE, 0, width * height * sizeof(int), 0, NULL, NULL, &ret);
	if ((CL_SUCCESS != ret) || (NULL == device_buffer))
	{
		CString errMsg;
		errMsg.Format(_T("Error map buffer: %d "), ret);
		AfxMessageBox(errMsg);
		return;
	}

	memcpy(pixel, device_buffer, width * height * sizeof(int));

	//step 12:release all resource;
	if (NULL != kernel)
		clReleaseKernel(kernel);
	if (NULL != program)
		clReleaseProgram(program);
	if (NULL != mem_obj)
		clReleaseMemObject(mem_obj);
	if (NULL != command_queue)
		clReleaseCommandQueue(command_queue);
	if (NULL != context)
		clReleaseContext(context);
//	if (NULL != host_buffer)
//		free(host_buffer);
}

void CExperimentImgDlg::MedianFilter_WIN()
{
	int subLength = m_pImgSrc->GetWidth() * m_pImgSrc->GetHeight() / m_nThreadNum;
	int h = m_pImgSrc->GetHeight() / m_nThreadNum;
	int w = m_pImgSrc->GetWidth() / m_nThreadNum;
	for (int i = 0; i < m_nThreadNum; ++i)
	{
		m_pThreadParam[i].startIndex = i * subLength;
		m_pThreadParam[i].endIndex = i != m_nThreadNum - 1 ?
			(i + 1) * subLength - 1 : m_pImgSrc->GetWidth() * m_pImgSrc->GetHeight() - 1;
		m_pThreadParam[i].maxSpan = MAX_SPAN;
		m_pThreadParam[i].src = m_pImgSrc;
		AfxBeginThread((AFX_THREADPROC)&ImageProcess::medianFilter, &m_pThreadParam[i]);
	}
}

LRESULT CExperimentImgDlg::OnMedianFilterThreadMsgReceived(WPARAM wParam, LPARAM lParam)
{
	static int tempThreadCount = 0;
	static int tempProcessCount = 0;
	CButton* clb_circulation = ((CButton*)GetDlgItem(IDC_CHECK_CIRCULATION));
	int circulation = clb_circulation->GetCheck() == 0 ? 1 : 100;
	if ((int)wParam == 1)
	{
		// 当所有线程都返回了值1代表全部结束~显示时间
		if (m_nThreadNum == ++tempThreadCount)
		{
			CTime endTime = CTime::GetTickCount();
			CString timeStr;
			timeStr.Format(_T("耗时:%dms"), endTime - startTime);
			tempThreadCount = 0;
			tempProcessCount++;
			if (tempProcessCount < circulation)
				MedianFilter_WIN();
			else
			{
				tempProcessCount = 0;
				CTime endTime = CTime::GetTickCount();
				CString timeStr;
				timeStr.Format(_T("处理%d次,耗时:%dms"), circulation, endTime - startTime);
				AfxMessageBox(timeStr);
			}
			// 显示消息窗口
//			AfxMessageBox(timeStr);
		}
	}
	return 0;
}

LRESULT CExperimentImgDlg::OnNoiseThreadMsgReceived(WPARAM wParam, LPARAM lParam)
{
	static int tempCount = 0;
	static int tempProcessCount = 0;
	CButton* clb_circulation = ((CButton*)GetDlgItem(IDC_CHECK_CIRCULATION));
	int circulation = clb_circulation->GetCheck() == 0 ? 1 : 100;
	if ((int)wParam == 1)
		tempCount++;
	if (m_nThreadNum == tempCount)
	{
		//CTime endTime = CTime::GetTickCount();
		//CString timeStr;
		//timeStr.Format(_T("耗时:%dms", endTime - startTime));
		tempCount = 0;
		tempProcessCount++;
		if (tempProcessCount < circulation)
			AddNoise_WIN();
		else
		{
			tempProcessCount = 0;
			CTime endTime = CTime::GetTickCount();
			CString timeStr;
			timeStr.Format(_T("处理%d次,耗时:%dms"), circulation, endTime - startTime);
			AfxMessageBox(timeStr);
		}
	//	AfxMessageBox(timeStr);
	}
	return 0;
}

LRESULT CExperimentImgDlg::OnVideoViewThreadMsgReceived(WPARAM wParam, LPARAM lParam)
{
	static int tempCount = 0;
	static int tempProcessCount = 0;
	CButton* clb_circulation = ((CButton*)GetDlgItem(IDC_CHECK_CIRCULATION));
	int circulation = clb_circulation->GetCheck() == 0 ? 1 : 100;
	if ((int)wParam == 1)
		tempCount++;
	if (m_nThreadNum == tempCount)
	{
		//CTime endTime = CTime::GetTickCount();
		//CString timeStr;
		//timeStr.Format(_T("耗时:%dms", endTime - startTime));
		tempCount = 0;
		tempProcessCount++;
		if (tempProcessCount < circulation)
			VideoView_WIN();
		else
		{
			tempProcessCount = 0;
			CTime endTime = CTime::GetTickCount();
			CString timeStr;
			timeStr.Format(_T("处理%d次,耗时:%dms"), circulation, endTime - startTime);
			AfxMessageBox(timeStr);
		}
		//	AfxMessageBox(timeStr);
	}
	return 0;
}

char* CExperimentImgDlg::LoadProgSource(const char* cFilename, const char* cPreamble, size_t* szFinalLength)
{
	FILE* pFileStream = NULL;
	size_t szSourceLength;

	// open the OpenCL source code file  
	pFileStream = fopen(cFilename, "rb");
	if (pFileStream == NULL)
	{
		return NULL;
	}

	size_t szPreambleLength = strlen(cPreamble);

	// get the length of the source code  
	fseek(pFileStream, 0, SEEK_END);
	szSourceLength = ftell(pFileStream);
	fseek(pFileStream, 0, SEEK_SET);

	// allocate a buffer for the source code string and read it in  
	char* cSourceString = (char *)malloc(szSourceLength + szPreambleLength + 1);
	memcpy(cSourceString, cPreamble, szPreambleLength);
	if (fread((cSourceString)+szPreambleLength, szSourceLength, 1, pFileStream) != 1)
	{
		fclose(pFileStream);
		free(cSourceString);
		return 0;
	}

	// close the file and return the total length of the combined (preamble + source) string  
	fclose(pFileStream);
	if (szFinalLength != 0)
	{
		*szFinalLength = szSourceLength + szPreambleLength;
	}
	cSourceString[szSourceLength + szPreambleLength] = '\0';

	return cSourceString;
}

void CExperimentImgDlg::init_buf(int *buf, int len)
{
	int i;
	for (int i = 0; i<len; i++)
	{
		buf[i] = i + 1;
	}
}

size_t CExperimentImgDlg::RoundUp(int groupSize, int globalSize)
{
	int r = globalSize % groupSize;
	if (r == 0)
	{
		return globalSize;
	}
	else
	{
		return globalSize + groupSize - r;
	}
}

void CExperimentImgDlg::OnStnClickedPicture1()
{
	// TODO: 在此添加控件通知处理程序代码
}

int CExperimentImgDlg::Mat2CImage(cv::Mat *mat, CImage &img) {
	if (!mat || mat->empty())
		return -1;
	int nBPP = mat->channels() * 8;
	img.Create(mat->cols, mat->rows, nBPP);
	if (nBPP == 8)
	{
		static RGBQUAD pRGB[256];
		for (int i = 0; i < 256; i++)
			pRGB[i].rgbBlue = pRGB[i].rgbGreen = pRGB[i].rgbRed = i;
		img.SetColorTable(0, 256, pRGB);
	}
	uchar* psrc = mat->data;
	uchar* pdst = (uchar*)img.GetBits();
	int imgPitch = img.GetPitch();
	for (int y = 0; y < mat->rows; y++)
	{
		memcpy(pdst, psrc, mat->cols*mat->channels());//mat->step is incorrect for those images created by roi (sub-images!)
		psrc += mat->step;
		pdst += imgPitch;
	}
	return 0;

	//if (mat.channels() == 1)
	//{
	//	//调整图片与显示控件大小一致
	//	RECT rect;
	//	GetClientRect(hWndDisplay, &rect);
	//	cv::Mat imgShow(abs(rect.top - rect.bottom), abs(rect.right - rect.left), CV_8UC1);
	//	resize(mat, imgShow, imgShow.size());

	//	//ATL::CImage CI;
	//	int w = imgShow.cols;//宽  
	//	int h = imgShow.rows;//高  
	//	int channels = imgShow.channels();//通道数  
	//	CI.Destroy();//防止重复创建导致程序崩溃
	//	CI.Create(w, h, 8 * channels);
	//	RGBQUAD* ColorTable;
	//	int MaxColors = 256;
	//	ColorTable = new RGBQUAD[MaxColors];
	//	CI.GetColorTable(0, MaxColors, ColorTable);//这里是取得指针  
	//	for (int i = 0; i < MaxColors; i++)
	//	{
	//		ColorTable->rgbBlue = (BYTE)i;
	//		ColorTable->rgbGreen = (BYTE)i;
	//		ColorTable->rgbRed = (BYTE)i;
	//	}
	//	CI.SetColorTable(0, MaxColors, ColorTable);
	//	delete[] ColorTable;

	//	uchar *pS;
	//	uchar *pImg = (uchar*)CI.GetBits();
	//	int step = CI.GetPitch();
	//	for (int i = 0; i < h; i++)
	//	{
	//		pS = mat.ptr<uchar>(i);
	//		for (int j = 0; j < w; j++)
	//		{
	//			*(pImg + i * step + j) = pS[j];
	//		}
	//	}
	//	HDC dc = GetDC(hWndDisplay);
	//	CI.Draw(dc, 0, 0);

	//	ReleaseDC(hWndDisplay, dc);
	//	CI.Destroy();
	//}
	//if (mat.channels() == 3)
	//{
	//	//调整图片与显示控件大小一致
	//	RECT rect;
	//	GetClientRect(hWndDisplay, &rect);
	//	cv::Mat imgShow(abs(rect.top - rect.bottom), abs(rect.right - rect.left), CV_8UC3);
	//	resize(mat, imgShow, imgShow.size());

	//	//ATL::CImage CI;
	//	int w = imgShow.cols;//宽  
	//	int h = imgShow.rows;//高  
	//	int channels = imgShow.channels();//通道数  
	//	CI.Destroy();//防止重复创建导致程序崩溃
	//	CI.Create(w, h, 8 * channels);

	//	uchar *pS;//图片数据类型：uchar； value range：0~255
	//	uchar *pImg = (uchar *)CI.GetBits();//得到CImage数据区地址 
	//	int step = CI.GetPitch();
	//	for (int i = 0; i < h; i++)
	//	{
	//		pS = imgShow.ptr<uchar>(i);
	//		for (int j = 0; j < w; j++)
	//		{
	//			for (int k = 0; k < 3; k++)
	//				*(pImg + i * step + j * 3 + k) = pS[j * 3 + k];
	//		}
	//	}
	//	HDC dc = GetDC(hWndDisplay);
	//	CI.Draw(dc, 0, 0);

	//	ReleaseDC(hWndDisplay, dc);
	//	CI.Destroy();
	//}
}

void CExperimentImgDlg::DrawMatToPic(cv::Mat cvImg, UINT ID)
{

	CDC *pDC = GetDlgItem(ID)->GetDC();
	HDC hDCDst = pDC->GetSafeHdc();
	CRect rect;
	GetDlgItem(ID)->GetClientRect(&rect);

	int NewWidth = cvImg.cols;
	int NewHeight = cvImg.rows;
	double XScale = double(rect.right) / double(cvImg.cols);
	double YScale = double(rect.bottom) / double(cvImg.rows);

	if (XScale > YScale)
	{
		NewWidth = cvImg.cols*YScale;
		NewHeight = rect.bottom;
	}
	else if (XScale < YScale)
	{
		NewWidth = rect.right;
		NewHeight = cvImg.rows*XScale;
	}
	cv::Size NewSize(NewWidth, NewHeight);
	cv::Mat cvImgScaled(NewSize, CV_8UC3);
	resize(cvImg, cvImgScaled, NewSize);
	imshow("view", cvImgScaled);
}

void CExperimentImgDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	// TODO: 在此处添加实现代码.
	//if (1 == nIDEvent)
	switch (nIDEvent) 
	{
	case 1:
	{
		cv::Mat matImg;
		capture >> matImg;//取出一帧图像
		if (matImg.empty())
		{
			KillTimer(1);
			MessageBox(_T("视频结束"), _T("提示"));
			capture.release();
		}
		else
		{
			CImage img;
			RECT rt;
			DrawMatToPic(matImg, IDC_PICTURE1);
			//Mat2CImage(&matImg, img);
			/*GetClientRect(&rt);
			rt.bottom -= 40;
			img.Draw(this->GetDC()->GetSafeHdc(), rt);*/

			//cv::namedWindow("test", cv::WINDOW_AUTOSIZE);
			//HWND hWnd = static_cast<HWND>(cvGetWindowHandle("test"));
			//if (!hWnd)
			//{
			//	return;
			//}
			//HWND hParent = ::GetParent(hWnd);			// 等同于FindWindow找到的真正的对话框句柄
			//if (!hParent)
			//{
			//	return;
			//}

			//HWND hNewParent = GetDlgItem(IDC_PICTURE1)->GetSafeHwnd();
			//if (!hNewParent)
			//{
			//	return;
			//}

			//HWND a = ::SetParent(hWnd, hNewParent);		// 将图像显示区域嵌入到PIC控件上，返回旧的父窗口句柄（hParent）

			//cv::imshow("test",matImg);					// OpenCV内部持有显示区域的句柄，亦即hWnd
			//::ShowWindow(hParent, SW_HIDE);				// 隐藏对话框
		}
	}
	}
	CDialogEx::OnTimer(nIDEvent);
}

void CExperimentImgDlg::OnBnClickedOpen()
{
	// TODO: 在此添加控件通知处理程序代码
	TCHAR szFilter[] = _T("*.avi|*.avi|*.rmvb|*.rmvb|*.mp4|*.mp4|*.wmv|*.wmv| All Files (*.*) |*.*||");
	CString filePath("");
	CFileDialog fileOpenDialog(TRUE, NULL, NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT|OFN_ALLOWMULTISELECT, szFilter);
	if (fileOpenDialog.DoModal() == IDOK)
	{
		VERIFY(filePath = fileOpenDialog.GetPathName());
		CString strFilePath(filePath);
		picpath = strFilePath.GetBuffer(0);
		//		mEditInfo.SetWindowTextW(strFilePath);	//在文本框中显示图像路径
		//capture.open(strStr);//随便指定一个视频所在路径
		//if (capture.isOpened()) {
		//}
		//else {
		//	AfxMessageBox(_T("打开失败"));
		//	return;
		//}
		GetDlgItem(IDC_PICTURE1)->GetClientRect(&rectang1);
		GetDlgItem(IDC_PICTURE)->GetClientRect(&rectang);
		this->VideoShow(m_horiScrollbar,rectang,rectang1,"view","view1", picpath);
		//CRect dst(rectang.left, rectang.top, rectang.right, rectang.bottom);
		//CString to string  使用这个方法记得字符集选用“使用多字节字符”，不然会报错 
		//SetTimer(1, (unsigned int)(1000 * 1.0 / capture.get(cv::CAP_PROP_FPS)), NULL);//按视频帧率设置定时器，所以 ON_WM_TIMER()的设置很重要，不然不会响应定时事件

		this->Invalidate();
	}
}


void CExperimentImgDlg::OnBnClickedPlay()
{
	// TODO: 在此添加控件通知处理程序代码
	if (this->z)
	{
		this->z = FALSE;
		GetDlgItem(IDC_PICTURE1)->GetClientRect(&rectang1);
		GetDlgItem(IDC_PICTURE)->GetClientRect(&rectang);
		this->VideoShow(m_horiScrollbar,rectang,rectang1,"view", "view1", picpath);
	}
	else
	{
		this->z = TRUE;
		GetDlgItem(IDC_PICTURE1)->GetClientRect(&rectang1);
		GetDlgItem(IDC_PICTURE)->GetClientRect(&rectang);
		this->VideoShow(m_horiScrollbar,rectang,rectang1,"view", "view1", picpath);
	}
}


void CExperimentImgDlg::OnBnClickedExit() //orb检测
{
	// TODO: 在此添加控件通知处理程序代码
	// 判断输入图像是否读取成功
	cvtColor(img11, img1, CV_BGR2GRAY);
	cvtColor(img22, img2, CV_BGR2GRAY);
	if (img1.empty() || img2.empty() || img1.channels() != 1 || img2.channels() != 1)
	{
		cout << "Input Image is nullptr or the image channels is not gray!" << endl;
		system("pause");
	}
	// ORB算法继承Feature2D基类
	//Ptr<ORB> orb = ORB::create();
	Ptr<ORB> orb = ORB::create(1000, 1.2, 8, 31, 0, 2, ORB::FAST_SCORE, 31, 20);
	// 调整精度，值越小点越少，越精准
	vector<KeyPoint> kpts1, kpts2;
	// 特征点检测算法...
	orb->detect(img1, kpts1);
	orb->detect(img2, kpts2);

	// 特征点描述算法...
	Mat desc1, desc2;
	//orb->compute(img1, kpts1, desc1);
	//orb->compute(img2, kpts2, desc2);

	bool SelectiveDescMethods = false;
	// 默认选择BRIEF描述符
	if (SelectiveDescMethods)
	{
		// ORB 算法中默认BRIEF描述符
		orb->compute(img1, kpts1, desc1);
		orb->compute(img2, kpts2, desc2);
	}
	else
	{
		//LDB描述子描述FAsT特征点 
		bool flag = true;
		LDB ldb(48);
		ldb.compute(img1, kpts1, desc1, flag);
		ldb.compute(img2, kpts2, desc2, flag);
	}

	// 粗精匹配数据存储结构
	vector<vector<DMatch>> matches;
	vector<DMatch> goodMatchKpts;
	// Keypoint Matching...
	DescriptorMatcher *pMatcher = new BFMatcher(NORM_HAMMING, false);
	pMatcher->knnMatch(desc1, desc2, matches, 2);
	// 欧式距离度量  阈值设置为0.8
	for (unsigned int i = 0; i < matches.size(); ++i)
	{
		if (matches[i][0].distance < 0.8*matches[i][1].distance)
		{
			goodMatchKpts.push_back(matches[i][0]);
		}
	}
	// 显示匹配点对
	Mat show_match;
	drawMatches(img1, kpts1, img2, kpts2, goodMatchKpts, show_match);

	// 显示输出
	ostringstream s_time;
	s_time << time;
	//imshow("view", show_match);
	//imshow("view1", show_match);
	imshow("ORB_Algorithms_" + s_time.str(), show_match);

	cout << "(kpts1: " << kpts1.size() << ") && (kpts2:" \
		<< kpts2.size() << ") = goodMatchesKpts: " << goodMatchKpts.size() << endl;

	waitKey(0);

	// RANSAC Geometric Verification
	if (goodMatchKpts.size() < 4)
	{
		cout << "The Match Kpts' Size is less than Four to estimate!" << endl;
		return ;
	}

	vector<Point2f> obj, scene;
	for (unsigned int i = 0; i < goodMatchKpts.size(); ++i)
	{
		obj.push_back(kpts1[goodMatchKpts[i].queryIdx].pt);
		scene.push_back(kpts2[goodMatchKpts[i].trainIdx].pt);
	}
	// 估计Two Views变换矩阵
	Mat H = findHomography(obj, scene, CV_RANSAC);
	vector<Point2f> obj_corners(4), scene_corners(4);
	obj_corners[0] = cvPoint(0, 0); obj_corners[1] = cvPoint(img1.cols, 0);
	obj_corners[2] = cvPoint(img1.cols, img1.rows); obj_corners[3] = cvPoint(0, img1.rows);
	// 点集变换标出匹配重复区域
	perspectiveTransform(obj_corners, scene_corners, H);

	line(show_match, scene_corners[0] + Point2f(img1.cols, 0), scene_corners[1] + Point2f(img1.cols, 0), Scalar(0, 255, 0), 4);
	line(show_match, scene_corners[1] + Point2f(img1.cols, 0), scene_corners[2] + Point2f(img1.cols, 0), Scalar(0, 255, 0), 4);
	line(show_match, scene_corners[2] + Point2f(img1.cols, 0), scene_corners[3] + Point2f(img1.cols, 0), Scalar(0, 255, 0), 4);
	line(show_match, scene_corners[3] + Point2f(img1.cols, 0), scene_corners[0] + Point2f(img1.cols, 0), Scalar(0, 255, 0), 4);

	//imshow("view1", show_match);
	imshow("Match End", show_match);
	imwrite("img_boat15.jpg", show_match);
	waitKey(0);
	system("pause");
	return;
}

//int CExperimentImgDlg::OpenVideo(CString FilePath, HDC hdc, CRect rect)
//{
//
//	CvCapture *capture;
//	capture = cvCaptureFromAVI(FilePath);      //捕获视频
//	if (!capture)
//	{
//		AfxMessageBox(_T("打开文件失败！"));
//		return 0;
//	}
//	m_rect = rect;                       //获得picture控件的RECT    
//	m_hdc = hdc;                         //获得picture控件的画图句柄
//	m_filepath = FilePath;               //获得视频文件路径
//	m_pcapture = capture;
//	m_totalframes = (int)cvGetCaptureProperty(m_pcapture, CV_CAP_PROP_FRAME_COUNT);  //获取视频帧总数
//	if (m_totalframes == 0)
//	{
//		AfxMessageBox(_T("不能打开该格式文件！"));
//		return 0;
//	}
//	m_fps = (int)cvGetCaptureProperty(m_pcapture, CV_CAP_PROP_FPS);   //帧率
//	return 1;
//
//}
//
//void CExperimentImgDlg::CloseFile()
//{
//
//	if (m_pcapture)
//	{
//		cvReleaseCapture(&m_pcapture);
//	}
//
//}
//
//void CExperimentImgDlg::PlayVideo(int frame_pos)
//{
//
//	if (frame_pos > m_totalframes)
//	{
//		return;
//	}
//	m_framepos = frame_pos;                      //播放帧开始位置
//	cvSetCaptureProperty(m_pcapture, CV_CAP_PROP_POS_FRAMES, (m_framepos%m_totalframes)); //设置播放帧位置
//
//	m_pframe = cvQueryFrame(m_pcapture);         //获取一帧
//	if (m_pframe)
//	{
//		m_showimage.CopyOf(m_pframe, 1);
//		m_showimage.DrawToHDC(m_hdc, &m_rect);   //显示到控件
//
//		m_showimage.DrawToHDC(m_hdc, &m_rect);   //显示到控件
//	}
//
//}
//
//int CExperimentImgDlg::GetPlaySpeed()
//{
//
//	if (m_fps > 0 && m_fps < 1000)
//	{
//		m_speed = (int)(1000 / m_fps);        //每一帧多少毫秒
//		return m_speed;
//	}
//	return 0;
//
//}

void CExperimentImgDlg::OnNMThemeChangedHoriScrollbar(NMHDR *pNMHDR, LRESULT *pResult)
{
	// 该功能要求使用 Windows XP 或更高版本。
	// 符号 _WIN32_WINNT 必须 >= 0x0501。
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
}


void CExperimentImgDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	int TempPos = m_horiScrollbar.GetScrollPos(); // 获取水平滚动条当前位置
	switch (nSBCode)
	{
		// 如果向左滚动一列，则pos减1
	case SB_LINELEFT://点击左边的箭头
		if (TempPos > 1)
		{
			TempPos--;
			m_horiScrollbar.SetScrollPos(TempPos);
		}
		nowzhen = TempPos;
		break;
		// 如果向右滚动一列，则pos加1
	case SB_LINERIGHT://点击右边的箭头
		if (TempPos < zhen)
		{
			TempPos++;
			m_horiScrollbar.SetScrollPos(TempPos);
		}
		nowzhen = TempPos;
		break;
	//	// 如果向右滚动一页，则pos加10
	//case SB_PAGERIGHT:
	//	pos += 10;
	//	break;
		// 如果滚动到最左端，则pos为1
	/*case SB_LEFT:
		pos = 1;
		break;*/
		// 如果滚动到最右端，则pos为100
	/*case SB_RIGHT:
		pos = 100;
		break;*/
		// 如果拖动滚动块滚动到指定位置，则pos赋值为nPos的值
	case SB_THUMBPOSITION:
		m_horiScrollbar.SetScrollPos(nPos);
		nowzhen = nPos;
		break;
		// 下面的m_horiScrollbar.SetScrollPos(pos);执行时会第二次进入此函数，最终确定滚动块位置，并且会直接到default分支，所以在此处设置编辑框中显示数值
	case SB_PAGEUP://点击滚动条左方空白
		if (TempPos > 1)
		{
			TempPos = TempPos - zhen / 5;
			m_horiScrollbar.SetScrollPos(TempPos - zhen / 5, TRUE);
		}
		nowzhen = TempPos - zhen / 5;
		break;
	case SB_PAGEDOWN://点击滚动条右方空白
		if (TempPos < zhen)
		{
			TempPos = TempPos + zhen / 5;
			m_horiScrollbar.SetScrollPos(TempPos + zhen / 5, TRUE);
		}
		nowzhen = TempPos + zhen / 5;
		break;
	default:
		SetDlgItemInt(IDC_HSCROLL_EDIT, TempPos);
		return;
	}
	// 设置滚动块位置
	CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CExperimentImgDlg::VideoShow(CScrollBar &gdt, CRect rect,CRect rect1,std::string name,std::string name1, std::string picpath)
{
	cv::Mat frame;
	cv::Mat frame1;
	//ShiPin = imread(picpath);
	cv::VideoCapture capture(picpath);//读入视频
	zhen = capture.get(cv::CAP_PROP_FRAME_COUNT);
	gdt.SetScrollRange(1, zhen);//设置滚动条的初始值范围

	while (1)
	{
		if (nowzhen == zhen | nowzhen >= zhen)//循环播放
		{
			gdt.SetScrollPos(0);//初始化
			nowzhen = 0;
			this->z = TRUE;
		}
		char string[10];
		double t, fps;

		//设置帧率模块
		t = (double)cv::getTickCount();
		capture.set(cv::CAP_PROP_POS_FRAMES, nowzhen);//滚动条改变后设置新帧率
		capture >> frame;//读取当前帧
		capture.set(cv::CAP_PROP_POS_FRAMES, nowzhen+5.0);//滚动条改变后设置新帧率
		capture >> frame1;//从5帧后开始读取，用于视频的实时特征检测
		/*int j;
		for (size_t i = 0; i < capture.get(CAP_PROP_FRAME_COUNT); i++)
		{
			j = i;
			if (j>=20)
			{
				capture >> frame1;
			}
			else
			{
				continue;
			}
		}*/
		t = ((double)cv::getTickCount() - t) / cv::getTickFrequency();
		fps = 1.0 / t;
		sprintf_s(string, "%.2f", fps);
		std::string fpsString("FPS:");
		SetDlgItemInt(IDC_HSCROLL_EDIT, nowzhen);

		//若视频播放完成，退出循环
		if (this->z)//判断是否暂停
		{
			nowzhen++; //自增帧
			gdt.SetScrollPos(++nowzhen);//滑动条跟踪
			fpsString += string;//帧率运动
		}

		//resize(frame, img1, cv::Size(rect.Width(), rect.Height()));
		//resize(frame1, img2, cv::Size(rect.Width(), rect.Height()));

		int NewWidth = frame.cols;
		int NewHeight = frame.rows;
		double XScale = double(rect.right) / double(frame.cols);
		double YScale = double(rect.bottom) / double(frame.rows);

		if (XScale > YScale)
		{
			NewWidth = frame.cols*YScale;
			NewHeight = rect.bottom;
		}
		else if (XScale < YScale)
		{
			NewWidth = rect.right;
			NewHeight = frame.rows*XScale;
		}
		cv::Size NewSize(NewWidth, NewHeight);
		/*cv::Mat img11(NewSize, CV_8UC2);
		cv::Mat img22(NewSize, CV_8UC2);*/
		resize(frame, img11, NewSize);
		resize(frame1, img22, NewSize);
		putText(img11, fpsString, cv::Point(20, 20), cv::FONT_HERSHEY_PLAIN, 1, (255, 0, 0));
		putText(img22, fpsString, cv::Point(20, 20), cv::FONT_HERSHEY_PLAIN, 1, (255, 0, 0));
		imshow(name, img11);
		imshow(name1, img22);
		cv::waitKey(30);//延时30ms
	}
}

//滚动条
void CExperimentImgDlg::SCroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	
	// TODO: 在此添加消息处理程序代码和/或调用默认值
}

//暂停
void CExperimentImgDlg::ZanTing()
{
	
}

//快进
void  CExperimentImgDlg::KuaiJin(CScrollBar &gdt)
{
	
}

//快退
void  CExperimentImgDlg::KuaiTui(CScrollBar &gdt)
{
	if (nowzhen <= zhen / 5)//防止溢出
	{
		nowzhen = 0;
		gdt.SetScrollPos(nowzhen);
	}
	else
	{
		nowzhen -= zhen / 5;//快退
		gdt.SetScrollPos(nowzhen);
	}
}

