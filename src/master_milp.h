#pragma once

#include <string>
#include <utility>
#include <vector>
#include "spp.h"

namespace StripPacking
{

enum class MasterSolveStatus
{
	optimal,
	infeasible,
	time_limit,
	error
};

struct MasterSolveResult
{
	MasterSolveStatus status = MasterSolveStatus::error;
	double objective = 0.0;
	std::vector<int> assignment; // start column per item index in input vector
};

MasterSolveResult solveMasterWithHiGHS(
	const std::vector<const item*>& t_items,
	const std::vector<std::vector<int>>& t_options,
	int t_stripWidth,
	const std::vector<std::pair<std::vector<std::pair<int, int>>, int>>& t_noGoodCuts,
	double t_timeLimitSeconds,
	bool t_integer);

}
