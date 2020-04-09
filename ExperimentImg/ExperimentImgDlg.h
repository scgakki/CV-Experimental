
// ExperimentImgDlg.h : 头文件
//

#pragma once
#include "afxwin.h"
#include "ImageProcess.h"
#include <opencv2/opencv.hpp>

#ifndef CIRCLE_H
#define CIRCLE_H
#endif // !CIRCLE_H

#include<string>
#define MAX_THREAD 8
#define MAX_SPAN 15


struct DrawPara
{
	CImage* pImgSrc;
	CDC* pDC;
	int oriX;
	int oriY;
	int width;
	int height;
};

// CExperimentImgDlg 对话框
class CExperimentImgDlg : public CDialogEx
{
// 构造
public:
	CExperimentImgDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_EXPERIMENTIMG_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持
	CImage* getImage() { return m_pImgSrc; }
	void VideoView();
	void VideoView_WIN();
	void MedianFilter();
	void AddNoise();
	void AddNoise_WIN();
	void ThreadDraw(DrawPara *p);
	static UINT Update(void* p);
	static UINT UpdateCpy(void* p);
	void ImageCopy(CImage* pImgSrc, CImage* pImgDrt);
	void MedianFilter_WIN();
	void MedianFilter_CL(int *pixel, int *pixelIndex, int width, int height);
	void init_buf(int *buf, int len);
	size_t RoundUp(int groupSize, int globalSize);
	char * LoadProgSource(const char* cFilename, const char* cPreamble, size_t* szFinalLength);
	afx_msg LRESULT OnMedianFilterThreadMsgReceived(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnNoiseThreadMsgReceived(WPARAM wParam, LPARAM lParam); 
	afx_msg LRESULT OnVideoViewThreadMsgReceived(WPARAM wParam, LPARAM lParam);
// 实现
protected:
	HICON m_hIcon;
	HWND hWndDisplay;
	HWND hWndDisplay1;
	cv::VideoCapture capture;
	CImage * m_pImgSrc;
	CImage * m_pImgCpy;
	int m_nThreadNum;
	ThreadParam* m_pThreadParam;
	CTime startTime;

//	ThreadParam * m_pThreadParam;
	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonOpen();
	CEdit mEditInfo;
	CStatic mPictureControl;
	afx_msg void OnCbnSelchangeComboFunction();
	afx_msg void OnNMCustomdrawSliderThreadnum(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonProcess();
	CButton m_CheckCirculation;
	CStatic mPictureControl1;
	afx_msg void OnStnClickedPicture1();
	afx_msg void OnTimer(UINT_PTR nIDEvent);

	afx_msg void OnBnClickedOpen();
	afx_msg void OnBnClickedPlay();
	afx_msg void OnBnClickedExit();

	int Mat2CImage(cv::Mat *mat, CImage &img);
	void DrawMatToPic(cv::Mat cvImg, UINT ID);
	//void MatToCImage(cv::Mat& mat, CImage& cimage);
	//void CImageToMat(CImage& cimage, cv::Mat& mat);
	//int OpenVideo(CString FilePath,HDC hdc,CRect rect);
	//void CloseFile();
	//void PlayVideo(int frame_pos);
	//int GetPlaySpeed();
	//int framepos;          //记录播放视频帧位置
	//bool IsPlaying;        //判断是否正在播放
	//bool IsPausing;        //判断是否暂停中
	//bool IsAviFile;        //判断是否是播放AVI视频文件
	//CRect rect;
	//CWnd *pwnd;
	//CDC *pdc;
	//HDC hdc;
	//CvvImage m_showimage;        //CvvImage对象，用于显示帧到控件
	//IplImage *m_pframe;          //获取帧
	//CvCapture *m_pcapture;       //捕获
	//CRect m_rect;                //播放区域 
	//HDC m_hdc;                   //用于显示帧图像的句柄
	//CString m_filepath;          //文件路径
	//bool m_isplay;               //判断是否播放中
	//int m_framepos;              //帧位置
	//int m_totalframes;           //帧总数
	//int m_fps;
	//UINT m_timer;                //定时器
	//int m_speed;                 //播放速度
	CScrollBar m_horiScrollbar;
	afx_msg void OnNMThemeChangedHoriScrollbar(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	void VideoShow(CScrollBar &gdt, CRect rect,CRect rect1, std::string name,std::string name1, std::string picpath);//打开文件展示函数
	void SCroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);//滑动窗口函数
	void ZanTing();//暂停播放函数
	void KuaiJin(CScrollBar &gdt);//快进 参数：滚动条变量
	void KuaiTui(CScrollBar &gdt);//快退 参数：滚动条变量
	double zhen;
	double nowzhen = 0;
	bool z = TRUE;
	CRect rectang;
	CRect rectang1;
	cv::Mat img11;
	cv::Mat img22;
	cv::Mat img1;
	cv::Mat img2;
	std::string picpath;
};
