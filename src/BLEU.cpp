#include "BLEU.h"
#include <algorithm>
#include "util.h"
BLEU::BLEU(const std::vector<const item*>& t_items, const int t_W)
	:_allItems(t_items),_W(t_W)
{
	
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
	std::cout<<"Lower bound 1 is "<<this->LowerBound1();
	std::cout<<"Lower bound 2 is "<<this->LowerBound2();
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
		for (const auto& it : _processedItems)
		{
			double tmp = (alpha + 1.0)*((double(it->width) / _processedW));
			double intPart;
			if (std::modf(tmp, &intPart) < 0.0001) // if tmp is an integer
				allTransformedWidths.push_back(std::round(tmp));
			else
				allTransformedWidths.push_back(std::floor(tmp)*(_processedW / double(alpha)));		
		}
		double sum = 0.0;
		for (size_t i = 0; i < _processedItems.size(); i++)		sum += allTransformedWidths[i] * _processedItems[i]->height;
		if (lowerBound < std::ceil(sum / _processedW))
			lowerBound = std::ceil(sum / _processedW);
	}
	return lowerBound;
}