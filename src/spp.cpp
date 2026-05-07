#include "spp.h"
#include "master_milp.h"
#include <algorithm>
#include <list>
#include <set>
#include <functional>
#include <limits>
#include <cmath>

/*
The paper section 2.1 (1) and (2)
t_items: all the items
t_idx: the item excluded (idx in the t_items)
flag = false, --> height 
flag = true, -->  width
*/


std::ostringstream StripPacking::item::ss;
StripPacking::item::item(const int t_idx, const int t_width, const int t_height) :idx(t_idx), origIdx(t_idx), width(t_width), height(t_height)
{
	subItems.clear();
}

StripPacking::item::item(const int t_idx, const int t_width, const int t_height, const int t_idxHelper) : idx(t_idx), origIdx(t_idx), width(t_width), height(t_height),
idxHelper(t_idxHelper)
{
	subItems.clear();
}

std::set<int> StripPacking::computeFX(const int t_x,  const int t_idx,
	const std::vector<const StripPacking::item*>& t_items, bool flag)
{
	if (t_x < 0 || t_items.empty())
	{
		return std::set<int>();
	}
	std::vector<std::vector<int>> res;
	for (size_t i = 0; i < t_items.size(); ++i)
	{
		std::vector<int> tmp(static_cast<size_t>(t_x + 1), 0);
		res.push_back(tmp);
	}
	for (int j = 1; j <= t_x; ++j) res[0][static_cast<size_t>(j)] = 0;
	for (size_t i = 0; i < t_items.size(); ++i) res[i][0] = 1;
	int itemIdx;
	for (int j = 1; j <= t_x; ++j)		// W 
	{
		for (size_t i = 1; i < t_items.size(); ++i)
		{
			if (i - 1 < t_idx) itemIdx = i - 1;
			else itemIdx = i;
			if (flag)
			{
				int diff = j - t_items[itemIdx]->width;
				if ((diff) < 0) res[i][static_cast<size_t>(j)] = res[i - 1][static_cast<size_t>(j)];
				else res[i][static_cast<size_t>(j)] = std::max(res[i - 1][static_cast<size_t>(j)], res[i - 1][static_cast<size_t>(j - t_items[itemIdx]->width)]);
			}
			else
			{
				int diff = j - t_items[itemIdx]->height;
				if ((diff) < 0) res[i][static_cast<size_t>(j)] = res[i - 1][static_cast<size_t>(j)];
				else res[i][static_cast<size_t>(j)] = std::max(res[i - 1][static_cast<size_t>(j)], res[i - 1][static_cast<size_t>(j - t_items[itemIdx]->height)]);
			}
		}
	}
	std::set<int> possiblePositions;
	for (int j = 0; j <= t_x; ++j)
	{
		if (res[res.size() - 1][static_cast<size_t>(j)]  == 1) possiblePositions.insert(j);
	}
	return possiblePositions;
}


std::set<int> StripPacking::computeFX(const int t_x, const int t_idx,
	const std::vector<StripPacking::item*>& t_items, bool flag)
{
	if (t_x < 0 || t_items.empty())
	{
		return std::set<int>();
	}
	std::vector<std::vector<int>> res;
	for (size_t i = 0; i < t_items.size(); ++i)
	{
		std::vector<int> tmp(static_cast<size_t>(t_x + 1), 0);
		res.push_back(tmp);
	}
	for (int j = 1; j <= t_x; ++j) res[0][static_cast<size_t>(j)] = 0;
	for (size_t i = 0; i < t_items.size(); ++i) res[i][0] = 1;
	int itemIdx;
	for (int j = 1; j <= t_x; ++j)		// W 
	{
		for (size_t i = 1; i < t_items.size(); ++i)
		{
			if (i - 1 < t_idx) itemIdx = i - 1;
			else itemIdx = i;
			if (flag)
			{
				int diff = j - t_items[itemIdx]->width;
				if ((diff) < 0) res[i][static_cast<size_t>(j)] = res[i - 1][static_cast<size_t>(j)];
				else res[i][static_cast<size_t>(j)] = std::max(res[i - 1][static_cast<size_t>(j)], res[i - 1][static_cast<size_t>(j - t_items[itemIdx]->width)]);
			}
			else
			{
				int diff = j - t_items[itemIdx]->height;
				if ((diff) < 0) res[i][static_cast<size_t>(j)] = res[i - 1][static_cast<size_t>(j)];
				else res[i][static_cast<size_t>(j)] = std::max(res[i - 1][static_cast<size_t>(j)], res[i - 1][static_cast<size_t>(j - t_items[itemIdx]->height)]);
			}
		}
	}
	std::set<int> possiblePositions;
	for (int j = 0; j <= t_x; ++j)
	{
		if (res[res.size() - 1][static_cast<size_t>(j)] == 1) possiblePositions.insert(j);
	}
	return possiblePositions;
}


