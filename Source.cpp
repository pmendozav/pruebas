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
#include "Superpixel2.h"
#include "Matcher.h"



int main()
{
	Descriptor dp1, dp2;
	Superpixel2 sp1, sp2;
	Matcher matcher;

	cv::Mat img1 = cv::imread("./test2/1799.bmp");
	cv::Mat img2 = cv::imread("./test2/1811.bmp");

	cvtColor(img1, img1, CV_BGR2Lab);
	cvtColor(img2, img2, CV_BGR2Lab);

	dp1.Set(img1, DAISY);
	sp1.Set(dp1);

	dp2.Set(img2, DAISY);
	sp2.Set(dp2);
	
	matcher.Set(sp1, sp2);

	matcher.Debug();

	//cv::Ptr<cv::xfeatures2d::DAISY> daisy =
	//	cv::xfeatures2d::DAISY::create(5, 3, 4, 8,
	//		cv::xfeatures2d::DAISY::NRM_FULL, cv::noArray(), false, false);

	//cv::Mat outFeatures;
	//daisy->compute(img, outFeatures);

	//std::cout << img.size() << std::endl;
	//std::cout << outFeatures.size() << std::endl;
	//std::cout << outFeatures.depth() << std::endl;

	//cv::Mat img2 = cv::imread("C:\\Users\\Public\\Pictures\\Sample Pictures\\Desert.jpg", cv::IMREAD_GRAYSCALE);

	//cv::Mat outFeatures2;
	//daisy->compute(img, outFeatures2);

	//std::cout << img2.size() << std::endl;
	//std::cout << outFeatures2.size() << std::endl;
	//std::cout << outFeatures2.depth() << std::endl;

}