#include "stdafx.h"
#include "pch.h"
#include "MatCImage.h"

// ʵ��cv::Mat �ṹ�� CImage�ṹ��ת��  
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


	//�ؽ�cimage  
	cimage.Destroy();  //��һ���Ƿ�ֹ�ظ���������ڴ�����
	cimage.Create(nWidth, nHeight, 8 * nChannels);  //Ĭ��ͼ�����ص�ͨ��ռ��1���ֽ�


													//��������  


	uchar* pucRow;                                  //ָ������������ָ��  
	uchar* pucImage = (uchar*)cimage.GetBits();     //ָ����������ָ��  
	int nStep = cimage.GetPitch();                  //ÿ�е��ֽ���,ע���������ֵ�����и�  

													// �����1��ͨ����ͼ��(�Ҷ�ͼ��) DIB��ʽ����Ҫ�Ե�ɫ������    
													// CImage�������˵�ɫ�壬����Ҫ�������и�ֵ��  
	if (1 == nChannels)                             //���ڵ�ͨ����ͼ����Ҫ��ʼ����ɫ��  
	{
		RGBQUAD* rgbquadColorTable;
		int nMaxColors = 256;
		rgbquadColorTable = new RGBQUAD[nMaxColors];
		//�������ͨ��CI.GetMaxColorTableEntries()�õ���С(�������CI.Load����ͼ��Ļ�)    
		cimage.GetColorTable(0, nMaxColors, rgbquadColorTable);  //������ȡ��ָ��  
		for (int nColor = 0; nColor < nMaxColors; nColor++)
		{
			//BYTE��ucharһ���£���MFC�ж�����  
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


	//�ؽ�mat  
	if (1 == nChannels)
	{
		mat.create(nHeight, nWidth, CV_8UC1);
	}
	else if (3 == nChannels)
	{
		mat.create(nHeight, nWidth, CV_8UC3);
	}


	//��������  


	uchar* pucRow;                                  //ָ������������ָ��  
	uchar* pucImage = (uchar*)cimage.GetBits();     //ָ����������ָ��  
	int nStep = cimage.GetPitch();                  //ÿ�е��ֽ���,ע���������ֵ�����и�  


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
// VSĬ�Ϲ�����Unicode���루���ֽڣ�����ʱ��ҪANSI�������ֽڣ�ʵ�ֿ�����ת��  
string MatCImage::CString2StdString(const CString& cstr)
{
	CT2A str(cstr);
	return string(str.m_psz);
}

// ��ʾͼ��ָ������  
void MatCImage::DisplayImage(CWnd* m_pMyWnd, const CImage& image)
{

	CDC *m_pDC = m_pMyWnd->GetDC();//��ȡ������ӵ�е��豸�����ģ�������ʾͼ��  
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

	::SetStretchBltMode(m_pDC->GetSafeHdc(), COLORONCOLOR);//����λͼ������ģʽ  
	image.StretchBlt(m_pDC->GetSafeHdc(), offsetx, offsety, show_width, show_height,
		0, 0, image.GetWidth(), image.GetHeight(), SRCCOPY);
}


void MatCImage::DisplayImageEx(CWnd* pWnd, const CImage& image)
{
	CDC *m_pDC = pWnd->GetDC();//��ȡ������ӵ�е��豸�����ģ�������ʾͼ��  
	pWnd->UpdateWindow();

	CRect rc;
	//�ͻ�����С  
	//CRect rc1;  
	pWnd->GetWindowRect(&rc);


	//ScreenToClient(&rc);  

	::SetStretchBltMode(m_pDC->GetSafeHdc(), COLORONCOLOR);//����λͼ������ģʽ  
	image.StretchBlt(m_pDC->GetSafeHdc(), 0, 0, rc.Width() - 1, rc.Height() - 1,
		0, 0, image.GetWidth(), image.GetHeight(), SRCCOPY);
}

// ��ʽת����AWX��ͼת��������ʾ��opencv֧�ֵĸ�ʽ  
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