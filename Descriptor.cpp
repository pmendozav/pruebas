#include "Descriptor.h"
#include "utils.h"


Descriptor::Descriptor()
{
}


Descriptor::~Descriptor()
{
}

void Descriptor::Set(const cv::Mat &img, const DESCRIPTOR_TYPE &type)
{
	this->img = img;

	switch (type)
	{
	case DAISY:
		RunDaisy();
		break;
	default:
		error("It doesn't supported yet!");
		break;
	}
}

void Descriptor::RunDaisy()
{
	cv::Ptr<cv::xfeatures2d::DAISY> daisy =
		cv::xfeatures2d::DAISY::create(5, 3, 4, 8,
			cv::xfeatures2d::DAISY::NRM_FULL, cv::noArray(), false, false);

	daisy->compute(img, feat);

	width = img.cols;
	height = img.rows;
	channels = feat.cols;
}

const cv::Mat Descriptor::Get(const int &row, const int &col)
{
	return feat.row(row * width + col);
}
