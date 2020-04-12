#include"stdafx.h"
#include "highgui/highgui.hpp"    
#include "opencv2/nonfree/nonfree.hpp"    
#include "opencv2/legacy/legacy.hpp"   
#include <iostream>
#include <opencv2/core.hpp>
#include"Image_stitching.h"
using namespace cv;
using namespace std;

void Image_stitching::surf_stitching_Flann(cv::String path1, cv::String path2)
{
	Mat image01 = imread(path1);
	Mat image02 = imread(path2);
	//imshow("拼接图像1", image01);
	//imshow("拼接图像2", image02);

	//灰度图转换
	Mat image1, image2;
	cvtColor(image01, image1, CV_RGB2GRAY);
	cvtColor(image02, image2, CV_RGB2GRAY);

	//提取特征点  
	SurfFeatureDetector surfDetector(800);  // 海塞矩阵阈值
	vector<KeyPoint> keyPoint1, keyPoint2;
	surfDetector.detect(image1, keyPoint1);
	surfDetector.detect(image2, keyPoint2);

	//特征点描述，为下边的特征点匹配做准备  
	SurfDescriptorExtractor surfDescriptor;
	Mat imageDesc1, imageDesc2;
	surfDescriptor.compute(image1, keyPoint1, imageDesc1);
	surfDescriptor.compute(image2, keyPoint2, imageDesc2);

	//获得匹配特征点，并提取最优配对  	
	FlannBasedMatcher matcher;
	vector<DMatch> matchePoints;
	matcher.match(imageDesc1, imageDesc2, matchePoints, Mat());
	sort(matchePoints.begin(), matchePoints.end()); //特征点排序	
													//获取排在前N个的最优匹配特征点
	vector<Point2f> imagePoints1, imagePoints2;
	for (int i = 0; i < 10; i++)
	{
		imagePoints1.push_back(keyPoint1[matchePoints[i].queryIdx].pt);
		imagePoints2.push_back(keyPoint2[matchePoints[i].trainIdx].pt);
	}

	//获取图像1到图像2的投影映射矩阵，尺寸为3*3
	Mat homo = findHomography(imagePoints1, imagePoints2, CV_RANSAC);
	Mat adjustMat = (Mat_<double>(3, 3) << 1.0, 0, image01.cols, 0, 1.0, 0, 0, 0, 1.0);
	Mat adjustHomo = adjustMat * homo;

	//获取最强配对点在原始图像和矩阵变换后图像上的对应位置，用于图像拼接点的定位
	Point2f originalLinkPoint, targetLinkPoint, basedImagePoint;
	originalLinkPoint = keyPoint1[matchePoints[0].queryIdx].pt;
	targetLinkPoint = getTransformPoint(originalLinkPoint, adjustHomo);
	basedImagePoint = keyPoint2[matchePoints[0].trainIdx].pt;

	//图像配准
	Mat imageTransform1;
	warpPerspective(image01, imageTransform1, adjustMat*homo, Size(image02.cols + image01.cols + 110, image02.rows));

	//在最强匹配点左侧的重叠区域进行累加，是衔接稳定过渡，消除突变
	Mat image1Overlap, image2Overlap; //图1和图2的重叠部分	
	image1Overlap = imageTransform1(Rect(Point(targetLinkPoint.x - basedImagePoint.x, 0), Point(targetLinkPoint.x, image02.rows)));
	image2Overlap = image02(Rect(0, 0, image1Overlap.cols, image1Overlap.rows));
	Mat image1ROICopy = image1Overlap.clone();  //复制一份图1的重叠部分
	for (int i = 0; i < image1Overlap.rows; i++)
	{
		for (int j = 0; j < image1Overlap.cols; j++)
		{
			double weight;
			weight = (double)j / image1Overlap.cols;  //随距离改变而改变的叠加系数
			image1Overlap.at<Vec3b>(i, j)[0] = (1 - weight)*image1ROICopy.at<Vec3b>(i, j)[0] + weight * image2Overlap.at<Vec3b>(i, j)[0];
			image1Overlap.at<Vec3b>(i, j)[1] = (1 - weight)*image1ROICopy.at<Vec3b>(i, j)[1] + weight * image2Overlap.at<Vec3b>(i, j)[1];
			image1Overlap.at<Vec3b>(i, j)[2] = (1 - weight)*image1ROICopy.at<Vec3b>(i, j)[2] + weight * image2Overlap.at<Vec3b>(i, j)[2];
		}
	}
	Mat ROIMat = image02(Rect(Point(image1Overlap.cols, 0), Point(image02.cols, image02.rows)));	 //图2中不重合的部分
	ROIMat.copyTo(Mat(imageTransform1, Rect(targetLinkPoint.x, 0, ROIMat.cols, image02.rows))); //不重合的部分直接衔接上去
	namedWindow("拼接结果", 0);
	imshow("拼接结果", imageTransform1);
	//imwrite("D:\\拼接结果.jpg", imageTransform1);
	waitKey();
}

