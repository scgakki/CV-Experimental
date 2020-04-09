
// ExperimentImgDlg.h : ͷ�ļ�
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

// CExperimentImgDlg �Ի���
class CExperimentImgDlg : public CDialogEx
{
// ����
public:
	CExperimentImgDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_EXPERIMENTIMG_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��
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
// ʵ��
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
	// ���ɵ���Ϣӳ�亯��
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
	//int framepos;          //��¼������Ƶ֡λ��
	//bool IsPlaying;        //�ж��Ƿ����ڲ���
	//bool IsPausing;        //�ж��Ƿ���ͣ��
	//bool IsAviFile;        //�ж��Ƿ��ǲ���AVI��Ƶ�ļ�
	//CRect rect;
	//CWnd *pwnd;
	//CDC *pdc;
	//HDC hdc;
	//CvvImage m_showimage;        //CvvImage����������ʾ֡���ؼ�
	//IplImage *m_pframe;          //��ȡ֡
	//CvCapture *m_pcapture;       //����
	//CRect m_rect;                //�������� 
	//HDC m_hdc;                   //������ʾ֡ͼ��ľ��
	//CString m_filepath;          //�ļ�·��
	//bool m_isplay;               //�ж��Ƿ񲥷���
	//int m_framepos;              //֡λ��
	//int m_totalframes;           //֡����
	//int m_fps;
	//UINT m_timer;                //��ʱ��
	//int m_speed;                 //�����ٶ�
	CScrollBar m_horiScrollbar;
	afx_msg void OnNMThemeChangedHoriScrollbar(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	void VideoShow(CScrollBar &gdt, CRect rect,CRect rect1, std::string name,std::string name1, std::string picpath);//���ļ�չʾ����
	void SCroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);//�������ں���
	void ZanTing();//��ͣ���ź���
	void KuaiJin(CScrollBar &gdt);//��� ����������������
	void KuaiTui(CScrollBar &gdt);//���� ����������������
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
