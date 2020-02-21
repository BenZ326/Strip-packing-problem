#include "BLEU.h"
#include <algorithm>
#include "util.h"
#include "knapsack.h"

double BLEU::tolerance = 0.0001;

BLEU::BLEU(const std::vector<const item*>& t_items, const int t_W)
	:_allItems(t_items),_W(t_W)
{
	// sort the items by the nonincreasing of width and breaking ties by nonincreasing height
	std::sort(_allItems.begin(), _allItems.end(), compareItemByWidth);
}


/*
Cut all items into pieces along the direction of height
*/
void BLEU::cutItemsAlongHeight()
{
	int idx = 0;
	for (const auto& it : _allItems)
	{
		for (size_t i = 1; i <= it->height; ++i)
		{
			const itemPieceWidth* tmp = new itemPieceWidth(idx++, it->width);
			_allItemPiecesWidths.push_back(tmp);
		}
	}
}

/*
5.1 the three preprocessing procedures
*/
void BLEU::preprocessing()
{
	this->preprocessingFixItems();
	this->preprocessingReduceW();
	this->preprocessingModifyItemWidth();
}

/*
An exact algorithm for the two-dimensional strip packing problem 2010 by Marco Antonio Boschetti, Lorenza Montaletti
Section 2.2.1
find a subset of items containing all items that cannot be placed side by side with any other items
*/
void BLEU::preprocessingFixItems()
{
	// find the item with the minimal width
	int minWidth = _allItems.back()->width;
	int sumHeight = 0;
	for (const auto& it : _allItems)
	{
		if (it->width + minWidth > _W)
			sumHeight += it->height;
		else _processedItems.push_back(it);
	}
	_processedH = sumHeight;				// store the occupied height
}

/*
Calculate the maximal width that a subset of items can be packed side by side if the maximal width is strictly less than W,
than W = the maximal width
*/
void BLEU::preprocessingReduceW()
{
	for (const auto& it : _processedItems)
		_allWidths.push_back(it->width);
	_processedW = subSetSum(_allWidths,  _W);
}

/*
modify width for items according to 2.2.3 in the paper "An exact algorithm for the two-dimensional strip packing problem"
*/
void BLEU::preprocessingModifyItemWidth()
{
	for (std::vector<const item*>::iterator i_it = _processedItems.begin(); i_it != _processedItems.end(); ++i_it)
	{
		std::vector<int> integers;
		for (auto & j_it : _processedItems)
		{
			if ((*i_it)->idx == j_it->idx) continue;
			integers.push_back(j_it->width);
		}
		int maxWidth = subSetSum(integers, _processedW - (*i_it)->width) + (*i_it)->width;
		if (maxWidth < _processedW) 
			const_cast<item*>((*i_it))->width = (*i_it)->width + _processedW - maxWidth;
	}
}

/*
section 5.1 lower bounds plus upper bounds
*/
void BLEU::bounds()
{
	std::cout<<"Lower bound 1 is "<<this->LowerBound1()<<" ";
	std::cout<<"Lower bound 2 is "<<this->LowerBound2()<< " ";
	std::cout << "Lower bound 4 is " << this->LowerBound4()<< " "<<std::endl;
}

int BLEU::LowerBound1()
{
	int sum = 0;
	for (const auto& it : _allItems)
	{
		sum += it->height*it->width;
	}
	return int(std::ceil(sum / _W));
}

/*
Section 3.2 in the paper "An exact algorithm for the two-dimensional strip packing problem" 
dual feasible functions 1, 2 ,3
For understanding the concept of dual feasible functions, refer to the paper : "New  classes of fast lower  bounds  for  bin  packingproblems"
*/
int BLEU::LowerBound2()
{
	//dual feasible function 1:
	int lowerBound = 0;
	for (size_t alpha = 1; alpha <= _processedW; ++alpha)
	{
		std::vector<double> allTransformedWidths;
		for (const auto& it : _processedItems) allTransformedWidths.push_back(this->DualFeasibleFunction1(alpha,it->width));
		double sum = 0.0;
		for (size_t i = 0; i < _processedItems.size(); i++)		sum += allTransformedWidths[i] * _processedItems[i]->height;
		if (lowerBound < std::ceil(sum / _processedW))
			lowerBound = std::ceil(sum / _processedW);
	}
	 //dual feasible function 2:
	for (size_t alpha = 1; alpha <= std::floor(_processedW/2); ++alpha)
	{
		std::vector<double> allTransformedWidths;
		for (const auto& it : _processedItems) 
			allTransformedWidths.push_back(this->DualFeasibleFunction2(alpha, it->width));
		double sum = 0.0;
		for (size_t i = 0; i < _processedItems.size(); i++)		sum += allTransformedWidths[i] * _processedItems[i]->height;
		auto candidate = std::ceil(sum / this->DualFeasibleFunction2(alpha, _processedW));
		if (lowerBound < candidate)
			lowerBound = candidate;
	}

	// dual feasible function 3:
	for (size_t alpha = 1; alpha <= std::floor(_processedW / 2); ++alpha)
	{
		std::vector<double> allTransformedWidths;
		for (const auto& it : _processedItems)
			allTransformedWidths.push_back(this->DualFeasibleFunction3(alpha, it->width));
		double sum = 0.0;
		for (size_t i = 0; i < _processedItems.size(); i++)		sum += allTransformedWidths[i] * _processedItems[i]->height;
		auto candidate = std::ceil(sum / this->DualFeasibleFunction3(alpha, _processedW));
		if (lowerBound < candidate)
			lowerBound = candidate;
	}
	return lowerBound + _processedH;
}

