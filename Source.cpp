#include <iostream>

#include <opencv2\opencv.hpp>
#include <opencv2\core\core.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <opencv2\ximgproc\slic.hpp>
#include <opencv2\imgproc\imgproc.hpp>
#include <opencv2\imgcodecs\imgcodecs.hpp>

#include <opencv2/xfeatures2d.hpp>

#include "Descriptor.h"
#include "defines.h"
#include "Superpixel.h"
#include "Matcher.h"



int main()
{
	Descriptor dp1, dp2;
	Superpixel sp1, sp2;
	Matcher matcher;

	cv::Mat img1 = cv::imread("./IMG/test2/83x.bmp");
	cv::Mat img2 = cv::imread("./IMG/test2/1811.bmp");

	cvtColor(img1, img1, CV_BGR2Lab);
	cvtColor(img2, img2, CV_BGR2Lab);

	dp1.Set(img1, DAISY);
	sp1.Set(dp1);

	dp2.Set(img2, DAISY);
	sp2.Set(dp2);
	
	matcher.Set(sp1, sp2);

	matcher.Debug();
}