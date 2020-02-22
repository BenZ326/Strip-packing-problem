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
	static int bigNumber;
	static int BBMaxExplNodesPerPack;				// maximal number of explored nodes for perfect packing (for the BB algorithm)
	static int BBMaxExplNodesNonPerPack;		// maximal number of explored nodes for non-perfect packing (for the BB algorithm)
public:
	BLEU(const std::vector<const item*>& t_items, const int t_W, const int t_TrialHeight);
	void preprocessing();
	void bounds();
	void takeOff();			// start to solve
	
// algorithms
protected:
	const solutionStatus combianotrialBenders() const;
	//preprocessing and bounds	
protected:
	// 5.1 preprocessing
	void preprocessingFixItems();
	void preprocessingReduceW();
	void preprocessingModifyItemWidth();
	// 5.1 preprocess the bounds
	const int LowerBound1() const;			// 5.1 lower bound 1
	const int LowerBound2() const;			// 5.2 lower bound 2
	const int LowerBound3() const;			// 5.3 lower bound 3
	const int LowerBound4() const;			// 5.4 lower bound 4
	const int LowerBound5() const;			// 5.5 lower bound 5
	// cut items into pieces each of which has 1 unit height and the original width
	void cutItemsAlongHeight();
	// dual feasibility function
const	double DualFeasibleFunction1(const int t_alpha, const int t_width) const;
const double DualFeasibleFunction2(const int t_alpha, const int t_width) const ;
const	double DualFeasibleFunction3(const int t_alpha, const int t_width) const;


/*
the y-check algorithm---------------------------------------------------start
*/
bool yCheckAlgorithm(const int t_processedW, const int t_TrialHeight,const std::vector<coordinate>& itemPositions, 
	const std::vector<const item*> t_processedItems) const;


/*
the y-check algorithm---------------------------------------------------end
*/

/*
The branch and bound algorithms----------------------------------------start
*/
class BBNode
{
public:
	BBNode(const std::vector<const item*>& t_remainingItems, const int t_Width, const int t_TrialHeight);
	BBNode(const BBNode& t_BBNode);					// copy constructor
	const int trialHeight;
	int leftMostIdx;
	std::vector<int> columnsOccupiedHeight;			// [10,5,3,2] means 10 units of height in the 1st column is occupied and 5 units for the 2nd column...
	std::vector<const item*> remainingItems;			// make heap
	std::vector<int> maxiItemIdxColumns;				// [3,5,1,7] means among items placed in 1st column, 3 is the largest index of...
	std::vector<coordinate> itemPositions;				// store the final positions of all the items in processedItems (same order)
};
protected:
	const solutionStatus branchAndBound() const;
	void makeBranch(const std::unique_ptr<BBNode>& t_currentNode, 
		const std::stack<std::unique_ptr<BBNode>>& t_dfstree) const;
	const bool bounding(const std::unique_ptr<BBNode>& t_currentNode) const;


	


/*
The branch and bound algorithms----------------------------------------end
*/

private:
	std::vector<const item*> _allItems;
	std::vector<const itemPieceWidth*> _allItemPiecesWidths;
	std::vector<int> _allWidths;
	std::vector<const item*> _processedItems;				// items that are going to be computed
	const int _W;
	int _processedH;
	int _processedW;
	int _bestLowerBound;
	int _trialHeight;			// the current height being tried



};