#pragma once
#include "datareader.h"

/*
Fully reproduce the paper: Combinatorial Benders' Cuts for the strip packing problem
*/
class BLEU
{
public:
	BLEU(const std::vector<const item*>& t_items, const int t_W);
	void preprocessing();
	void bounds();
//	
protected:
	// 5.1 preprocessing
	void preprocessingFixItems();
	void preprocessingReduceW();
	void preprocessingModifyItemWidth();
	// 5.1 preprocess the bounds
	int LowerBound1();			// 5.1 lower bound 1
	int LowerBound2();			// 5.2 lower bound 2
//	void LowerBound3();			// 5.3 lower bound 3
//	void LowerBound4();			// 5.4 lower bound 4
//	void LowerBound5();			// 5.5 lower bound 5
	
// utility function strongly related to the strip packing problem
protected:
	
private:
	const std::vector<const item*> _allItems;
	std::vector<int> _allWidths;
	std::vector<const item*> _processedItems;				// items that are going to be computed
	const int _W;
	int _processedH;
	int _processedW;



};