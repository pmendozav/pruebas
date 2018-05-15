#pragma once
#include <opencv2\opencv.hpp>
#include <opencv2\core\core.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <opencv2\ximgproc\slic.hpp>
#include <opencv2\imgproc\imgproc.hpp>
#include <opencv2\imgcodecs\imgcodecs.hpp>

#include <opencv2/xfeatures2d.hpp>

enum DESCRIPTOR_TYPE {DAISY};

class Descriptor
{
private:
	friend class Superpixel2;
	friend class Matcher;

	cv::Mat img;
	cv::Mat feat;
	int width, height, channels;

	void RunDaisy();
public:
	Descriptor();
	~Descriptor();

	void Set(const cv::Mat &img, const DESCRIPTOR_TYPE &type);
	const cv::Mat Get(const int &row, const int &col);
};

