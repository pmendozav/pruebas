#include "Matcher.h"
#include "utils.h"
#include "defines.h"

Matcher::Matcher()
{
	
}


Matcher::~Matcher()
{
}

void Matcher::Set(Superpixel &sp_from, Superpixel &sp_to)
{
	this->sp_from = sp_from;
	this->sp_to = sp_to;
}

std::set<int> Matcher::FindNeighs(const std::vector<SP> &sp_info,
	const std::set<int> &from)
{
	std::set<int> _to;

	for (const auto &id : from)
	{
		for (const auto &other_id : sp_info[id].neighs)
		{
			_to.insert(other_id);
		}
	}

	return _to;
}

float Matcher::MatchCost(const cv::Mat &dp1, const cv::Mat &dp2)
{
	return cv::norm(dp1 - dp2, cv::NORM_L1);
}

float Matcher::MatchCostColor(const cv::Vec3f &color1, const cv::Vec3f &color2)
{
	return cv::norm(color1 - color2, cv::NORM_L1);
}

void Matcher::Check1d(const std::vector<cv::Point2i> &disps,
	std::vector<bool> &flags,
	const int &dim)
{
	bool is_short;
	int count;
	std::vector<int> vals;
	const int n = disps.size();

	if (n != flags.size()) flags.resize(n, true);

	vals.resize(n);
	count = 0;
	if (dim == 0) for (auto i = 0; i < n; i++) vals[i] = disps[i].x;
	else if (dim == 1) for (auto i = 0; i < n; i++) vals[i] = disps[i].y;
	else error("dim it is not valid");

	

	//check short displacements
	//if (vals[i])

}

void Matcher::RemoveOutliers()
{
	const int n_superpixels = sp_from.n_superpixels;
	std::set<int> neighs;
	cv::Point2i disp;
	std::vector<cv::Point2i> disps;
	std::vector<bool> flags;

	for (int id = 0; id < n_superpixels; id++)
	{
		neighs = sp_from.sp_info[id].neighs;
		neighs.insert(id);

		disps.clear(); //TODO: use a fixed vector with flags
		for (const auto &l_from : neighs)
		{
			//disp = sp_from.sp_info[l_from].displacement;
			disps.push_back(sp_from.sp_info[l_from].displacement);
		}

		flags.resize(disps.size());

	}
}

void Matcher::FindOneMatch(const int &id_from,
	int &id_to,
	float &best_cost,
	cv::Point2i &displacement)
{
	cv::Mat tmp1, tmp2;
	float cost, cost2;

	std::set<int> neighs1 = sp_from.sp_info[id_from].neighs;
	std::set<int> neighs2 = FindNeighs(sp_from.sp_info, neighs1);
	std::set<int> neighs3 = FindNeighs(sp_from.sp_info, neighs2);
	std::set<int> neighs4 = FindNeighs(sp_from.sp_info, neighs3);
	std::set<int> neighs5 = FindNeighs(sp_from.sp_info, neighs4);
	
	std::set<int> neighs;
	neighs1.insert(id_from);
	for (const auto &e : neighs1) neighs.insert(e);
	for (const auto &e : neighs2) neighs.insert(e);
	for (const auto &e : neighs3) neighs.insert(e);
	for (const auto &e : neighs4) neighs.insert(e);
	for (const auto &e : neighs5) neighs.insert(e);

	cv::Mat dp_from = sp_from.sp_info[id_from].dp;
	cv::Vec3f color_from = sp_from.sp_info[id_from].color;

	best_cost = 1e10;
	//tmp1 = cv::Mat::zeros(sp_to.dp.img.size(), CV_8UC1);
	//tmp2 = cv::Mat::zeros(sp_to.dp.img.size(), CV_8UC1);

	//cv::Mat img_from = sp_from.Debug("");
	//cv::Mat img_to = sp_to.Debug("");
	for (auto id : neighs)
	{
		cv::Point2i pt_to = sp_from.sp_info[id].center;
		int label = sp_to.labels.at<int>(pt_to.y, pt_to.x); //TODO: CHECK
		//std::cout << label << std::endl;
		//cv::circle(img_to, pt_to, 3, cv::Scalar(0, 255, 0), CV_FILLED);

		//cv::bitwise_or(tmp2, sp_to.labels == label, tmp2);
		//cv::imwrite("tmp2.tif", tmp2);

		cost = MatchCost(dp_from, sp_to.sp_info[label].dp);
		cost2 = 0.015 * MatchCostColor(color_from, sp_to.sp_info[label].color);
		cost = cost + cost2;
		if (cost < best_cost)
		{
			id_to = label;
			best_cost = cost;
		}
	}

	displacement = sp_from.sp_info[id_to].center - sp_from.sp_info[id_from].center;

	//cvtColor(img_to, img_to, CV_Lab2BGR);
	//cv::imwrite("img_to.tif", img_to);
	//cvtColor(img_to, img_to, CV_BGR2Lab);
	//cv::imwrite("tmp2.tif", tmp2);
}

