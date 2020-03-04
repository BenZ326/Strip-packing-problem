#include "spp.h"
#include "datareader.h"
#include <algorithm>
#include <list>
#include <set>
/*
The paper section 2.1 (1) and (2)
t_items: all the items
t_idx: the item excluded (idx in the t_items)
flag = false, --> height 
flag = true, -->  width
*/


std::ostringstream item::ss;
item::item(const int t_idx, const int t_width, const int t_height) :idx(t_idx), width(t_width), height(t_height)
{
	subItems.clear();
}

std::list<int> computeFX(const int t_x,  const int t_idx,
	const std::vector<const item*>& t_items, bool flag)
{
	std::vector<std::vector<int>> res;
	for (size_t i = 0; i < t_items.size()-1; ++i)
	{
		std::vector<int> tmp(t_x+1, 0);
		res.push_back(tmp);
	}
	for (size_t j = 1; j <= t_x; ++j) res[0][j] = BigNumber;
	int itemIdx;
	for (size_t j = 0; j <= t_x; ++j)
	{
		for (size_t i = 1; i < t_items.size()-1; ++i)
		{
			if (i - 1 < t_idx) itemIdx = i - 1;
			else itemIdx = i;
			if (flag)
			{
				int diff = j - t_items[itemIdx]->width;
				if ((diff) < 0) res[i][j] = res[i - 1][j];
				else res[i][j] = std::min(res[i - 1][j], std::max(t_items[itemIdx]->height, res[i - 1][j - t_items[itemIdx]->width]));
			}
			else
			{
				int diff = j - t_items[itemIdx]->height;
				if ((diff) < 0) res[i][j] = res[i - 1][j];
				else res[i][j] = std::min(res[i - 1][j], std::max(t_items[itemIdx]->width, res[i - 1][j - t_items[itemIdx]->height]));
			}
		}
	}
	std::list<int> possiblePositions;
	for (size_t j = 0; j <= t_x; ++j)
	{
		if (res[res.size() - 1][j] < BigNumber) possiblePositions.push_back(j);
	}
	return possiblePositions;
}


int getMaximalHeight(const std::vector<const item*>& t_items)
{
	int res = -1;
	for (size_t i = 0; i < t_items.size(); ++i)
	{
		res = std::max(t_items[i]->height, res);
	}
	return res;
}


/*
Build the SPP model and solve
*/
double solve(const std::vector<const item*>& t_allItems, const std::map<int, std::list<int>>& t_mapPosWidth,
	const std::map<int, std::list<int>>& t_mapPosHeight)
{
	// data preparation
	std::set<int> allPositions;
	for (const auto& it : t_mapPosWidth)
		for (const auto& it2 : it.second)
			allPositions.insert(it2);
	IloEnv env;
	IloModel model(env);
	std::map<std::string, IloNumVar> allVars;
	// first constraints set
	for (const auto& it : t_allItems)
	{
		IloExpr expr(env);
		for (const auto& it2 : t_mapPosWidth.find(it->idx)->second)
		{
			auto varName = getVarName(it->idx, it2);
			IloNumVar var(env, 0, 1, ILOFLOAT, varName.c_str());
			allVars.insert(std::pair<std::string, IloNumVar>(varName, var));
			expr += var;
		}
		model.add(expr == 1);
		expr.end();
	}
	// second constraints set
	IloNumVar z(env, 0, IloInfinity, "ObjZ");
	for (const auto q : allPositions)
	{
		IloExpr expr(env);
		for (const auto it : t_allItems)
		{
			// calculate W(j, q)
			for (const auto& it2 : t_mapPosWidth.find(it->idx)->second)
			{
				if (it2 <= q && it2 >= q - it->width + 1)
				{
					auto iter = allVars.find(getVarName(it->idx, it2));
					assert(iter != allVars.end());
					expr += iter->second*it->height;
				}
			}
		}
		model.add(expr <= z);
		expr.end();
	}
	model.add(IloMinimize(env, z));
	IloCplex cplex(env);
	cplex.extract(model);
	cplex.setOut(env.getNullStream());
	cplex.setWarning(env.getNullStream());
	//cplex.exportModel("spp.lp");
	cplex.solve();
	double result = cplex.getObjValue();
	env.end();
	return result;
}