void Image_stitching::sift_stitching_Flann(cv::String path1, cv::String path2)
{
	Mat image01 = imread(path1);
	Mat image02 = imread(path2);
	//imshow("拼接图像1", image01);
	//imshow("拼接图像2", image02);

	//灰度图转换
	Mat image1, image2;
	cvtColor(image01, image1, CV_RGB2GRAY);
	cvtColor(image02, image2, CV_RGB2GRAY);

	//提取特征点  
	SiftFeatureDetector siftDetector(800);  // 海塞矩阵阈值
	vector<KeyPoint> keyPoint1, keyPoint2;
	siftDetector.detect(image1, keyPoint1);
	siftDetector.detect(image2, keyPoint2);

	//特征点描述，为下边的特征点匹配做准备  
	SiftFeatureDetector siftDescriptor;
	Mat imageDesc1, imageDesc2;
	siftDescriptor.compute(image1, keyPoint1, imageDesc1);
	siftDescriptor.compute(image2, keyPoint2, imageDesc2);

	//获得匹配特征点，并提取最优配对  	
	FlannBasedMatcher matcher;
	vector<DMatch> matchePoints;
	matcher.match(imageDesc1, imageDesc2, matchePoints, Mat());
	sort(matchePoints.begin(), matchePoints.end()); //特征点排序	
													//获取排在前N个的最优匹配特征点
	vector<Point2f> imagePoints1, imagePoints2;
	for (int i = 0; i < 10; i++)
	{
		imagePoints1.push_back(keyPoint1[matchePoints[i].queryIdx].pt);
		imagePoints2.push_back(keyPoint2[matchePoints[i].trainIdx].pt);
	}

	//获取图像1到图像2的投影映射矩阵，尺寸为3*3
	Mat homo = findHomography(imagePoints1, imagePoints2, CV_RANSAC);
	Mat adjustMat = (Mat_<double>(3, 3) << 1.0, 0, image01.cols, 0, 1.0, 0, 0, 0, 1.0);
	Mat adjustHomo = adjustMat * homo;

	//获取最强配对点在原始图像和矩阵变换后图像上的对应位置，用于图像拼接点的定位
	Point2f originalLinkPoint, targetLinkPoint, basedImagePoint;
	originalLinkPoint = keyPoint1[matchePoints[0].queryIdx].pt;
	targetLinkPoint = getTransformPoint(originalLinkPoint, adjustHomo);
	basedImagePoint = keyPoint2[matchePoints[0].trainIdx].pt;

	//图像配准
	Mat imageTransform1;
	warpPerspective(image01, imageTransform1, adjustMat*homo, Size(image02.cols + image01.cols + 110, image02.rows));

	//在最强匹配点左侧的重叠区域进行累加，是衔接稳定过渡，消除突变
	Mat image1Overlap, image2Overlap; //图1和图2的重叠部分	
	image1Overlap = imageTransform1(Rect(Point(targetLinkPoint.x - basedImagePoint.x, 0), Point(targetLinkPoint.x, image02.rows)));
	image2Overlap = image02(Rect(0, 0, image1Overlap.cols, image1Overlap.rows));
	Mat image1ROICopy = image1Overlap.clone();  //复制一份图1的重叠部分
	for (int i = 0; i < image1Overlap.rows; i++)
	{
		for (int j = 0; j < image1Overlap.cols; j++)
		{
			double weight;
			weight = (double)j / image1Overlap.cols;  //随距离改变而改变的叠加系数
			image1Overlap.at<Vec3b>(i, j)[0] = (1 - weight)*image1ROICopy.at<Vec3b>(i, j)[0] + weight * image2Overlap.at<Vec3b>(i, j)[0];
			image1Overlap.at<Vec3b>(i, j)[1] = (1 - weight)*image1ROICopy.at<Vec3b>(i, j)[1] + weight * image2Overlap.at<Vec3b>(i, j)[1];
			image1Overlap.at<Vec3b>(i, j)[2] = (1 - weight)*image1ROICopy.at<Vec3b>(i, j)[2] + weight * image2Overlap.at<Vec3b>(i, j)[2];
		}
	}
	Mat ROIMat = image02(Rect(Point(image1Overlap.cols, 0), Point(image02.cols, image02.rows)));	 //图2中不重合的部分
	ROIMat.copyTo(Mat(imageTransform1, Rect(targetLinkPoint.x, 0, ROIMat.cols, image02.rows))); //不重合的部分直接衔接上去
	namedWindow("拼接结果", 0);
	imshow("拼接结果", imageTransform1);
	//imwrite("D:\\拼接结果.jpg", imageTransform1);
	waitKey();
}