void Matcher::FindMatches()
{
	const int n_superpixels = sp_from.n_superpixels;
	
	for (int i = 0; i < n_superpixels; i++)
	{
		FindOneMatch(i, sp_from.sp_info[i].correspondence_id, sp_from.sp_info[i].best_cost, sp_from.sp_info[i].displacement);

		sp_from.sp_info[i].displacement = sp_from.sp_info[i].center - sp_from.sp_info[sp_from.sp_info[i].correspondence_id].center;
	}
}

void Matcher::DrawMatches(const std::set<int> neighs)
{
	cv::Mat img_from = sp_from.dp.img.clone();
	cv::Mat img_to = sp_to.dp.img.clone();

	cvtColor(img_from, img_from, CV_Lab2BGR);
	cvtColor(img_to, img_to, CV_Lab2BGR);

	cv::Mat img = img_from;
	cv::hconcat(img, img_to, img);

	for (const auto &label : neighs)
	{
		//sp_from.sp_info[id_from].correspondence_id = id_to;

		cv::Point2i pt1 = sp_from.sp_info[label].center;
		cv::Point2i pt2 = sp_to.sp_info[sp_from.sp_info[label].correspondence_id].center;
		//std::cout << sp_to.sp_info[label].correspondence_id << std::endl;
		//std::cout << pt1 << " " << pt2 << std::endl;
		pt2.x += img_from.cols;
		cv::line(img, pt1, pt2, cv::Scalar(rand() % 255, rand() % 255, rand() % 255));
		
	}

	cv::imwrite("haber.tif", img);
}

void Matcher::DrawMatches(const std::vector < std::pair<int, int>> &matches)
{
	cv::Mat img_from = sp_from.dp.img.clone();
	cv::Mat img_to = sp_to.dp.img.clone();
	
	cvtColor(img_from, img_from, CV_Lab2BGR);
	cvtColor(img_to, img_to, CV_Lab2BGR);

	cv::Mat img = img_from;
	cv::hconcat(img, img_to, img);

	
	cv::Mat img0 = img.clone();
	for (const auto &p : matches)
	{
		img = img0.clone();
		cv::Point2i pt1 = sp_from.sp_info[p.first].center;
		cv::Point2i pt2 = sp_to.sp_info[p.second].center;
		pt2.x += img_from.cols;
		cv::line(img, pt1, pt2, cv::Scalar(rand() % 255, rand() % 255, rand() % 255));
		cv::imwrite("haber.tif", img);
		pause();	
	}
	
	
}

void Matcher::Debug(std::string str)
{
	float best_cost = 1e10;
	int id_from = 422, id_to; //50
	cv::Point2i disp;

	FindOneMatch(id_from, id_to, best_cost, disp);
	//return;

	//cv::Mat asd = sp_from.labels.clone();
	//asd.convertTo(asd, CV_16UC1);
	//cv::imwrite("labels_from.tif", asd);

	const int n_superpixels = sp_from.n_superpixels;
	std::vector < std::pair<int, int>> matches(n_superpixels);
	for (int i = 0; i < n_superpixels; i++)
	{
		id_from = i;

		FindOneMatch(id_from, id_to, best_cost, disp);
		matches[i].first = id_from;
		matches[i].second = id_to;
		//cv::Mat img_from = sp_from.dp.img.clone();
		//cv::Mat img_to = sp_to.dp.img.clone();

		//cv::Point2i pt_from = sp_from.sp_info[id_from].center;
		//cv::Point2i pt_to = sp_to.sp_info[id_to].center;

		//cv::circle(img_from, pt_from, 10, cv::Scalar(255, 0, 255), CV_FILLED);
		//cv::circle(img_to, pt_to, 10, cv::Scalar(255, 0, 255), CV_FILLED);


		//cvtColor(img_from, img_from, CV_Lab2BGR);
		//cvtColor(img_to, img_to, CV_Lab2BGR);
		//cv::imwrite("debug_match_from.tif", img_from);
		//cv::imwrite("debug_match_to.tif", img_to);
		//std::cout << i << ") " << best_cost << std::endl;
		//pause();

		sp_from.sp_info[id_from].correspondence_id = id_to;
	}
	
	for (int id_from = 0; id_from < n_superpixels; id_from++)
	{
		std::set<int> neighs1 = sp_from.sp_info[id_from].neighs;
		std::set<int> neighs2 = FindNeighs(sp_from.sp_info, neighs1);
		std::set<int> neighs;
		neighs1.insert(id_from);
		for (const auto &e : neighs1) neighs.insert(e);
		neighs.insert(id_from);
		DrawMatches(neighs);
		pause();
	}
	



	//cv::Mat mask1, mask2, mask3, mask4;
	//sp_from.TestNeighs(id_from, mask1, mask2);
	//cv::imwrite("ID_FROM_NEIGHS.tif", mask2);

	//sp_to.TestNeighs(id_from, mask3, mask4);
	//cv::imwrite("ID_TO_NEIGHS.tif", mask4);

	//sp_from.Debug("SP_FROM_");
	//sp_to.Debug("SP_TO_");
}