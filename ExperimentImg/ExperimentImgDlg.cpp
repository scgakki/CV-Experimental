
// ExperimentImgDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ExperimentImg.h"
#include "ExperimentImgDlg.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <CL/cl.h>
#include "afxdialogex.h"

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
//	m_pImgCpy = NULL;
	m_nThreadNum = 1;
	m_pThreadParam = new ThreadParam[MAX_THREAD];
	srand(time(0));
}

void CExperimentImgDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	//	DDX_Control(pDX, IDC_EDIT_INFO, mEditInfo);
	DDX_Control(pDX, IDC_PICTURE, mPictureControl);
	DDX_Control(pDX, IDC_CHECK_100, m_CheckCirculation);
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

	// TODO: 在此添加额外的初始化代码
//	mEditInfo.SetWindowTextW(CString("File Path"));
	CComboBox * cmb_function = ((CComboBox*)GetDlgItem(IDC_COMBO_FUNCTION));
	cmb_function->AddString(_T("椒盐噪声"));
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

			if (width <= rect.Width() && height <= rect.Width())
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
	case 1: //自适应中值滤波
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