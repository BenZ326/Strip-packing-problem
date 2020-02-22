#include "BLEU.h"
#include <algorithm>
#include "util.h"
#include "spp.h"
#include "knapsack.h"
#include <iostream>
#include <stack>


double BLEU::tolerance = 0.0001;
int BLEU::bigNumber = 999999;
int BLEU::BBMaxExplNodesPerPack = 10000000;
int BLEU::BBMaxExplNodesNonPerPack = 50000;

BLEU::BLEU(const std::vector<const item*>& t_items, const int t_W, const int t_TrialHeight)
	:_allItems(t_items),_W(t_W), _trialHeight(t_TrialHeight)
{
	// sort the items by the nonincreasing of width and breaking ties by nonincreasing height
	std::sort(_allItems.begin(), _allItems.end(), compareItemByWidth);
}

void BLEU::takeOff()
{
}


/*
the y-check algorithm ---------------------------------------------------------------------------------------start
*/
/*
Arguements:
t_processedW: the width of the strip
t_TrialHeight: the tempting height
itemPositions: the left-bottom coordinates of all the items
t_processedItems: all the items to be packed in the strip
Return: if it is a feasible solution for the SPP then return true, else false
*/
bool BLEU::yCheckAlgorithm(const int t_processedW, const int t_TrialHeight, const std::vector<coordinate>& itemPositions,
	const std::vector<const item*> t_processedItems) const

{
	return false;
}
/*
the y-check algorithm ---------------------------------------------------------------------------------------end
*/


//The branch and bound algorithm -----------------------------------------------------------------------------start
/*

*/
BLEU::BBNode::BBNode(const std::vector<const item*>& t_remainingItems, const int t_Width, const int t_TrialHeight):
remainingItems(t_remainingItems), trialHeight(t_TrialHeight)
{
	leftMostIdx = 0;
	columnsOccupiedHeight = std::vector<int>(0, t_Width);			// the order is consistent to the left most column -> the right most column
	maxiItemIdxColumns = std::vector<int>(0, t_Width);				// the order is consistent to the left most column -> the right most column
}

BLEU::BBNode::BBNode(const BBNode& t_BBNode):trialHeight(t_BBNode.trialHeight)
{
	leftMostIdx = t_BBNode.leftMostIdx;
	columnsOccupiedHeight = t_BBNode.columnsOccupiedHeight;			// [10,5,3,2] means 10 units of height in the 1st column is occupied and 5 units for the 2nd column...
	remainingItems = t_BBNode.remainingItems;			// make heap
	maxiItemIdxColumns = t_BBNode.maxiItemIdxColumns;				// [3,5,1,7] means among items placed in 1st column, 3 is the largest index of...
	itemPositions = t_BBNode.itemPositions;
}

const bool BLEU::bounding(const std::unique_ptr<BBNode>& t_currentNode) const
{

}

void  BLEU::makeBranch(const std::unique_ptr<BBNode>& t_currentNode,
	const std::stack<std::unique_ptr<BBNode>>& t_dfstree) const
{

}

const solutionStatus BLEU::branchAndBound() const
{
	std::unique_ptr<BBNode>	root(new BBNode(_processedItems, _processedW, _trialHeight));
	std::stack<std::unique_ptr<BBNode>> dfsTree;
	dfsTree.push(root);
	int maxExpNodes;
	if (this->LowerBound1() == root->trialHeight) maxExpNodes = BLEU::BBMaxExplNodesPerPack;
	else maxExpNodes = BLEU::BBMaxExplNodesNonPerPack;
	int numberExploredNodes = 0;
	while (!dfsTree.empty() && numberExploredNodes <= maxExpNodes)
	{
		const auto currentNode = std::move(dfsTree.top());
		dfsTree.pop();
		numberExploredNodes++;
		// if it's a feasible solution then invoke the y-check algorithm
		if (currentNode->remainingItems.empty())
		{
			if (this->yCheckAlgorithm(_processedW, _trialHeight, currentNode->itemPositions, _processedItems))
				return solutionStatus::feasible;
			else continue;							// the node can not be transformed to a feasible solution for the SPP
		}
		else
		{
			// bounding the current Node
			if (this->bounding(currentNode))
				continue;
			// make branch
			this->makeBranch(currentNode, dfsTree);
		}
	}
}



//The branch and bound algorithm -----------------------------------------------------------------------------end

