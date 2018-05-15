#include "Superpixel2.h"
#include <array>
#include "utils.h"

Superpixel2::Superpixel2()
{
}


Superpixel2::~Superpixel2()
{
}

void Superpixel2::Set(Descriptor &dp)
{
	this->dp = dp;

	cv::Mat img = dp.img;

	cv::imwrite("img.tif", img);

	cv::Ptr<cv::ximgproc::SuperpixelSLIC> lsc =
		cv::ximgproc::createSuperpixelSLIC(
			img,
			101,
			20,
			10);

	lsc->iterate(10);
	lsc->enforceLabelConnectivity(10);
	lsc->getLabels(labels);
	lsc->getLabelContourMask(contours);

	n_superpixels = lsc->getNumberOfSuperpixels();


	//labels.convertTo(labels, CV_16UC1, 65535.0 / n_superpixels);
	//cv::imwrite("labels.tif", labels);
	//cv::imwrite("contours.tif", contours);

	//std::cout << n_superpixels << std::endl;

	SetMeanDP();
}

inline void Superpixel2::SetMeanDP()
{
	sp_info.resize(n_superpixels);

	_CalcClusters();
	_CalcNeighs();
	_CalcMeanDPandColor();
}

void Superpixel2::_CalcClusters()
{
	const int rows = dp.height;
	const int cols = dp.width;

	for (auto &sp : sp_info)
		sp.center = cv::Point2i(0, 0);
	
	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < cols; j++)
		{
			const int label = labels.at<int>(i, j);

			sp_info[label].pts.push_back(cv::Point2i(j, i));
			sp_info[label].center.x += j;
			sp_info[label].center.y += i;
		}
	}

	for (auto &sp : sp_info)
	{
		sp.size = sp.pts.size();
		sp.center.x /= (float)(sp.size);
		sp.center.y /= (float)(sp.size);
	}


}

void Superpixel2::_CalcNeighs()
{
	cv::Mat visited;
	const int rows = labels.rows;
	const int cols = labels.cols;
	const int n_neigh = 8;
	const std::array<int, n_neigh> dx = { -1,  0,  1, -1, 1, -1, 0, 1 };
	const std::array<int, n_neigh> dy = { -1, -1, -1,  0, 0,  1, 1, 1 };

	for (int y = 0; y < rows; y++)
	{
		for (int x = 0; x < cols; x++)
		{
			if (contours.at<uchar>(y, x) == 0) continue;

			//visited.at<uchar>(y, x) = 255;
			for (int i = 0; i < n_neigh; i++)
			{
				const int _x = __max(__min(x + dx[i], cols - 1), 0);
				const int _y = __max(__min(y + dy[i], rows - 1), 0);

				//if (contours.at<uchar>(_y, _x) != 0 || visited.at<uchar>(y, x) != 0) continue;
				if (contours.at<uchar>(_y, _x) != 0) continue;

				const int label1 = labels.at<int>(y, x);
				const int label2 = labels.at<int>(_y, _x);

				if (label1 == label2) continue;

				sp_info[label1].neighs.insert(label2);
				sp_info[label2].neighs.insert(label1);
			}
		}
	}
}

void Superpixel2::_CalcMeanDPandColor()
{
	const int channels = dp.channels;
	cv::Mat mean_dp(1, channels, CV_32FC1);
	//cv::Vec3b mean_color;
	float mean_color_x, mean_color_y, mean_color_z;

	//std::cout << dp.img.type() << std::endl;

 	for (int i = 0; i < n_superpixels; i++)
	{
		//mean_color = cv::Vec3b(0, 0, 0);
		mean_color_x = 0;
		mean_color_y = 0;
		mean_color_z = 0;
		mean_dp.setTo(0);
		for (const auto &pt : sp_info[i].pts)
		{
			mean_dp += dp.Get(pt.y, pt.x);
			//mean_color += dp.img.at<cv::Vec3b>(pt);
			mean_color_x += dp.img.at<cv::Vec3b>(pt)[0];//TODO: -.-
			mean_color_y += dp.img.at<cv::Vec3b>(pt)[1];
			mean_color_z += dp.img.at<cv::Vec3b>(pt)[2];
		}

		//std::cout << dp.img.row(100).col(100) << std::endl;
		//std::cout << dp.img.at<cv::Vec3b>(100, 100) << std::endl;

		mean_dp /= sp_info[i].size;
		sp_info[i].dp = mean_dp.clone();
		sp_info[i].color = cv::Vec3b(mean_color_x/ sp_info[i].size,
			mean_color_y / sp_info[i].size, 
			mean_color_z / sp_info[i].size);
	}
}

cv::Mat Superpixel2::Debug(std::string str)
{
	cv::Mat img = dp.img.clone();

	const int rows = labels.rows;
	const int cols = labels.cols;

	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < cols; j++)
		{
			if (contours.at<uchar>(i, j) != 0)
			{
				img.at<cv::Vec3b>(i, j) = cv::Vec3b(255, 0, 0);
			}
		}
	}

	for (auto e : sp_info)
	{
		cv::circle(img, e.center, 3, cv::Scalar(255, 255, 0));
	}

	if (str.compare("") == 0)
		cv::imwrite("debug.tif", img);
	else
		cv::imwrite(str + "debug.tif", img);

	return img;

	//srand(time(NULL));
	//int rn = rand() % n_superpixels;
	//rn = 50;
	//cv::Mat mask = labels == rn;

	//if (str.compare("") == 0)
	//	cv::imwrite("debug_mask1.tif", mask);
	//else
	//	cv::imwrite(str + "debug_mask1.tif", mask);
	//

	//for (auto id : sp_info[rn].neighs)
	//{
	//	cv::bitwise_or(mask, labels == id, mask);
	//}

	//if (str.compare("") == 0)
	//	cv::imwrite("debug_mask2.tif", mask);
	//else
	//	cv::imwrite(str + "debug_mask2.tif", mask);
}

void Superpixel2::TestNeighs(const int id, cv::Mat &mask1, cv::Mat &mask2)
{
	mask1 = labels == id;
	mask2 = mask1.clone();

	for (auto id : sp_info[id].neighs)
	{
		cv::bitwise_or(mask2, labels == id, mask2);
	}
}