/*
Section 5.1 
Solve a noncontiguous packing problem (NCBP)
*/
int BLEU::LowerBound4()
{
	std::vector<int> allWidths;
	int colIdx = 0;
	// initialize columns
	std::list<std::set<int>> columns;			// a collection of columns		a column is a set of item index
	for (size_t i=0; i< _processedItems.size(); ++i)
	{
		std::set<int> tmp;
		tmp.insert(_processedItems[i]->idx);
		columns.push_back(tmp);
		allWidths.push_back(_processedItems[i]->width);
	}
	// model the LP
	IloEnv env;
	IloModel NCBP(env);
	std::map<int, IloRange> allCstrs;
	// add constraints
	for (size_t i = 0; i < _processedItems.size(); ++i)
	{
		char nameCstr[64];
		sprintf_s(nameCstr, 64, "Cstr %d", _processedItems[i]->idx);
		IloExpr expr(env);
		IloRange cstr(env, _processedItems[i]->height, expr, IloInfinity, nameCstr);
		allCstrs.insert(std::pair<int, IloRange>(_processedItems[i]->idx, cstr));
		NCBP.add(cstr);
		expr.end();
	}
	// add the variables and the objective function
	IloExpr obj(env);
	for (const auto& i_it : columns)
	{
		IloNumColumn col(env);
		for (const auto& j_it : i_it)
		{
			auto ptr = allCstrs.find(j_it);
			if (ptr != allCstrs.end()) col += ptr->second(1);
		}
		char varName[64];
		sprintf_s(varName, 64, "Var %d", colIdx++);
		IloNumVar var(col);
		var.setName(varName);
		var.setBounds(0, +IloInfinity);
		NCBP.add(var);
		col.end();
		obj += var;
	}
	IloObjective realObj = IloMinimize(env, obj);
	NCBP.add(realObj);
	obj.end();
	try {
		IloCplex cplex(NCBP);
		cplex.setOut(env.getNullStream());
		cplex.setWarning(env.getNullStream());
		while (true)
		{
			cplex.solve();
			cplex.exportModel("NCBP.lp");
			// solve the pricing problem
			std::vector<IloNum> dualValues;
			for (size_t i=0; i<_processedItems.size(); ++i)
				dualValues.push_back(cplex.getDual(allCstrs.find(_processedItems[i]->idx)->second));
			std::vector<int> selectedItems;
			double value = dynamicPrg4KnapSack(dualValues, allWidths, _processedW, selectedItems);
			if (1.0 - value < - BLEU::tolerance)		// negative reduced cost
			{
				// add a column
				std::set<int> newCol;
				for (const auto& it : selectedItems) newCol.insert(_processedItems[it]->idx);
				IloNumColumn col(env);
				for (const auto& it : newCol)
				{
					auto ptr = allCstrs.find(it);
					if (ptr != allCstrs.end()) 
						col += ptr->second(1);
				}
				char varName[64];
				sprintf_s(varName, 64, "Var %d", colIdx++);
				IloNumVar var(col);
				var.setName(varName);
				var.setBounds(0, +IloInfinity);
				NCBP.add(var);
				realObj.setLinearCoef(var, 1.0);
			}
			else
			{
				double objValue = cplex.getObjValue();
				env.end();
				double lowerBound;
				std::modf(objValue, &lowerBound);
				if (objValue - lowerBound > BLEU::tolerance)
					return lowerBound + 1 + _processedH;
				else
					return lowerBound + _processedH;	
			}
		}
		// add columns
		// return the solution
	}
	catch (IloException & e)
	{
		std::cout << "NCBP error\n";
		std::cout << e<< std::endl;
		return -1;
	}
	
}

const double BLEU::DualFeasibleFunction1(const int t_alpha, const int t_width) const
{
	double tmp = (t_alpha + 1.0)*((double(t_width) / _processedW));
	double intPart;
	if (std::modf(tmp, &intPart) < BLEU::tolerance) // if tmp is an integer
		return (std::round(tmp));
	else
		return (std::floor(tmp)*(_processedW / double(t_alpha)));
}


const double BLEU::DualFeasibleFunction2(const int t_alpha, const int t_width) const
{
	if (t_width > _processedW - t_alpha)
		return _processedW;
	else
	{
		if (t_alpha <= t_width && t_width <= _processedW - t_alpha)
			return t_width;
		else return 0;
	}
}

const	 double BLEU::DualFeasibleFunction3(const int t_alpha, const int t_width) const
{
	if (t_width > (_processedW / 2.0))
		return 2 * (std::floor(_processedW / double(t_alpha)) - std::floor((_processedW - t_width) / double(t_alpha)));
	if (std::abs(t_width - (_processedW / 2.0)) < BLEU::tolerance)
		return std::floor(_processedW / double(t_alpha));
	if (t_width < (_processedW / 2.0))
		return 2 * std::floor(t_width / double(t_alpha));
}