void Image_stitching::sift_stitching_BruteForce(cv::String path1, cv::String path2) {
	Mat image01 = imread(path1);
	Mat image02 = imread(path2);
	//imshow("拼接图像1", image01);
	//imshow("拼接图像2", image02);

	//灰度图转换
	Mat image1, image2;
	cvtColor(image01, image1, CV_RGB2GRAY);
	cvtColor(image02, image2, CV_RGB2GRAY);

	//提取特征点  
	SiftFeatureDetector siftDetector(800);  // 海塞矩阵阈值
	vector<KeyPoint> keyPoint1, keyPoint2;
	siftDetector.detect(image1, keyPoint1);
	siftDetector.detect(image2, keyPoint2);

	//特征点描述，为下边的特征点匹配做准备  
	SiftFeatureDetector siftDescriptor;
	Mat imageDesc1, imageDesc2;
	siftDescriptor.compute(image1, keyPoint1, imageDesc1);
	siftDescriptor.compute(image2, keyPoint2, imageDesc2);

	//获得匹配特征点，并提取最优配对  	
	BFMatcher matcher;
	vector<DMatch> matchePoints;
	matcher.match(imageDesc1, imageDesc2, matchePoints, Mat());
	sort(matchePoints.begin(), matchePoints.end()); //特征点排序	
													//获取排在前N个的最优匹配特征点
	vector<Point2f> imagePoints1, imagePoints2;
	for (int i = 0; i < 10; i++)
	{
		imagePoints1.push_back(keyPoint1[matchePoints[i].queryIdx].pt);
		imagePoints2.push_back(keyPoint2[matchePoints[i].trainIdx].pt);
	}

	//获取图像1到图像2的投影映射矩阵，尺寸为3*3
	Mat homo = findHomography(imagePoints1, imagePoints2, CV_RANSAC);
	Mat adjustMat = (Mat_<double>(3, 3) << 1.0, 0, image01.cols, 0, 1.0, 0, 0, 0, 1.0);
	Mat adjustHomo = adjustMat * homo;

	//获取最强配对点在原始图像和矩阵变换后图像上的对应位置，用于图像拼接点的定位
	Point2f originalLinkPoint, targetLinkPoint, basedImagePoint;
	originalLinkPoint = keyPoint1[matchePoints[0].queryIdx].pt;
	targetLinkPoint = getTransformPoint(originalLinkPoint, adjustHomo);
	basedImagePoint = keyPoint2[matchePoints[0].trainIdx].pt;

	//图像配准
	Mat imageTransform1;
	warpPerspective(image01, imageTransform1, adjustMat*homo, Size(image02.cols + image01.cols + 110, image02.rows));

	//在最强匹配点左侧的重叠区域进行累加，是衔接稳定过渡，消除突变
	Mat image1Overlap, image2Overlap; //图1和图2的重叠部分	
	image1Overlap = imageTransform1(Rect(Point(targetLinkPoint.x - basedImagePoint.x, 0), Point(targetLinkPoint.x, image02.rows)));
	image2Overlap = image02(Rect(0, 0, image1Overlap.cols, image1Overlap.rows));
	Mat image1ROICopy = image1Overlap.clone();  //复制一份图1的重叠部分
	for (int i = 0; i < image1Overlap.rows; i++)
	{
		for (int j = 0; j < image1Overlap.cols; j++)
		{
			double weight;
			weight = (double)j / image1Overlap.cols;  //随距离改变而改变的叠加系数
			image1Overlap.at<Vec3b>(i, j)[0] = (1 - weight)*image1ROICopy.at<Vec3b>(i, j)[0] + weight * image2Overlap.at<Vec3b>(i, j)[0];
			image1Overlap.at<Vec3b>(i, j)[1] = (1 - weight)*image1ROICopy.at<Vec3b>(i, j)[1] + weight * image2Overlap.at<Vec3b>(i, j)[1];
			image1Overlap.at<Vec3b>(i, j)[2] = (1 - weight)*image1ROICopy.at<Vec3b>(i, j)[2] + weight * image2Overlap.at<Vec3b>(i, j)[2];
		}
	}
	Mat ROIMat = image02(Rect(Point(image1Overlap.cols, 0), Point(image02.cols, image02.rows)));	 //图2中不重合的部分
	ROIMat.copyTo(Mat(imageTransform1, Rect(targetLinkPoint.x, 0, ROIMat.cols, image02.rows))); //不重合的部分直接衔接上去
	namedWindow("拼接结果", 0);
	imshow("拼接结果", imageTransform1);
	//imwrite("D:\\拼接结果.jpg", imageTransform1);
	waitKey();
}

