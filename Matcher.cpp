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

void Matcher::DebugNeighs(int id_from, std::set<int> neighs)
{
	cv::Mat img_from = sp_from.dp.img.clone();
	cv::Mat img_to = sp_to.dp.img.clone();

	cvtColor(img_from, img_from, CV_Lab2BGR);
	cvtColor(img_to, img_to, CV_Lab2BGR);

	cv::Mat img = img_from;
	cv::hconcat(img, img_to, img);

	for (const auto &label : neighs)
	{
		cv::Point2i pt1 = sp_from.sp_info[label].center;
		cv::Point2i pt2 = sp_to.sp_info[sp_from.sp_info[label].correspondence_id].center;

		pt2.x += img_from.cols;
		cv::line(img, pt1, pt2, cv::Scalar(rand() % 255, rand() % 255, rand() % 255));

		//std::cout << label << ") " << sp_from.sp_info[label].displacement << std::endl;
	}

	//
	cv::Point2i pt1 = sp_from.sp_info[id_from].center;
	cv::Point2i pt2 = sp_to.sp_info[sp_from.sp_info[id_from].correspondence_id].center;

	pt2.x += img_from.cols;
	cv::line(img, pt1, pt2, cv::Scalar(0, 0, 0));
	//std::cout << "REF: " << sp_from.sp_info[id_from].displacement << std::endl;
	
	//

	cv::imwrite("haber.tif", img);
}

bool Matcher::CheckOneMatch(const std::vector<cv::Point2i> &disp_neighs,
	const cv::Point2i &from)
{
	const int max_disp_sign = 15;
	const int max_disp_abs = 10;
	
	cv::Point2i tmp1, tmp2;
	int x1, x2, y1, y2;

	x1 = abs(from.x);
	y1 = abs(from.y);

	for (const auto &pt : disp_neighs)
	{
		x2 = abs(pt.x);
		y2 = abs(pt.y);

		tmp1 = cv::Point2i(abs(pt.x), abs(pt.y)) - tmp2;
		

		if (abs(x1 - x2) < max_disp_abs && abs(y1 - y2) < max_disp_abs)
		{
			return true;
		}
			
		
		//if (abs(abs(pt.x) - abs(from.x)) < max_disp_sign &&
		//	abs(abs(pt.y) - abs(from.y)) < max_disp_abs)
		//	return true;
	}

	return false;
}

bool Matcher::RemoveOneOutliersStep2(const int &id_from,
	const int &disp_max,
	const int &count_points,
	const int &max_neighs,
	const int &max_iters)
{
	volatile int iters, count;
	std::set<int> valid_neighs, neighs;

	if (!(sp_from.sp_info[id_from].valid_disp)) return false;

	int id_to = sp_from.sp_info[id_from].correspondence_id;

	iters = 0;
	neighs = sp_from.sp_info[id_from].neighs;
	while (valid_neighs.size() < max_neighs && iters++ < max_iters)
	{
		for (const auto &l_from : neighs)
		{
			auto l_to = sp_from.sp_info[l_from].correspondence_id;

			//deberia ser en el to.. pero este no lo seteo -.-
			if (sp_from.sp_info[l_from].valid_disp)
			//if (sp_to.sp_info[l_to].valid_disp)
			{
				valid_neighs.insert(l_to);
			}
				

			if (valid_neighs.size() >= max_neighs) break;
		}

		neighs = FindNeighs(sp_from.sp_info, neighs);
	}

	count = 0;
	for (const auto &label : valid_neighs)
	{
		const cv::Point2i pt = sp_to.sp_info[label].center - sp_to.sp_info[id_to].center;
		if (__max(abs(pt.x), abs(pt.y)) < disp_max) count++;
	}

	return (count >= count_points);
}

void Matcher::RemoveAllOutliersStep2(const int &disp_max,
	const int &count_points,
	const int &max_neighs,
	const int &max_iters)
{
	int count = 0;
	const int n_superpixels = sp_from.n_superpixels;

	for (int i = 0; i < n_superpixels; i++)
	{
		if (!RemoveOneOutliersStep2(i, disp_max, count_points, max_neighs, max_iters))
			sp_from.sp_info[i].valid_disp = false;
		else
			count++;
	}

	std::cout << count << std::endl;
}

void Matcher::RemoveAllOutliers()
{
	const int n_superpixels = sp_from.n_superpixels;
	std::set<int> neighs;
	std::vector<cv::Point2i> disps;

	for (int id_from = 0; id_from < n_superpixels; id_from++)
	{
		neighs = sp_from.sp_info[id_from].neighs;

		disps.clear(); //TODO: use a fixed vector with flags
		for (const auto &l_from : neighs)
		{
			disps.push_back(sp_from.sp_info[l_from].displacement);
		}

		if (!CheckOneMatch(disps, sp_from.sp_info[id_from].displacement))
		{
			sp_from.sp_info[id_from].valid_disp = false;
		}
	}
}