int StripPacking::getMaximalHeight(const std::vector<const StripPacking::item*>& t_items)
{
	int res = -1;
	for (size_t i = 0; i < t_items.size(); ++i)
	{
		res = std::max(t_items[i]->height, res);
	}
	return res;
}


/*
Build the contiguity parallel machine scheduling problem as a lower bound for the spp
*/
double StripPacking::solve(const std::vector<const StripPacking::item*>& t_allItems, const std::map<int, std::set<int>>& t_mapPosWidth,
	const std::map<int, std::set<int>>& t_mapPosHeight, const bool t_Integer)
{
	(void)t_mapPosHeight;
	(void)t_Integer;
	std::set<int> allPositions;
	for (const auto& it : t_mapPosWidth)
		for (const auto& it2 : it.second)
			allPositions.insert(it2);
	int stripWidth = 0;
	std::vector<std::vector<int>> options(t_allItems.size());
	for (size_t i = 0; i < t_allItems.size(); ++i)
	{
		const auto* it = t_allItems[i];
		const auto& possible = t_mapPosWidth.find(it->idx)->second;
		options[i].assign(possible.begin(), possible.end());
		std::sort(options[i].begin(), options[i].end());
		if (!options[i].empty())
		{
			stripWidth = std::max(stripWidth, options[i].back() + it->width);
		}
	}
	if (stripWidth == 0) return 0.0;
	const auto milpResult = StripPacking::solveMasterWithHiGHS(
		t_allItems,
		options,
		stripWidth,
		std::vector<std::pair<std::vector<std::pair<int, int>>, int>>(),
		60.0,
		t_Integer);
	if (milpResult.status == StripPacking::MasterSolveStatus::optimal)
	{
		return milpResult.objective;
	}
	if (milpResult.status == StripPacking::MasterSolveStatus::infeasible)
	{
		return BigNumber;
	}

	std::vector<int> loads(stripWidth, 0);
	std::vector<int> order(t_allItems.size(), 0);
	for (size_t i = 0; i < t_allItems.size(); ++i) order[i] = static_cast<int>(i);
	std::sort(order.begin(), order.end(), [&](int lhs, int rhs) {
		if (options[lhs].size() != options[rhs].size()) return options[lhs].size() < options[rhs].size();
		return t_allItems[lhs]->width > t_allItems[rhs]->width;
	});
	std::vector<int> suffixArea(order.size() + 1, 0);
	for (int i = static_cast<int>(order.size()) - 1; i >= 0; --i)
	{
		const auto* it = t_allItems[order[static_cast<size_t>(i)]];
		suffixArea[static_cast<size_t>(i)] = suffixArea[static_cast<size_t>(i + 1)] + it->width * it->height;
	}
	int bestObj = std::numeric_limits<int>::max();

	std::function<void(size_t, int, int)> dfs = [&](size_t depth, int currentMax, int usedArea) {
		if (currentMax >= bestObj) return;
		const int avgLb = static_cast<int>(std::ceil((usedArea + suffixArea[depth]) / static_cast<double>(stripWidth)));
		if (std::max(currentMax, avgLb) >= bestObj) return;
		if (depth == order.size())
		{
			bestObj = currentMax;
			return;
		}
		const int itemIdx = order[depth];
		const auto* itemPtr = t_allItems[itemIdx];
		for (int pos : options[itemIdx])
		{
			int nextMax = currentMax;
			for (int col = pos; col < pos + itemPtr->width; ++col)
			{
				loads[col] += itemPtr->height;
				nextMax = std::max(nextMax, loads[col]);
			}
			dfs(depth + 1, nextMax, usedArea + itemPtr->width * itemPtr->height);
			for (int col = pos; col < pos + itemPtr->width; ++col)
			{
				loads[col] -= itemPtr->height;
			}
		}
	};
	dfs(0, 0, 0);
	return (bestObj == std::numeric_limits<int>::max()) ? BigNumber : bestObj;
}
