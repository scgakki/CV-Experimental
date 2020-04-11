#pragma once
#pragma once

#include "pch.h"
#include "afxwin.h"
#include <opencv2/core/core.hpp>  
#include <opencv2/imgproc/imgproc.hpp>  
#include <opencv2/highgui/highgui.hpp>  

using namespace cv;
using namespace std;
class MatCImage
{
public:

	void MatToCImage(Mat& mat, CImage& cimage);

	void CImageToMat(CImage& cimage, Mat& mat);


	// VSĬ�Ϲ�����Unicode���루���ֽڣ�����ʱ��ҪANSI�������ֽڣ�ʵ�ֿ�����ת��  
	string CString2StdString(const CString& cstr);
	// ��ʾͼ��ָ������  
	void DisplayImage(CWnd* m_pMyWnd, const CImage& image);

	// ��ʽת����AWX��ͼת��������ʾ��opencv֧�ֵĸ�ʽ  
	Mat AWX2Mat(CString filePath);

	void DisplayImageEx(CWnd* pWnd, const CImage& image);
};