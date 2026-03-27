#include "master_milp.h"

#include <cmath>
#include <algorithm>
#include <limits>

#include "highs/Highs.h"

namespace {
bool highsOk(const HighsStatus status) {
	return status == HighsStatus::kOk || status == HighsStatus::kWarning;
}
}

StripPacking::MasterSolveResult StripPacking::solveMasterWithHiGHS(
	const std::vector<const item*>& t_items,
	const std::vector<std::vector<int>>& t_options,
	int t_stripWidth,
	const std::vector<std::pair<std::vector<std::pair<int, int>>, int>>& t_noGoodCuts,
	double t_timeLimitSeconds,
	bool t_integer)
{
	MasterSolveResult result;
	result.assignment.assign(t_items.size(), -1);
	if (t_items.empty())
	{
		result.status = MasterSolveStatus::optimal;
		result.objective = 0.0;
		return result;
	}

	Highs highs;
	if (!highsOk(highs.setOptionValue("output_flag", false)))
	{
		return result;
	}
	if (t_timeLimitSeconds > 0.0)
	{
		if (!highsOk(highs.setOptionValue("time_limit", t_timeLimitSeconds)))
		{
			return result;
		}
	}
	if (!highsOk(highs.changeObjectiveSense(ObjSense::kMinimize)))
	{
		return result;
	}

	// Add z first: minimize z with 0 <= z.
	if (!highsOk(highs.addCol(1.0, 0.0, kHighsInf, 0, nullptr, nullptr)))
	{
		return result;
	}
	const HighsInt zCol = 0;

	std::vector<std::vector<HighsInt>> xColIndex(t_items.size());
	for (size_t i = 0; i < t_items.size(); ++i)
	{
		xColIndex[i].reserve(t_options[i].size());
		for (size_t k = 0; k < t_options[i].size(); ++k)
		{
			if (!highsOk(highs.addCol(0.0, 0.0, 1.0, 0, nullptr, nullptr)))
			{
				return result;
			}
			const HighsInt colIndex = static_cast<HighsInt>(highs.getNumCol() - 1);
			xColIndex[i].push_back(colIndex);
			if (t_integer)
			{
				if (!highsOk(highs.changeColIntegrality(colIndex, HighsVarType::kInteger)))
				{
					return result;
				}
			}
		}
	}

	// Assignment rows: sum_pos x_i_pos = 1
	for (size_t i = 0; i < t_items.size(); ++i)
	{
		std::vector<HighsInt> indices;
		std::vector<double> values;
		indices.reserve(t_options[i].size());
		values.reserve(t_options[i].size());
		for (size_t k = 0; k < t_options[i].size(); ++k)
		{
			indices.push_back(xColIndex[i][k]);
			values.push_back(1.0);
		}
		if (!highsOk(highs.addRow(1.0, 1.0,
			static_cast<HighsInt>(indices.size()), indices.data(), values.data())))
		{
			return result;
		}
	}

	// Load rows: sum_i,pos h_i * x_i_pos - z <= 0, for each q.
	for (int q = 0; q < t_stripWidth; ++q)
	{
		std::vector<HighsInt> indices;
		std::vector<double> values;
		indices.push_back(zCol);
		values.push_back(-1.0);
		for (size_t i = 0; i < t_items.size(); ++i)
		{
			for (size_t k = 0; k < t_options[i].size(); ++k)
			{
				const int pos = t_options[i][k];
				if (pos <= q && q < pos + t_items[i]->width)
				{
					indices.push_back(xColIndex[i][k]);
					values.push_back(static_cast<double>(t_items[i]->height));
				}
			}
		}
		if (!highsOk(highs.addRow(-kHighsInf, 0.0,
			static_cast<HighsInt>(indices.size()), indices.data(), values.data())))
		{
			return result;
		}
	}

	// No-good cuts: sum selected x <= rhs
	for (size_t c = 0; c < t_noGoodCuts.size(); ++c)
	{
		const auto& cut = t_noGoodCuts[c].first;
		const int rhs = t_noGoodCuts[c].second;
		if (cut.empty()) continue;
		std::vector<HighsInt> indices;
		std::vector<double> values;
		indices.reserve(cut.size());
		values.reserve(cut.size());
		for (size_t k = 0; k < cut.size(); ++k)
		{
			const int itemIdx = cut[k].first;
			const int pos = cut[k].second;
			if (itemIdx < 0 || itemIdx >= static_cast<int>(t_options.size())) continue;
			const auto posIt = std::find(t_options[itemIdx].begin(), t_options[itemIdx].end(), pos);
			if (posIt == t_options[itemIdx].end()) continue;
			const size_t optionIndex = static_cast<size_t>(std::distance(t_options[itemIdx].begin(), posIt));
			indices.push_back(xColIndex[itemIdx][optionIndex]);
			values.push_back(1.0);
		}
		if (indices.empty()) continue;
		if (!highsOk(highs.addRow(-kHighsInf, static_cast<double>(rhs),
			static_cast<HighsInt>(indices.size()), indices.data(), values.data())))
		{
			return result;
		}
	}

	if (!highsOk(highs.run()))
	{
		return result;
	}

	const auto modelStatus = highs.getModelStatus();
	if (modelStatus == HighsModelStatus::kOptimal)
	{
		result.status = MasterSolveStatus::optimal;
	}
	else if (modelStatus == HighsModelStatus::kInfeasible)
	{
		result.status = MasterSolveStatus::infeasible;
		return result;
	}
	else if (modelStatus == HighsModelStatus::kTimeLimit ||
		modelStatus == HighsModelStatus::kIterationLimit)
	{
		result.status = MasterSolveStatus::time_limit;
		return result;
	}
	else
	{
		result.status = MasterSolveStatus::error;
		return result;
	}

	result.objective = highs.getObjectiveValue();
	const auto& solution = highs.getSolution();
	if (solution.col_value.size() != static_cast<size_t>(highs.getNumCol()))
	{
		result.status = MasterSolveStatus::error;
		return result;
	}
	for (size_t i = 0; i < t_items.size(); ++i)
	{
		for (size_t k = 0; k < t_options[i].size(); ++k)
		{
			if (solution.col_value[xColIndex[i][k]] >= 0.5)
			{
				result.assignment[i] = t_options[i][k];
				break;
			}
		}
	}
	return result;
}
