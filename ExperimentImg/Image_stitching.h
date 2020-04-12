#pragma once
#include "highgui/highgui.hpp"    
#include "opencv2/nonfree/nonfree.hpp"    
#include "opencv2/legacy/legacy.hpp"
#include <opencv2/core.hpp>
#include <iostream>  
using namespace cv;

class Image_stitching
{
public:
	static Point2f Image_stitching::getTransformPoint(const Point2f originalPoint, const Mat &transformMaxtri);
	static void stitching(cv::String path1,cv::String path2);
};