const solutionStatus BLEU::combianotrialBenders() const
{


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
	int lb1 = this->LowerBound1();
	int lb2 = this->LowerBound2();
	int lb4 = this->LowerBound4();
	int lb5 = this->LowerBound5();
	_bestLowerBound = std::max(lb1, lb2);
	int lb3 = this->LowerBound3();
	_bestLowerBound = std::max({ _bestLowerBound, lb3, lb4, lb5 });
	std::cout << "Lower bound is " << _bestLowerBound << std::endl;
}

const int BLEU::LowerBound1() const
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
const int BLEU::LowerBound2() const
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
The lower bound proposed in the paper:
Alvarez-Vales R, Parreno F, Tamarit JM. A branch and bound algorithm for the strip packing problem. OR spectrum. 
2009 Apr 1;31(2):431-59.
Section 4.2.6
*/
const int BLEU::LowerBound3() const
{
	// step 1:
	bool exitFlag = false;
	int result;
	for (int k = 0; !exitFlag; k++)
	{
		int RectangleW = _processedW;
		int RectangleH = _bestLowerBound + k;
		std::vector<item*> itemsArr;
		for (const auto & it : _processedItems)
		{
			item*  tmp = new item(*it);
			itemsArr.push_back(tmp);
		}
		while (true)
		{
			std::make_heap(itemsArr.begin(), itemsArr.end(), compareItemByWHDifference(RectangleH, RectangleW));
			std::pop_heap(itemsArr.begin(), itemsArr.end(), compareItemByWHDifference(RectangleH, RectangleW));
			item* selectedItem = itemsArr.back();
			itemsArr.pop_back();
			// check if the item can fit the rectangle 
			if (selectedItem->height > RectangleH && selectedItem->width > RectangleW) break;
			std::set<int> removedItems;
			// step 2 and step 3
			if (RectangleW - selectedItem->width <= RectangleH - selectedItem->height)
			{
				// pack the item at the bottom and update the rectangle and width of some items
				RectangleH -= selectedItem->height;
				for (size_t i = 0; i < itemsArr.size(); ++i)
				{
					if (itemsArr[i]->width <= RectangleW - selectedItem->width)
					{
						itemsArr[i]->height = std::max(0, itemsArr[i]->height - selectedItem->height);
						if (itemsArr[i]->height == 0)
							removedItems.insert(itemsArr[i]->idx);
					}
				}
			}
			else
			{
				// pack the item at the left
				RectangleW -= selectedItem->width;
				for (size_t i = 0; i < itemsArr.size(); ++i)
				{
					if (itemsArr[i]->height <= RectangleH - selectedItem->height)
					{
						itemsArr[i]->width = std::max(0, itemsArr[i]->width - selectedItem->width);
						if (itemsArr[i]->width == 0)
							removedItems.insert(itemsArr[i]->idx);
					}
				}
			}
			std::vector<item*> remainingItemsArr;
			for (size_t i = 0; i < itemsArr.size(); ++i)
			{
				if (removedItems.find(itemsArr[i]->idx) == removedItems.end()) 	remainingItemsArr.push_back(itemsArr[i]);
				else 	delete itemsArr[i];
			}
			if (remainingItemsArr.empty())   // means all the items are packed
			{
				exitFlag = true;
				result = _bestLowerBound + k;
				break;
			}
			itemsArr.clear();
			itemsArr.assign(remainingItemsArr.begin(), remainingItemsArr.end());
		}
	}
	return result;
}

/*
Section 5.1 
Solve a noncontiguous packing problem (NCBP)
*/
const int BLEU::LowerBound4()const 
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

/*
solve the root node of the parallel machine scheduling with contiguous constraints
*/
const int BLEU::LowerBound5()const
{
	int maxHeight = getMaximalHeight(_processedItems);
	std::map<int, std::list<int>> mapPosWidth, mapPosHeight;
	for (size_t idx = 0; idx < _processedItems.size(); ++idx)
	{
		auto possiblePositionsWidth = computeFX(_processedW - _processedItems[idx]->width, idx,
			_processedItems, true);
		auto possiblePositionsHeight = computeFX(maxHeight - _processedItems[idx]->height, idx,
			_processedItems, false);
		mapPosWidth.insert(std::pair<int, std::list<int>>(_processedItems[idx]->idx, possiblePositionsWidth));
		mapPosHeight.insert(std::pair<int, std::list<int>>(_processedItems[idx]->idx, possiblePositionsHeight));
	}

	return solve(_processedItems, mapPosWidth, mapPosHeight) + _processedH;
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