void Image_stitching::surf_stitching_BruteForce(cv::String path1, cv::String path2)
{
	Mat image01 = imread(path1);
	Mat image02 = imread(path2);
	//imshow("拼接图像1", image01);
	//imshow("拼接图像2", image02);

	//灰度图转换
	Mat image1, image2;
	cvtColor(image01, image1, CV_RGB2GRAY);
	cvtColor(image02, image2, CV_RGB2GRAY);

	//提取特征点  
	SurfFeatureDetector surfDetector(800);  // 海塞矩阵阈值
	vector<KeyPoint> keyPoint1, keyPoint2;
	surfDetector.detect(image1, keyPoint1);
	surfDetector.detect(image2, keyPoint2);

	//特征点描述，为下边的特征点匹配做准备  
	SurfDescriptorExtractor surfDescriptor;
	Mat imageDesc1, imageDesc2;
	surfDescriptor.compute(image1, keyPoint1, imageDesc1);
	surfDescriptor.compute(image2, keyPoint2, imageDesc2);

	//获得匹配特征点，并提取最优配对  	
	BFMatcher matcher;
	vector<DMatch> matchePoints;
	matcher.match(imageDesc1, imageDesc2, matchePoints, Mat());
	sort(matchePoints.begin(), matchePoints.end()); //特征点排序	
													//获取排在前N个的最优匹配特征点
	vector<Point2f> imagePoints1, imagePoints2;
	for (int i = 0; i < 10; i++)
	{
		imagePoints1.push_back(keyPoint1[matchePoints[i].queryIdx].pt);
		imagePoints2.push_back(keyPoint2[matchePoints[i].trainIdx].pt);
	}

	//获取图像1到图像2的投影映射矩阵，尺寸为3*3
	Mat homo = findHomography(imagePoints1, imagePoints2, CV_RANSAC);
	Mat adjustMat = (Mat_<double>(3, 3) << 1.0, 0, image01.cols, 0, 1.0, 0, 0, 0, 1.0);
	Mat adjustHomo = adjustMat * homo;

	//获取最强配对点在原始图像和矩阵变换后图像上的对应位置，用于图像拼接点的定位
	Point2f originalLinkPoint, targetLinkPoint, basedImagePoint;
	originalLinkPoint = keyPoint1[matchePoints[0].queryIdx].pt;
	targetLinkPoint = getTransformPoint(originalLinkPoint, adjustHomo);
	basedImagePoint = keyPoint2[matchePoints[0].trainIdx].pt;

	//图像配准
	Mat imageTransform1;
	warpPerspective(image01, imageTransform1, adjustMat*homo, Size(image02.cols + image01.cols + 110, image02.rows));

	//在最强匹配点左侧的重叠区域进行累加，是衔接稳定过渡，消除突变
	Mat image1Overlap, image2Overlap; //图1和图2的重叠部分	
	image1Overlap = imageTransform1(Rect(Point(targetLinkPoint.x - basedImagePoint.x, 0), Point(targetLinkPoint.x, image02.rows)));
	image2Overlap = image02(Rect(0, 0, image1Overlap.cols, image1Overlap.rows));
	Mat image1ROICopy = image1Overlap.clone();  //复制一份图1的重叠部分
	for (int i = 0; i < image1Overlap.rows; i++)
	{
		for (int j = 0; j < image1Overlap.cols; j++)
		{
			double weight;
			weight = (double)j / image1Overlap.cols;  //随距离改变而改变的叠加系数
			image1Overlap.at<Vec3b>(i, j)[0] = (1 - weight)*image1ROICopy.at<Vec3b>(i, j)[0] + weight * image2Overlap.at<Vec3b>(i, j)[0];
			image1Overlap.at<Vec3b>(i, j)[1] = (1 - weight)*image1ROICopy.at<Vec3b>(i, j)[1] + weight * image2Overlap.at<Vec3b>(i, j)[1];
			image1Overlap.at<Vec3b>(i, j)[2] = (1 - weight)*image1ROICopy.at<Vec3b>(i, j)[2] + weight * image2Overlap.at<Vec3b>(i, j)[2];
		}
	}
	Mat ROIMat = image02(Rect(Point(image1Overlap.cols, 0), Point(image02.cols, image02.rows)));	 //图2中不重合的部分
	ROIMat.copyTo(Mat(imageTransform1, Rect(targetLinkPoint.x, 0, ROIMat.cols, image02.rows))); //不重合的部分直接衔接上去
	namedWindow("拼接结果", 0);
	imshow("拼接结果", imageTransform1);
	//imwrite("D:\\拼接结果.jpg", imageTransform1);
	waitKey();
}


//计算原始图像点位在经过矩阵变换后在目标图像上对应位置
Point2f Image_stitching::getTransformPoint(const Point2f originalPoint, const Mat &transformMaxtri)
{
	Mat originelP, targetP;
	originelP = (Mat_<double>(3, 1) << originalPoint.x, originalPoint.y, 1.0);
	targetP = transformMaxtri * originelP;
	float x = targetP.at<double>(0, 0) / targetP.at<double>(2, 0);
	float y = targetP.at<double>(1, 0) / targetP.at<double>(2, 0);
	return Point2f(x, y);
}
