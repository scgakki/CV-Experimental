
// ExperimentImgDlg.h : ͷ�ļ�
//

#pragma once
#include "afxwin.h"
#include "ImageProcess.h"


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
};
