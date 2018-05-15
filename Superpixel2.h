#pragma once
#include <set>
#include "Descriptor.h"

#include <opencv2\ximgproc\slic.hpp>

enum SUPERPIXEL_TYPE {SLIC};

struct SP
{
	std::vector<cv::Point2i> pts;
	std::set<int> neighs;
	cv::Point2i center;
	int size;
	cv::Mat dp;
	cv::Vec3b color;

	int correspondence_id;
	float best_cost;
	cv::Point2i displacement;
};

class Superpixel2
{
private:
	friend class Matcher;

	Descriptor dp;
	std::vector<SP> sp_info;

	cv::Mat labels, contours;
	int n_superpixels;

	void _CalcClusters();
	void _CalcNeighs();
	void _CalcMeanDPandColor();
protected:
	inline void SetMeanDP();
public:
	Superpixel2();
	~Superpixel2();

	void Set(Descriptor &_dp);
	cv::Mat Debug(std::string str = "");

	void TestNeighs(const int id, cv::Mat &mask1, cv::Mat &mask2);
};

