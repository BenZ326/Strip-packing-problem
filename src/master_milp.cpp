#include "master_milp.h"

#include <cstdlib>
#include <cmath>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <unistd.h>

namespace
{
std::string getHiGHSBinary()
{
	const char* envPath = std::getenv("HIGHS_BIN");
	if (envPath != nullptr && std::filesystem::exists(envPath))
	{
		return std::string(envPath);
	}
	const std::string vendored = "./third_party/highs/bin/highs";
	if (std::filesystem::exists(vendored))
	{
		return vendored;
	}
	return "highs";
}

std::string quotePath(const std::string& p)
{
	return "\"" + p + "\"";
}

} // namespace

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

	const std::string tag = std::to_string(::getpid()) + "_" + std::to_string(std::rand());
	const std::string modelPath = "/tmp/master_" + tag + ".lp";
	const std::string solPath = "/tmp/master_" + tag + ".sol";
	const std::string logPath = "/tmp/master_" + tag + ".log";

	std::ofstream lp(modelPath);
	if (!lp.is_open()) return result;

	lp << "Minimize\n";
	lp << " obj: z\n";
	lp << "Subject To\n";

	for (size_t i = 0; i < t_items.size(); ++i)
	{
		lp << " assign_" << i << ": ";
		bool first = true;
		for (int pos : t_options[i])
		{
			if (!first) lp << " + ";
			first = false;
			lp << "x_" << i << "_" << pos;
		}
		if (first)
		{
			lp << "0 = 1\n";
		}
		else
		{
			lp << " = 1\n";
		}
	}

	for (int q = 0; q < t_stripWidth; ++q)
	{
		lp << " load_" << q << ": ";
		bool first = true;
		for (size_t i = 0; i < t_items.size(); ++i)
		{
			for (int pos : t_options[i])
			{
				if (pos <= q && q < pos + t_items[i]->width)
				{
					if (!first) lp << " + ";
					first = false;
					lp << t_items[i]->height << " x_" << i << "_" << pos;
				}
			}
		}
		if (first)
		{
			lp << " - z <= 0\n";
		}
		else
		{
			lp << " - z <= 0\n";
		}
	}

	for (size_t c = 0; c < t_noGoodCuts.size(); ++c)
	{
		const auto& cut = t_noGoodCuts[c].first;
		const int rhs = t_noGoodCuts[c].second;
		if (cut.empty()) continue;
		lp << " cut_" << c << ": ";
		for (size_t k = 0; k < cut.size(); ++k)
		{
			if (k) lp << " + ";
			lp << "x_" << cut[k].first << "_" << cut[k].second;
		}
		lp << " <= " << rhs << "\n";
	}

	lp << "Bounds\n";
	lp << " 0 <= z\n";
	for (size_t i = 0; i < t_items.size(); ++i)
	{
		for (int pos : t_options[i])
		{
			lp << " 0 <= x_" << i << "_" << pos << " <= 1\n";
		}
	}

	if (t_integer)
	{
		lp << "Binary\n";
		for (size_t i = 0; i < t_items.size(); ++i)
		{
			for (int pos : t_options[i])
			{
				lp << " x_" << i << "_" << pos << "\n";
			}
		}
	}

	lp << "End\n";
	lp.close();

	const std::string highsBin = getHiGHSBinary();
	std::ostringstream cmd;
	cmd << quotePath(highsBin)
		<< " --model_file " << quotePath(modelPath)
		<< " --solution_file " << quotePath(solPath)
		<< " --time_limit " << t_timeLimitSeconds
		<< " > " << quotePath(logPath) << " 2>&1";
	const int rc = std::system(cmd.str().c_str());
	(void)rc;

	std::ifstream sol(solPath);
	if (!sol.is_open())
	{
		result.status = MasterSolveStatus::error;
		std::filesystem::remove(modelPath);
		std::filesystem::remove(logPath);
		return result;
	}

	std::string line;
	bool inColumns = false;
	std::string modelStatus;
	while (std::getline(sol, line))
	{
		if (line == "Model status")
		{
			if (std::getline(sol, modelStatus))
			{
				if (modelStatus.find("Optimal") != std::string::npos)
				{
					result.status = MasterSolveStatus::optimal;
				}
				else if (modelStatus.find("Infeasible") != std::string::npos)
				{
					result.status = MasterSolveStatus::infeasible;
				}
				else if (modelStatus.find("Time") != std::string::npos || modelStatus.find("limit") != std::string::npos)
				{
					result.status = MasterSolveStatus::time_limit;
				}
			}
			continue;
		}
		if (line.rfind("Objective ", 0) == 0)
		{
			result.objective = std::stod(line.substr(std::string("Objective ").size()));
			continue;
		}
		if (line.rfind("# Columns", 0) == 0)
		{
			inColumns = true;
			continue;
		}
		if (line.rfind("# Rows", 0) == 0)
		{
			inColumns = false;
			continue;
		}
		if (inColumns)
		{
			std::istringstream iss(line);
			std::string varName;
			double value = 0.0;
			if (!(iss >> varName >> value)) continue;
			if (varName == "z") continue;
			if (value < 0.5) continue;
			// x_i_pos
			if (varName.rfind("x_", 0) == 0)
			{
				size_t p1 = varName.find('_', 2);
				if (p1 == std::string::npos) continue;
				const int itemIdx = std::stoi(varName.substr(2, p1 - 2));
				const int pos = std::stoi(varName.substr(p1 + 1));
				if (0 <= itemIdx && itemIdx < static_cast<int>(result.assignment.size()))
				{
					result.assignment[itemIdx] = pos;
				}
			}
		}
	}
	sol.close();

	std::filesystem::remove(modelPath);
	std::filesystem::remove(solPath);
	std::filesystem::remove(logPath);
	if (result.status == MasterSolveStatus::error && modelStatus.empty())
	{
		result.status = MasterSolveStatus::time_limit;
	}
	return result;
}
