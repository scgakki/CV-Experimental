#include "stdafx.h"
#include "pch.h"
#include "MatCImage.h"

// 实现cv::Mat 结构到 CImage结构的转化  
void MatCImage::MatToCImage(Mat& mat, CImage& cimage)
{
	if (0 == mat.total())
	{
		return;
	}


	int nChannels = mat.channels();
	if ((1 != nChannels) && (3 != nChannels))
	{
		return;
	}
	int nWidth = mat.cols;
	int nHeight = mat.rows;


	//重建cimage  
	cimage.Destroy();  //这一步是防止重复利用造成内存问题
	cimage.Create(nWidth, nHeight, 8 * nChannels);  //默认图像像素单通道占用1个字节


													//拷贝数据  


	uchar* pucRow;                                  //指向数据区的行指针  
	uchar* pucImage = (uchar*)cimage.GetBits();     //指向数据区的指针  
	int nStep = cimage.GetPitch();                  //每行的字节数,注意这个返回值有正有负  

													// 如果是1个通道的图像(灰度图像) DIB格式才需要对调色板设置    
													// CImage中内置了调色板，我们要对他进行赋值：  
	if (1 == nChannels)                             //对于单通道的图像需要初始化调色板  
	{
		RGBQUAD* rgbquadColorTable;
		int nMaxColors = 256;
		rgbquadColorTable = new RGBQUAD[nMaxColors];
		//这里可以通过CI.GetMaxColorTableEntries()得到大小(如果你是CI.Load读入图像的话)    
		cimage.GetColorTable(0, nMaxColors, rgbquadColorTable);  //这里是取得指针  
		for (int nColor = 0; nColor < nMaxColors; nColor++)
		{
			//BYTE和uchar一回事，但MFC中都用它  
			rgbquadColorTable[nColor].rgbBlue = (uchar)nColor;  // (BYTE)nColor
			rgbquadColorTable[nColor].rgbGreen = (uchar)nColor;
			rgbquadColorTable[nColor].rgbRed = (uchar)nColor;
		}
		cimage.SetColorTable(0, nMaxColors, rgbquadColorTable);
		delete[]rgbquadColorTable;
	}


	for (int nRow = 0; nRow < nHeight; nRow++)
	{
		pucRow = (mat.ptr<uchar>(nRow));
		for (int nCol = 0; nCol < nWidth; nCol++)
		{
			if (1 == nChannels)
			{
				*(pucImage + nRow * nStep + nCol) = pucRow[nCol];
			}
			else if (3 == nChannels)
			{
				for (int nCha = 0; nCha < 3; nCha++)
				{
					*(pucImage + nRow * nStep + nCol * 3 + nCha) = pucRow[nCol * 3 + nCha];
				}
			}
		}
	}
}

void MatCImage::CImageToMat(CImage& cimage, Mat& mat)
{
	if (true == cimage.IsNull())
	{
		return;
	}


	int nChannels = cimage.GetBPP() / 8;
	if ((1 != nChannels) && (3 != nChannels))
	{
		return;
	}
	int nWidth = cimage.GetWidth();
	int nHeight = cimage.GetHeight();


	//重建mat  
	if (1 == nChannels)
	{
		mat.create(nHeight, nWidth, CV_8UC1);
	}
	else if (3 == nChannels)
	{
		mat.create(nHeight, nWidth, CV_8UC3);
	}


	//拷贝数据  


	uchar* pucRow;                                  //指向数据区的行指针  
	uchar* pucImage = (uchar*)cimage.GetBits();     //指向数据区的指针  
	int nStep = cimage.GetPitch();                  //每行的字节数,注意这个返回值有正有负  


	for (int nRow = 0; nRow < nHeight; nRow++)
	{
		pucRow = (mat.ptr<uchar>(nRow));
		for (int nCol = 0; nCol < nWidth; nCol++)
		{
			if (1 == nChannels)
			{
				pucRow[nCol] = *(pucImage + nRow * nStep + nCol);
			}
			else if (3 == nChannels)
			{
				for (int nCha = 0; nCha < 3; nCha++)
				{
					pucRow[nCol * 3 + nCha] = *(pucImage + nRow * nStep + nCol * 3 + nCha);
				}
			}
		}
	}
}
// VS默认工程是Unicode编码（宽字节），有时需要ANSI，即单字节，实现宽到单的转化  
string MatCImage::CString2StdString(const CString& cstr)
{
	CT2A str(cstr);
	return string(str.m_psz);
}

// 显示图像到指定窗口  
void MatCImage::DisplayImage(CWnd* m_pMyWnd, const CImage& image)
{

	CDC *m_pDC = m_pMyWnd->GetDC();//获取窗口所拥有的设备上下文，用于显示图像  
	m_pMyWnd->UpdateWindow();

	CRect rc;
	//    m_pMyWnd->GetWindowRect(&rc);  

	/*InvalidateRect(m_pMyWnd->m_hWnd,&rc,true);*/
	int nwidth = rc.Width();
	int nheight = rc.Height();

	int fixed_width = min(image.GetWidth(), nwidth);
	int fixed_height = min(image.GetHeight(), nheight);

	double ratio_w = fixed_width / (double)image.GetWidth();
	double ratio_h = fixed_height / (double)image.GetHeight();
	double ratio = min(ratio_w, ratio_h);

	int show_width = (int)(image.GetWidth() * ratio);
	int show_height = (int)(image.GetHeight() * ratio);

	int offsetx = (nwidth - show_width) / 2;
	int offsety = (nheight - show_height) / 2;

	::SetStretchBltMode(m_pDC->GetSafeHdc(), COLORONCOLOR);//设置位图的伸缩模式  
	image.StretchBlt(m_pDC->GetSafeHdc(), offsetx, offsety, show_width, show_height,
		0, 0, image.GetWidth(), image.GetHeight(), SRCCOPY);
}


void MatCImage::DisplayImageEx(CWnd* pWnd, const CImage& image)
{
	CDC *m_pDC = pWnd->GetDC();//获取窗口所拥有的设备上下文，用于显示图像  
	pWnd->UpdateWindow();

	CRect rc;
	//客户区大小  
	//CRect rc1;  
	pWnd->GetWindowRect(&rc);


	//ScreenToClient(&rc);  

	::SetStretchBltMode(m_pDC->GetSafeHdc(), COLORONCOLOR);//设置位图的伸缩模式  
	image.StretchBlt(m_pDC->GetSafeHdc(), 0, 0, rc.Width() - 1, rc.Height() - 1,
		0, 0, image.GetWidth(), image.GetHeight(), SRCCOPY);
}

// 格式转换，AWX云图转到可以显示的opencv支持的格式  
Mat MatCImage::AWX2Mat(CString filePath)
{
	CFile fp;
	Mat   mat;
	fp.Open(filePath, CFile::modeRead);
	ULONGLONG flength = fp.GetLength();

	if (2475700 == flength)
	{
		mat.create(1300, 1900, CV_8UC1);
	}
	else if (1444803 == flength)
	{
		mat.create(1201, 1201, CV_8UC1);
	}

	LONGLONG size = mat.rows * mat.cols;
	LONGLONG sizebuff = fp.Seek(-size, CFile::end);

	uchar *pSource = new uchar[size];
	fp.Read(pSource, size);
	fp.Close();

	for (int i = 0; i < mat.rows; i++)
	{
		uchar * ps = mat.ptr<uchar>(i);
		for (int j = 0; j < mat.cols; j++)
		{
			ps[j] = *(pSource + i * mat.cols + j);
		}
	}
	delete pSource;



	return mat;
}