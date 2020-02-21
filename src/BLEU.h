#pragma once
#include "datareader.h"

class itemPieceWidth;
/*
Fully reproduce the paper: Combinatorial Benders' Cuts for the strip packing problem
*/
class BLEU
{
	// global parameters
public:
	static double tolerance;
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
//	int LowerBound3();			// 5.3 lower bound 3
	int LowerBound4();			// 5.4 lower bound 4
//	void LowerBound5();			// 5.5 lower bound 5
	
// dual feasibility function
const	double DualFeasibleFunction1(const int t_alpha, const int t_width) const;
const double DualFeasibleFunction2(const int t_alpha, const int t_width) const ;
const	double DualFeasibleFunction3(const int t_alpha, const int t_width) const;
// utility function strongly related to the strip packing problem
protected:
	// cut items into pieces each of which has 1 unit height and the original width
	void cutItemsAlongHeight();
	
private:
	std::vector<const item*> _allItems;
	std::vector<const itemPieceWidth*> _allItemPiecesWidths;
	std::vector<int> _allWidths;
	std::vector<const item*> _processedItems;				// items that are going to be computed
	const int _W;
	int _processedH;
	int _processedW;



};