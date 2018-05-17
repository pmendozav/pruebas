#pragma once
#include "Superpixel.h"

class Matcher
{
private:
	Superpixel sp_from;
	Superpixel sp_to;

	//std::vector < std::pair<int, int>> matches;

	std::set<int> FindNeighs(const std::vector<SP> &sp_info,
		const std::set<int> &from);

	float MatchCost(const cv::Mat &dp1, const cv::Mat &dp2);
	float MatchCostColor(const cv::Vec3f &color1, const cv::Vec3f &color2);

	bool CheckOneMatch(const std::vector<cv::Point2i> &disp_neighs,
		const cv::Point2i &from);
	void RemoveAllOutliers();
	void RemoveAllOutliersInv();

	bool RemoveOneOutliersStep2(const int &id,
		const int &disp_max,
		const int &count_points,
		const int &max_neighs,
		const int &max_iters);
	void RemoveAllOutliersStep2(const int &disp_max = 50,
		const int &count_points = 1,
		const int &max_neighs = 6,
		const int &max_iters = 5);

	void Check1d(const std::vector<cv::Point2i> &disps,
		std::vector<bool> &flags,
		const int &dim);
public:
	Matcher();
	~Matcher();

	void Set(Superpixel &sp_from, Superpixel &sp_to);

	void FindOneMatch(const int &id_from,
		int &id_to,
		float &best_cost,
		cv::Point2i &displacement);
	void FindOneMatchInv(const int &id_from,
		int &id_to,
		float &best_cost,
		cv::Point2i &displacement);

	void FindMatches();
	void FindMatchesInv();

	void Debug(std::string str = "");
	void DebugNeighs(int id_from, std::set<int> neighs);

	void DrawAllMatches(const int from = 0, const int to = 10);
	cv::Mat DrawAllMatches(const std::vector<int> &ids);
	cv::Mat DrawAllMatchesInv(const std::vector<int> &ids);
	void DrawMatches(const std::set<int> neighs);
	void DrawMatches(const std::vector < std::pair<int, int>> &matches);

	cv::Mat DrawLine(const int &from, const int &to);
};

