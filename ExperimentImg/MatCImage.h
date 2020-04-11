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


	// VS默认工程是Unicode编码（宽字节），有时需要ANSI，即单字节，实现宽到单的转化  
	string CString2StdString(const CString& cstr);
	// 显示图像到指定窗口  
	void DisplayImage(CWnd* m_pMyWnd, const CImage& image);

	// 格式转换，AWX云图转到可以显示的opencv支持的格式  
	Mat AWX2Mat(CString filePath);

	void DisplayImageEx(CWnd* pWnd, const CImage& image);
};