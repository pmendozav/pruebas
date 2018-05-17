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

#include "utils.h"

int test_daisy(std::string fname_src_1, std::string fname_src_2, std::string fname_dst)
{
	const float nn_match_ratio = 0.7f;      // Nearest neighbor matching ratio
	const float keypoint_diameter = 15.0f;

	cv::Mat img1 = cv::imread(fname_src_1);
	cv::Mat img2 = cv::imread(fname_src_2);

	std::vector<cv::KeyPoint> keypoints1, keypoints2;

	// Add every pixel to the list of keypoints for each image
	for (float xx = keypoint_diameter; xx < img1.size().width - keypoint_diameter; xx++) {
		for (float yy = keypoint_diameter; yy < img1.size().height - keypoint_diameter; yy++) {
			keypoints1.push_back(cv::KeyPoint(xx, yy, keypoint_diameter));
			keypoints2.push_back(cv::KeyPoint(xx, yy, keypoint_diameter));
		}
	}

	cv::Mat desc1, desc2;

	
	cv::Ptr<cv::xfeatures2d::DAISY> descriptor_extractor = cv::xfeatures2d::DAISY::create();

	// Compute DAISY descriptors for both images 
	descriptor_extractor->compute(img1, keypoints1, desc1);
	descriptor_extractor->compute(img2, keypoints2, desc2);
	Stopwatch timer;
	std::vector <std::vector<cv::DMatch>> matches;

	// For each descriptor in image1, find 2 closest matched in image2 (note: couldn't get BF matcher to work here at all)
	cv::FlannBasedMatcher flannmatcher;
	flannmatcher.add(desc1);
	flannmatcher.train();
	flannmatcher.knnMatch(desc2, matches, 2);


	// ignore matches with high ambiguity -- i.e. second closest match not much worse than first
	// push all remaining matches back into DMatch Vector "good_matches" so we can draw them using DrawMatches
	int                 num_good = 0;
	std::vector<cv::KeyPoint>    matched1, matched2;
	std::vector<cv::DMatch>      good_matches;

	for (int i = 0; i < matches.size(); i++) {
		cv::DMatch first = matches[i][0];
		cv::DMatch second = matches[i][1];

		if (first.distance < nn_match_ratio * second.distance) {
			matched1.push_back(keypoints1[first.trainIdx]);
			matched2.push_back(keypoints2[first.queryIdx]);
			good_matches.push_back(cv::DMatch(num_good, num_good, 0));
			num_good++;
		}
	}

	timer.toc("total time: ");

	cv::Mat res;
	cv::drawMatches(img1, matched1, img2, matched2, good_matches, res);
	cv::imwrite(fname_dst, res);
}

int main()
{
	//test_daisy("./IMG/test3/141.bmp",
	//	"./IMG/test3/249.bmp",
	//	"./results/test3/deisy.tif");
	//return 0;

	Descriptor dp1, dp2;
	Superpixel sp1, sp2;
	Matcher matcher;

	//test1
	//cv::Mat img1 = cv::imread("./IMG/test1/1.tif");
	//cv::Mat img2 = cv::imread("./IMG/test1/2.tif");

	//test2
	//cv::Mat img1 = cv::imread("./IMG/test2/83x.bmp");
	//cv::Mat img2 = cv::imread("./IMG/test2/1811.bmp");

	//test3
	cv::Mat img1 = cv::imread("./IMG/test3/141.bmp");
	cv::Mat img2 = cv::imread("./IMG/test3/249.bmp");
	
	cvtColor(img1, img1, CV_BGR2Lab);
	cvtColor(img2, img2, CV_BGR2Lab);

	dp1.Set(img1, DAISY);
	sp1.Set(dp1);

	dp2.Set(img2, DAISY);
	sp2.Set(dp2);
	
	matcher.Set(sp1, sp2);

	matcher.Debug();
}