void Matcher::RemoveAllOutliersInv()
{
	const int n_superpixels = sp_to.n_superpixels;
	std::set<int> neighs;
	std::vector<cv::Point2i> disps;

	for (int id_from = 0; id_from < n_superpixels; id_from++)
	{
		neighs = sp_to.sp_info[id_from].neighs;

		disps.clear(); //TODO: use a fixed vector with flags
		for (const auto &l_from : neighs)
		{
			disps.push_back(sp_to.sp_info[l_from].displacement);
		}

		if (!CheckOneMatch(disps, sp_to.sp_info[id_from].displacement))
		{
			sp_to.sp_info[id_from].valid_disp = false;
		}
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
	for (auto id : neighs)
	{
		cv::Point2i pt_to = sp_from.sp_info[id].center;
		int label = sp_to.labels.at<int>(pt_to.y, pt_to.x); //TODO: CHECK

		cost = MatchCost(dp_from, sp_to.sp_info[label].dp);
		cost2 = 0.015 * MatchCostColor(color_from, sp_to.sp_info[label].color);
		cost = cost + cost2;
		if (cost < best_cost)
		{
			id_to = label;
			best_cost = cost;
		}
	}

	displacement = sp_to.sp_info[id_to].center - sp_from.sp_info[id_from].center;
}

void Matcher::FindOneMatchInv(const int &id_from,
	int &id_to,
	float &best_cost,
	cv::Point2i &displacement)
{
	cv::Mat tmp1, tmp2;
	float cost, cost2;

	std::set<int> neighs1 = sp_to.sp_info[id_from].neighs;
	std::set<int> neighs2 = FindNeighs(sp_to.sp_info, neighs1);
	std::set<int> neighs3 = FindNeighs(sp_to.sp_info, neighs2);
	std::set<int> neighs4 = FindNeighs(sp_to.sp_info, neighs3);
	std::set<int> neighs5 = FindNeighs(sp_to.sp_info, neighs4);

	std::set<int> neighs;
	neighs1.insert(id_from);
	for (const auto &e : neighs1) neighs.insert(e);
	for (const auto &e : neighs2) neighs.insert(e);
	for (const auto &e : neighs3) neighs.insert(e);
	for (const auto &e : neighs4) neighs.insert(e);
	for (const auto &e : neighs5) neighs.insert(e);

	cv::Mat dp_from = sp_to.sp_info[id_from].dp;
	cv::Vec3f color_from = sp_to.sp_info[id_from].color;

	best_cost = 1e10;
	for (auto id : neighs)
	{
		cv::Point2i pt_to = sp_to.sp_info[id].center;
		int label = sp_from.labels.at<int>(pt_to.y, pt_to.x); //TODO: CHECK

		cost = MatchCost(dp_from, sp_from.sp_info[label].dp);
		cost2 = 0.015 * MatchCostColor(color_from, sp_from.sp_info[label].color);
		cost = cost + cost2;
		if (cost < best_cost)
		{
			id_to = label;
			best_cost = cost;
		}
	}

	displacement = sp_from.sp_info[id_to].center - sp_to.sp_info[id_from].center;
}

void Matcher::FindMatches()
{
	const int n_superpixels = sp_from.n_superpixels;

	for (int i = 0; i < n_superpixels; i++)
	{
		FindOneMatch(i,
			sp_from.sp_info[i].correspondence_id,
			sp_from.sp_info[i].best_cost,
			sp_from.sp_info[i].displacement);
		sp_from.sp_info[i].valid_disp = true;
	}
}

void Matcher::FindMatchesInv()
{
	const int n_superpixels = sp_to.n_superpixels;

	for (int i = 0; i < n_superpixels; i++)
	{
		FindOneMatchInv(i,
			sp_to.sp_info[i].correspondence_id,
			sp_to.sp_info[i].best_cost,
			sp_to.sp_info[i].displacement);
		sp_to.sp_info[i].valid_disp = true;
	}
}

cv::Mat Matcher::DrawLine(const int &from, const int &to)
{
	cv::Mat img_from = sp_from.dp.img.clone();
	cv::Mat img_to = sp_to.dp.img.clone();

	cvtColor(img_from, img_from, CV_Lab2BGR);
	cvtColor(img_to, img_to, CV_Lab2BGR);

	cv::Mat img = img_from;
	cv::hconcat(img, img_to, img);

	cv::Point2i pt1 = sp_from.sp_info[from].center;
	cv::Point2i pt2 = sp_to.sp_info[to].center;
	pt2.x += img_from.cols;

	cv::line(img, pt1, pt2, cv::Scalar(rand() % 255, rand() % 255, rand() % 255));
	cv::Scalar _color(rand() % 255, rand() % 255, rand() % 255);
	cv::circle(img, pt1, 3, _color);
	cv::circle(img, pt2, 3, _color);

	return img;
}

cv::Mat Matcher::DrawAllMatches(const std::vector<int> &ids)
{
	cv::Mat img_from = sp_from.dp.img.clone();
	cv::Mat img_to = sp_to.dp.img.clone();

	cvtColor(img_from, img_from, CV_Lab2BGR);
	cvtColor(img_to, img_to, CV_Lab2BGR);

	cv::Mat img = img_from;
	cv::hconcat(img, img_to, img);

	int count = 0;
	for (const auto &id_from : ids)
	{
		if (sp_from.sp_info[id_from].valid_disp == false) continue;

		cv::Point2i pt1 = sp_from.sp_info[id_from].center;
		cv::Point2i pt2 = sp_to.sp_info[sp_from.sp_info[id_from].correspondence_id].center;

		pt2.x += img_from.cols;
		cv::line(img, pt1, pt2, cv::Scalar(rand() % 255, rand() % 255, rand() % 255));
		cv::Scalar _color(rand() % 255, rand() % 255, rand() % 255);
		cv::circle(img, pt1, 3, _color);
		cv::circle(img, pt2, 3, _color);

		//cv::imwrite("TMP.tif", img);
		//cv::Mat tmp = DrawLine(id_from, sp_from.sp_info[id_from].correspondence_id);
		//std::stringstream buf;
		//buf << "./results/test3/TMP/" << count++ << ".tif";
		//cv::imwrite(buf.str(), tmp);
		//pause();
	}

	return img;
	//cv::imwrite("haber.tif", img);
}

cv::Mat Matcher::DrawAllMatchesInv(const std::vector<int> &ids)
{
	cv::Mat img_from = sp_to.dp.img.clone();
	cv::Mat img_to = sp_from.dp.img.clone();

	cvtColor(img_from, img_from, CV_Lab2BGR);
	cvtColor(img_to, img_to, CV_Lab2BGR);

	cv::Mat img = img_from;
	cv::hconcat(img, img_to, img);

	for (const auto &id_from : ids)
	{
		if (sp_to.sp_info[id_from].valid_disp == false) continue;

		cv::Point2i pt1 = sp_to.sp_info[id_from].center;
		cv::Point2i pt2 = sp_from.sp_info[sp_to.sp_info[id_from].correspondence_id].center;

		pt2.x += img_from.cols;
		cv::Scalar _color(rand() % 255, rand() % 255, rand() % 255);
		cv::circle(img, pt1, 3, _color);
		cv::circle(img, pt2, 3, _color);
	}

	return img;
}

void Matcher::DrawAllMatches(const int from, const int to)
{
	cv::Mat img_from = sp_from.dp.img.clone();
	cv::Mat img_to = sp_to.dp.img.clone();

	cvtColor(img_from, img_from, CV_Lab2BGR);
	cvtColor(img_to, img_to, CV_Lab2BGR);

	cv::Mat img = img_from;
	cv::hconcat(img, img_to, img);

	//const int n_superpixels = sp_from.n_superpixels;

	for (int id_from = from; id_from < to; id_from++)
	{
		//if (sp_from.sp_info[id_from].correspondence_id == -1) continue;
		if (sp_from.sp_info[id_from].valid_disp == false) continue;

		cv::Point2i pt1 = sp_from.sp_info[id_from].center;
		cv::Point2i pt2 = sp_to.sp_info[sp_from.sp_info[id_from].correspondence_id].center;

		pt2.x += img_from.cols;
		cv::line(img, pt1, pt2, cv::Scalar(rand() % 255, rand() % 255, rand() % 255));
	}

	cv::imwrite("haber.tif", img);
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

		if (sp_from.sp_info[label].correspondence_id == -1) continue;

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
	Stopwatch timer;
	FindMatches();
	//FindMatchesInv();

	RemoveAllOutliers();
	//RemoveAllOutliersInv();

	RemoveAllOutliersStep2(30, 2);

	timer.toc("total time: ");

	const int n_superpixels = sp_from.n_superpixels;
	std::vector<int> rn;

	for (int i = 0; i < n_superpixels; i ++)
	{
		if (sp_from.sp_info[i].valid_disp == false) continue;

		cv::Point2i pt1 = sp_from.sp_info[i].center;
		//if (136 < pt1.x && pt1.x < 460 && 105 < pt1.y && pt1.y < 350)
		//{
		//	rn.push_back(i);
		//}

		if (71 < pt1.x && pt1.x < 538 && 70 < pt1.y && pt1.y < 411)
		{
			rn.push_back(i);
		}
	}
	
	cv::Mat img1 = DrawAllMatches(rn);

	cv::imwrite("./results/testX/AllMatches.tif", img1);

}