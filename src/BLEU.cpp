#include "BLEU.h"
#include <algorithm>
#include "util.h"
#include "spp.h"
#include "knapsack.h"
#include <iostream>
#include <stack>
#include <chrono>
#include "sl/code/un_2spp_h2.h"
double StripPacking::BLEU::tolerance = 0.0001;
int StripPacking::BLEU::bigNumber = 999999;
int StripPacking::BLEU::BBMaxExplNodesPerPack = 10000000;
int StripPacking::BLEU::BBMaxExplNodesNonPerPack = 10000;
int StripPacking::BLEU::interestingStatics = 0;
int StripPacking::BLEU::ycheckExplNode = 10;
bool StripPacking::BLEU::nodeLimitFlag = false;
StripPacking::algorithmStatus StripPacking::BLEU::algStatus = StripPacking::algorithmStatus::exact;
/*
only invoke when it's in evaluatedMode
*/
StripPacking::BLEU::BLEU(const std::vector<const item*>& t_items, const int t_W, const int t_TrialHeight,
	const int t_timeLimit)
	:_allItems(t_items),_W(t_W), _trialHeight(t_TrialHeight), _evaluatedMode(true), _timeLimit(t_timeLimit)
{
	StripPacking::BLEU::algStatus = StripPacking::algorithmStatus::exact;
	// sort the items by the nonincreasing of width and breaking ties by nonincreasing height
	std::sort(_allItems.begin(), _allItems.end(), compareItemByWidth);
	this->reassignItemsIdx();
	auto startClock = std::chrono::high_resolution_clock::now();
	this->preprocessing();
	this->bounds();
	auto durationClock = std::chrono::high_resolution_clock::now() - startClock;
	XYZTimer::timerPreprocess += std::chrono::duration_cast<std::chrono::milliseconds>(durationClock).count() / 1000.0;
}

StripPacking::BLEU::BLEU(const std::vector<const item*>& t_items, const int t_W, const int t_timeLimit)
	:_allItems(t_items), _W(t_W), _evaluatedMode(false), _timeLimit(t_timeLimit)
{
	StripPacking::BLEU::algStatus = StripPacking::algorithmStatus::exact;
	// sort the items by the nonincreasing of width and breaking ties by nonincreasing height
	std::sort(_allItems.begin(), _allItems.end(), compareItemByWidth);
	this->reassignItemsIdx();
	this->preprocessing();
	this->bounds();
}

/*
reassign idx for items
*/
void StripPacking::BLEU::reassignItemsIdx()
{
	int idx = 0;
	for (auto it = _allItems.begin(); it != _allItems.end(); ++it)
	{
		const_cast<item*>(*it)->idx = idx++;
	}
}



const StripPacking::solutionStatus StripPacking::BLEU::evaluate()
{
	if (_processedItems.empty()) return solutionStatus::feasible;
	if (_bestLowerBound > _trialHeight) return solutionStatus::infeasible;
	this->calculateUB();
	if (_upperBound <= _trialHeight - _processedH) return solutionStatus::feasible;
	int binWidth = _processedW;
	int binHeight = _trialHeight - _processedH;
	int increment = 0;
	std::vector<item*> tmpItems;
	for (const auto& it : _processedItems)
	{
		item* tmp = new item(it->idx, it->width, it->height, it->idxHelper);
		tmpItems.push_back(tmp);
	}
	this->preprocessItemHeight(tmpItems, binHeight, binWidth);
	// make items constant
	std::vector<const item*> Items;
	for (const auto& it : tmpItems) Items.push_back(std::move(it));
	// end preprocess
	auto start = std::chrono::high_resolution_clock::now();
	auto status = this->branchAndBound(Items, binWidth, binHeight);
	XYZTimer::timerBB += std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count() / 1000.0;
	status = (BLEU::nodeLimitFlag && status == solutionStatus::infeasible) ? solutionStatus::pending : status;
	if (status != solutionStatus::pending)
	{
		this->releaseTmpItems(Items);
		return status;
	}
	start = std::chrono::high_resolution_clock::now();
	status = this->combinatorialBenders(Items, binWidth, binHeight, increment);
	XYZTimer::timerBD += std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count() / 1000.0;
	status = (BLEU::nodeLimitFlag && status == solutionStatus::infeasible) ? solutionStatus::pending : status;
	this->releaseTmpItems(Items);
	return status;
}
/*
There are two modes, one is the to solve an SPP algorithm, e.g. minimizing the height

The other mode is evaluating a given height to see if there is a packing can realize the height
if it is feasible, then return the height
else return the next possible height, implying the the height is infeasible for any packing

*/
int StripPacking::BLEU::takeOff()
{
	this->calculateUB();
	double elapsedTimeBB = 0.0;
	double elapsedTimeBD = 0.0;
	int increment = 0;
	while (true)
	{
		int binWidth = _processedW;
		int binHeight = _bestLowerBound - _processedH + increment;
		if (_upperBound <= binHeight) return _bestLowerBound + increment;
		std::vector<item*> tmpItems;
		for (const auto& it : _processedItems)
		{
			item* tmp = new item(it->idx, it->width, it->height, it->idxHelper);
			tmpItems.push_back(tmp);
		}
		this->preprocessItemHeight(tmpItems, binHeight, binWidth);
		auto rotate = this->ifRotateInstance(tmpItems, binHeight, binWidth);
		if (rotate)
		{
			this->rotateInstance(tmpItems, binWidth, binHeight);
			std::cout << "\nrotate\n";
		}
		 //make items constant
		std::vector<const item*> Items;
		for (const auto& it : tmpItems) Items.push_back(std::move(it));
		//// end preprocess
		auto start = std::chrono::high_resolution_clock::now();
		auto bbStatus = this->branchAndBound(Items, binWidth, binHeight);
		elapsedTimeBB += (std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start)).count() / 1000000.0;
		XYZTimer::timerBB = elapsedTimeBB;
		if (bbStatus == solutionStatus::feasible)
		{
			this->releaseTmpItems(Items);
			break;
		}
		start = std::chrono::high_resolution_clock::now();
		solutionStatus status = this->combinatorialBenders(Items, binWidth, binHeight, increment);
		elapsedTimeBD += (std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start)).count() / 1000000.0;
		//std::cout << "result of the combinatorial benders is  " << status << "the height is "<<binHeight + _processedH<<std::endl;
		if (StripPacking::BLEU::nodeLimitFlag && status == StripPacking::solutionStatus::infeasible)
			BLEU::algStatus = algorithmStatus::approximate;
		XYZTimer::timerBD = elapsedTimeBD;
		if (status == solutionStatus::feasible)
		{
			this->releaseTmpItems(Items);
			break;
		}
		else
		{
			this->releaseTmpItems(Items);
		}
	}
	return _bestLowerBound + increment;
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
bool StripPacking::BLEU::yCheckAlgorithm(const int t_processedW, const int t_TrialHeight, const std::vector<coordinate>& itemPositions,
	const std::vector<const item*> t_processedItems) const

{
	std::vector<coordinate> Cords4yCheck = itemPositions;
	int binWidth = t_processedW;
	auto Items = this->preprocess4yCheck(binWidth, t_processedItems, Cords4yCheck, t_TrialHeight);
	bool result = (this->yCheckEnumerationTree(Items, Cords4yCheck, t_TrialHeight, binWidth) == solutionStatus::feasible);
	this->releaseTmpItems(Items);
	//bool result = (this->yCheckEnumerationTree(t_processedItems, itemPositions, t_TrialHeight
	//	, t_processedW) == solutionStatus::feasible);
	return result;
}

/*

t_InterestItems: the items that are going through the enumeration tree
t_xCords: the corresonding x coordinates of the items, with the same ordering as that of t_interestItems
t_Height: the height of a bin to pack items in t_InterestItems
t_Width: the width of a bin to pack items in t_InterestItems
*/
StripPacking::solutionStatus StripPacking::BLEU::yCheckEnumerationTree(const std::vector<const item*>& t_InterestItems, const std::vector<coordinate>& t_Cords,
	const int t_Height, const int t_Width) const
{
	if (t_Width == 0) return solutionStatus::infeasible;			// it could happen when the very item ends at the current last column, then there will be no space for any merge
	std::unique_ptr<BBNode> root(new BBNode(t_InterestItems, t_Cords, t_Width, t_Height));
	std::stack<std::unique_ptr<BBNode>> yEnTree;
	yEnTree.push(std::move(root));
	int exploreNodes = 0;
	while (!yEnTree.empty())
	{
		const auto currentNode = std::move(yEnTree.top());
		yEnTree.pop();
		if (currentNode->remainingItems.empty())
		{
			return solutionStatus::feasible;
		}
		if (this->yCheckBounding(currentNode))	continue;
		exploreNodes++;
		this->yCheckMakeBranch(currentNode, yEnTree);
		if (exploreNodes > StripPacking::BLEU::ycheckExplNode)
		{
			StripPacking::BLEU::nodeLimitFlag = true;
			return solutionStatus::pending;
		}
	}
	return solutionStatus::infeasible;
}

bool StripPacking::BLEU::yCheckBounding(const std::unique_ptr<BBNode>& t_currentNode) const
{
	// fathoming criteria 1
	for (size_t i = 0; i < t_currentNode->columnsOccupiedHeight.size(); ++i)
	{
		int sumHeight = t_currentNode->columnsOccupiedHeight[i];
		for (auto it : t_currentNode->remainingItems)
		{
			if (t_currentNode->itemPositions[it->idxHelper].x <= i &&
				t_currentNode->itemPositions[it->idxHelper].x + it->width-1 >= i)
			{
				sumHeight += it->height;
			}
			if (sumHeight > t_currentNode->trialHeight)
				return true;
		}
	}

	if (!t_currentNode->packedItems.empty())
	{
		const item* tmpItem = t_currentNode->packedItems.back();
		// fathoming criteria 3
		for (const auto& it : t_currentNode->remainingItems)
		{
			if (tmpItem->height == it->height && tmpItem->width == it->width &&
				t_currentNode->itemPositions[tmpItem->idxHelper].x == t_currentNode->itemPositions[it->idxHelper].x&&
				it->idx < tmpItem->idx)
				return true;
		}
	}
	
	if (!t_currentNode->packedItems.empty()) {
		// fathoming criteria 4
		const item* itemJ = t_currentNode->packedItems.back();
		auto p_js = t_currentNode->itemPositions[itemJ->idxHelper].x;
		for (size_t i = 0; i < t_currentNode->packedItems.size() - 1; ++i)
		{
			const item* itemK = t_currentNode->packedItems[i];
			auto xCord = t_currentNode->itemPositions[itemK->idxHelper].x;
			if (p_js == xCord && itemJ->idx < itemK->idx && itemJ->width == itemK->width)
			{
				if (t_currentNode->itemPositions[itemK->idxHelper].y == t_currentNode->columnsOccupiedHeight[p_js] - itemJ->height- itemK->height)
				{
					return true;
				}
			}
		}
	}
	return false;
}

void StripPacking::BLEU::yCheckMakeBranch(const std::unique_ptr<BBNode>& t_currentNode,
	std::stack<std::unique_ptr<BBNode>>& t_yEntree) const
{
	std::list<std::unique_ptr<BBNode>> children;
	std::vector<const item*> copyRemainingItems = t_currentNode->remainingItems;
	std::sort(copyRemainingItems.begin(), copyRemainingItems.end(), compareItemByxCords(t_currentNode->itemPositions));
	auto niche = this->getNiche(t_currentNode->columnsOccupiedHeight);
	// h(l)
	int l_niche = niche[0] == 0 ? t_currentNode->trialHeight : t_currentNode->columnsOccupiedHeight[niche[0] - 1];
	// h(r)
	int r_niche = niche[1] == t_currentNode->columnsOccupiedHeight.size() - 1 ? t_currentNode->trialHeight : t_currentNode->columnsOccupiedHeight[niche[1] + 1];
	bool emptyItem = true;
	for (size_t i = 0; i < copyRemainingItems.size(); ++i)
	{
		if (t_currentNode->itemPositions[copyRemainingItems[i]->idxHelper].x >= niche[0] &&
			t_currentNode->itemPositions[copyRemainingItems[i]->idxHelper].x + copyRemainingItems[i]->width - 1 <= niche[1])
		{
			bool fathom = false;
			const int& p_js = t_currentNode->itemPositions[copyRemainingItems[i]->idxHelper].x;
			 //fathoming criteria 5
			if (p_js > niche[0])
			{
				for (size_t k= 0; k < copyRemainingItems.size(); ++k)
				{
					if (t_currentNode->itemPositions[copyRemainingItems[k]->idxHelper].x >= niche[0] &&
						t_currentNode->itemPositions[copyRemainingItems[k]->idxHelper].x + copyRemainingItems[k]->width - 1 <= niche[1])
					{
						if (i == k)
							continue;
						const int& p_ks = t_currentNode->itemPositions[copyRemainingItems[k]->idxHelper].x;
						if (p_ks + copyRemainingItems[k]->width -1 <= p_js)
						{
							if (copyRemainingItems[k]->height <= std::min(l_niche - t_currentNode->columnsOccupiedHeight[niche[0]], copyRemainingItems[i]->height))
							{
								fathom = true;
								break;
							}
						}
					
					}
				}
			
			}
			if (fathom) continue;
			std::unique_ptr<BBNode> child(new BBNode(*t_currentNode, copyRemainingItems[i]));
			child->itemPositions[copyRemainingItems[i]->idxHelper].y = 
				child->columnsOccupiedHeight[child->itemPositions[copyRemainingItems[i]->idxHelper].x];
			for (size_t j = child->itemPositions[copyRemainingItems[i]->idxHelper].x; 
				j < child->itemPositions[copyRemainingItems[i]->idxHelper].x + copyRemainingItems[i]->width; ++j)
			{
				child->columnsOccupiedHeight[j] += copyRemainingItems[i]->height;
			}
			// fathoming criteria 2
			if (emptyItem && child->columnsOccupiedHeight[p_js] + copyRemainingItems[i]->height<= std::min(l_niche, r_niche))
				emptyItem = false;
			children.push_back(std::move(child));
		}
	}

	if (emptyItem)
	{
		std::unique_ptr<BBNode> child(new BBNode(*t_currentNode));
		for (size_t j = niche[0]; j <= niche[1]; ++j)
		{
			child->columnsOccupiedHeight[j] = std::min(l_niche, r_niche);
		}
		children.push_back(std::move(child));
	}
	for (auto it = children.begin(); it != children.end(); ++it)
		t_yEntree.push(std::move(*it));
}
/*
Get the Niche 
t_ColumnsHeight: it is the height of each column 
the return value is a two dimensional array, one element of the array is the start column of the niche
while the second is the end column of the niche
*/
std::vector<int> StripPacking::BLEU::getNiche(const std::vector<int>& t_ColumnHeights) const
{
	std::vector<int>result (2, 0);
	auto ptr = std::min_element(t_ColumnHeights.begin(), t_ColumnHeights.end());
	int startCol =  ptr - t_ColumnHeights.begin();
	int endCol = t_ColumnHeights.size()-1;
	for (size_t i = startCol+1; i < t_ColumnHeights.size(); i++)
	{
		if (t_ColumnHeights[i] > (*ptr))
		{
			endCol = i - 1;
			break;
		}
	}
	result[0] = startCol;
	result[1] = endCol;
	return result;
}

/*
the y-check algorithm ---------------------------------------------------------------------------------------end
*/


//The branch and bound algorithm -----------------------------------------------------------------------------start
/*

*/
StripPacking::BLEU::BBNode::BBNode(const std::vector<const item*>& t_remainingItems, const int t_Width, const int t_TrialHeight):
remainingItems(t_remainingItems), trialHeight(t_TrialHeight)
{
	leftMostIdx = 0;
	columnsOccupiedHeight = std::vector<int>(t_Width,0);			// the order is consistent to the left most column -> the right most column
	maxiItemIdxColumns = std::vector<int>(t_Width,-1);				// the order is consistent to the left most column -> the right most column
	coordinate dummy(-1, -1);
	itemPositions = std::vector<coordinate>(t_remainingItems.size(), dummy);
}

StripPacking::BLEU::BBNode::BBNode(const BBNode& t_BBNode):trialHeight(t_BBNode.trialHeight)
{
	leftMostIdx = t_BBNode.leftMostIdx;
	columnsOccupiedHeight = t_BBNode.columnsOccupiedHeight;			// [10,5,3,2] means 10 units of height in the 1st column is occupied and 5 units for the 2nd column...
	remainingItems = t_BBNode.remainingItems;			// make heap
	maxiItemIdxColumns = t_BBNode.maxiItemIdxColumns;				// [3,5,1,7] means among items placed in 1st column, 3 is the largest index of...
	itemPositions = t_BBNode.itemPositions;
	packedItems = t_BBNode.packedItems;
}

// build a BBNode for the y check algorithm
StripPacking::BLEU::BBNode::BBNode(const std::vector<const item*>& t_remainingItems, const std::vector<coordinate>& t_Cords,
	const int t_Width, const int t_TrialHeight):remainingItems(t_remainingItems), trialHeight(t_TrialHeight)
{
	leftMostIdx = 0;
	columnsOccupiedHeight = std::vector<int>(t_Width, 0);			// the order is consistent to the left most column -> the right most column
	maxiItemIdxColumns = std::vector<int>(t_Width, 0);				// the order is consistent to the left most column -> the right most column
	coordinate dummy(-1, -1);
	itemPositions = t_Cords;
}

StripPacking::BLEU::BBNode::BBNode(const BBNode& t_BBNode, const item* t_placedItem)			// invoked when making a branch, the t_placedItem is the packed item in this time of branching
:trialHeight(t_BBNode.trialHeight)
{
	packedItems = t_BBNode.packedItems;
	packedItems.push_back(t_placedItem);
	for (const auto& it : t_BBNode.remainingItems)
	{
		if (it->idx == t_placedItem->idx)
			continue;
		else this->remainingItems.push_back(it);
	}
	leftMostIdx = t_BBNode.leftMostIdx;
	columnsOccupiedHeight = t_BBNode.columnsOccupiedHeight;			// [10,5,3,2] means 10 units of height in the 1st column is occupied and 5 units for the 2nd column...
	maxiItemIdxColumns = t_BBNode.maxiItemIdxColumns;				// [3,5,1,7] means among items placed in 1st column, 3 is the largest index of...
	itemPositions = t_BBNode.itemPositions;
}

const bool StripPacking::BLEU::bounding(const std::unique_ptr<BBNode>& t_currentNode) const
{
	//fathoming criteria 1
	if (!t_currentNode->packedItems.empty())
	{
		const item* tmpItem = t_currentNode->packedItems.back();
		for (const auto & it : t_currentNode->remainingItems)
		{
			if (it->height == tmpItem->height && it->width == tmpItem->width && it->idx < tmpItem->idx)
				return true;
		}
	}
	// fathoming criteria 2 is merged in the makeBranch function

	// standard continuous bounding which is described in the section 5.2 "branch and bound for the spp(L)"
	// fathoming criteria 3
	int remainingArea = 0;
	for(const auto& it : t_currentNode->remainingItems) 	remainingArea += it->height*it->width;
	int spaceArea = 0;
	for (size_t i = 0; i < t_currentNode->columnsOccupiedHeight.size(); ++i)
		spaceArea += (t_currentNode->trialHeight - t_currentNode->columnsOccupiedHeight[i]);
	if (remainingArea > spaceArea)
		return true;
	// fathoming criteria 4
	 //dynamic cuts:
	if (this->dynamicCuts(t_currentNode))
	{
		BLEU::interestingStatics++;
		return true;
	}
	return false;
	
}

void  StripPacking::BLEU::makeBranch(const std::unique_ptr<BBNode>& t_currentNode,
 std::stack<std::unique_ptr<BBNode>>& t_dfstree) const
{
	std::set<whPair> packedPairs;
	std::list<std::unique_ptr<BBNode>> children;
	std::list<std::unique_ptr<BBNode>> emptyChild;
	// selects the left-most column
	int selectedColumn = t_currentNode->leftMostIdx;   // start from 0, so it ranges from 0, 1,2,3,....., _processedW-1
	// pack nothing
	if (t_currentNode->columnsOccupiedHeight[selectedColumn] > 0)
	{
		std::unique_ptr<BBNode> child(new BBNode(*t_currentNode.get()));
		child->columnsOccupiedHeight[selectedColumn] = child->trialHeight;
		child->leftMostIdx++;
		emptyChild.push_back(std::move(child));
	}
	// pack j on the column
	std::vector<const item*> copyRemainingItems = t_currentNode->remainingItems;
	std::sort(copyRemainingItems.begin(), copyRemainingItems.end(),compareItemByIdx);
	for(int i = 0;  i< copyRemainingItems.size();i++)
	{
		auto chosenItem = copyRemainingItems[i];
		// check width of the item 
		if (chosenItem->width + selectedColumn > t_currentNode->columnsOccupiedHeight.size())
			continue;
		// check height of the item
		if (chosenItem->height + t_currentNode->columnsOccupiedHeight[selectedColumn] > t_currentNode->trialHeight)
			continue;
		if (t_currentNode->maxiItemIdxColumns[selectedColumn] > chosenItem->idx)
			continue;
		int minHeight = 99999;
		for (size_t k = 0; k < copyRemainingItems.size(); ++k)
		{
			if (k == i) continue;
			if (minHeight > copyRemainingItems[k]->height)
				minHeight = copyRemainingItems[k]->height;
		}
		// make a child by pack the item on the column, update the coordinate 
		std::unique_ptr<BBNode> child(new BBNode(*t_currentNode.get(), copyRemainingItems[i]));
		child->itemPositions[chosenItem->idxHelper].x = selectedColumn;
		child->itemPositions[chosenItem->idxHelper].y = t_currentNode->columnsOccupiedHeight[selectedColumn];
		child->maxiItemIdxColumns[selectedColumn] = chosenItem->idx;
		//if there exists an item can be added on the column
		for (int offset = 0; offset <= chosenItem->width - 1; ++offset)
		{
			if (chosenItem->height + t_currentNode->columnsOccupiedHeight[selectedColumn + offset] + minHeight
				<= t_currentNode->trialHeight)
			{
				child->columnsOccupiedHeight[selectedColumn + offset] += chosenItem->height;
			}
			else
			{
				child->columnsOccupiedHeight[selectedColumn + offset] = t_currentNode->trialHeight;
				child->leftMostIdx = selectedColumn + offset +1;
			}
		}
		children.push_back(std::move(child));
	}
	// load the empty child
	if (!emptyChild.empty())
		t_dfstree.push(std::move(emptyChild.back()));
	for (auto it = children.rbegin(); it != children.rend(); ++it)
		t_dfstree.push(std::move((*it)));
}

const StripPacking::solutionStatus StripPacking::BLEU::branchAndBound(const std::vector<const item*>& t_Items, const int t_binWidth,
	const int t_binHeight)
{
	StripPacking::BLEU::nodeLimitFlag = false;
	int tmpW = t_binWidth;
	int tmpH = t_binHeight;
	BLEU::interestingStatics = 0;
	int maxExpNodes;
	std::unique_ptr<BBNode>	root(new BBNode(t_Items, tmpW, tmpH));
	double totalArea = 0.0;
	for (const auto& it : t_Items) totalArea += it->width*it->height;
	if (abs(tmpH-(totalArea / tmpW)) <BLEU::tolerance) maxExpNodes = BLEU::BBMaxExplNodesPerPack;
	else maxExpNodes = BLEU::BBMaxExplNodesNonPerPack;
	std::stack<std::unique_ptr<BBNode>> dfsTree;
	dfsTree.push(std::move(root));
	int numberExploredNodes = 0;
	while (!dfsTree.empty() && numberExploredNodes < maxExpNodes)
	{
		const auto currentNode = std::move(dfsTree.top());
		dfsTree.pop();
		// if it's a feasible solution then invoke the y-check algorithm
		if (currentNode->remainingItems.empty())
		{
			if (this->yCheckAlgorithm(tmpW, tmpH, currentNode->itemPositions, t_Items))
			{
				return solutionStatus::feasible;
			}
			else continue;							// the node can not be transformed to a feasible solution for the SPP
		}
		else
		{
			// bounding the current Node
			if (this->bounding(currentNode))
				continue;
			// make branch
			numberExploredNodes++;
			this->makeBranch(currentNode, dfsTree);
		}
	}
	if (numberExploredNodes >= maxExpNodes)
	{
		return solutionStatus::pending;
	}
	else return solutionStatus::infeasible;
}

const bool StripPacking::BLEU::dynamicCuts(const std::unique_ptr<BBNode>& t_currentNode) const
{
	std::list<coordinate> leftCorners;
	int prev = t_currentNode->trialHeight;
	for (size_t i=0; i<t_currentNode->columnsOccupiedHeight.size(); ++i)
	{
		if (prev > t_currentNode->columnsOccupiedHeight[i])
		{
			coordinate cords(i, t_currentNode->columnsOccupiedHeight[i]);
			leftCorners.push_back(cords);
		}
		prev = t_currentNode->columnsOccupiedHeight[i];
	}
	bool result = false;
	size_t tmpSize = leftCorners.size();
	/*
	s[i][0]: the width of the i^th stage,
	s[i][1]: the height of the i^th stage
	g[i][0]: the x coordinate of the i^th left corner
	g[i][1]: the y coordinate of the i^th left corner
	l[i][0]: the realizable maximal width
	l[i][1]: the realizable maximal height
	*/
	int** s = new int*[tmpSize];
	int** g = new int*[tmpSize];
	int** l = new int*[tmpSize];
	for (size_t i = 0; i < tmpSize; ++i)
	{
		s[i] = new int[2];
		g[i] = new int[2];
		l[i] = new int[2];
	}
	int i = 0;
	int tmpXPrev = -1;			// store the x coordinate of the previous left corner
	int tmpYPrev = -1;			// store the y coordinate of the previous left corner
	/*
	At each left corner, the length can be updated for the onging left corner,
	while the width can only be updated by the upcoming left corner
	*/
	for (std::list<coordinate>::const_iterator iter = leftCorners.begin();
		iter != leftCorners.end(); ++iter)
	{
		g[i][0] = t_currentNode->maxiItemIdxColumns.size() - (*iter).x;
		g[i][1] = t_currentNode->trialHeight - (*iter).y;
		if (tmpXPrev == -1)
		{
			tmpXPrev = (*iter).x;
			tmpYPrev = (*iter).y;
			s[i][1] = t_currentNode->trialHeight - (*iter).y;
		}
		else
		{
			s[i - 1][0] = (*iter).x - tmpXPrev;
			s[i][1] = tmpYPrev - (*iter).y;
			tmpXPrev = (*iter).x;
			tmpYPrev = (*iter).y;
		}
		if (i == tmpSize - 1)
			s[i][0] = t_currentNode->maxiItemIdxColumns.size() - (*iter).x;
		++i;
	}

	/*
	compute the remaining available area (in the paper, it is denoted as V_pi)
	*/
	int vPi = 0;
	for (size_t i = 0; i < tmpSize; ++i)
		vPi += g[i][1] * s[i][0];

	/*
	preapre the onedimensional items for running the DP
	*/
	std::vector<const oneDimensionItem*> oneDim4HeightVec;
	std::vector<const oneDimensionItem*> oneDim4WidthVec;
	int accumulatedArea = 0;			//accumulated Area of the items considered so far
	for (std::vector<const item*>::const_iterator iter = t_currentNode->remainingItems.begin();
		iter != t_currentNode->remainingItems.end(); ++iter)
	{
		
		const oneDimensionItem* oneDimItemH = new oneDimensionItem((*iter)->height, (*iter)->height);
		const oneDimensionItem* oneDimItemW = new oneDimensionItem((*iter)->width, (*iter)->width);
		oneDim4HeightVec.push_back(oneDimItemH);
		oneDim4WidthVec.push_back(oneDimItemW);
	}
	std::vector<std::vector<int>> l0ValueOverItems;
	std::vector<std::vector<int>> l1ValueOverItems;
	for (size_t i = 0; i < tmpSize; ++i)
	{
		l0ValueOverItems.push_back(dynamicPrg4KnapSack<oneDimensionItem>(oneDim4WidthVec, g[i][0],0));
		l1ValueOverItems.push_back(dynamicPrg4KnapSack<oneDimensionItem>(oneDim4HeightVec, g[i][1],0));
	}
	/*
		calculate the values for matrix l
		*/
	for (size_t j = 0; j<t_currentNode->remainingItems.size();++j)
	{
		accumulatedArea += t_currentNode->remainingItems [j]->height*t_currentNode->remainingItems[j]->width;
		int A = 0;
		int sum_B = 0;
		std::vector<int> B;
		B.clear();
		for (size_t i = 0; i < tmpSize; ++i)
		{
			l[i][0] = l0ValueOverItems[i][j];
			l[i][1] = l1ValueOverItems[i][j];
		}
		for (size_t i = 0; i < tmpSize; ++i)
		{
			A += s[i][0] * (g[i][1] - l[i][1]) + s[i][1] * (g[i][0] - l[i][0]) - (g[i][1] - l[i][1])*(g[i][0] - l[i][0]);
			if (i < tmpSize - 1)
			{
				A += (g[i][1] - l[i][1])*(g[i + 1][0] - l[i + 1][0]);
			}
			if (t_currentNode->remainingItems.size() < tmpSize)
			{
				if (i == 0)
					B.push_back((g[1][0] - l[1][0] + s[0][0] - g[0][0] + l[0][0])*(s[0][1] - g[0][1] + l[0][1]));
				if (i == tmpSize - 1)
					B.push_back((s[i][0] - g[i][0] + l[i][0])*(g[i - 1][1] - l[i - 1][1] + s[i][1] - g[i][1] + l[i][1]));
				if (i > 0 && i < tmpSize - 1)
					B.push_back((g[i + 1][0] - l[i + 1][0] + s[i][0] - g[i][0] + l[i][0])*(g[i - 1][1] - l[i - 1][1] + s[i][1] - g[i][1] + l[i][1]));
			}
		}
		if (!B.empty())
		{
			std::sort(B.begin(), B.end());
			for (size_t k = 0; k < leftCorners.size() - t_currentNode->remainingItems.size(); ++k)
			{
				sum_B += B[k];
			}
		}
		if (vPi - (A + sum_B) < accumulatedArea)
		{
			result = true;
			break;
		}
	}
		
	/*
	Release the heap
	*/
	{
		for (size_t i = 0; i < tmpSize; ++i)
		{
			delete[] s[i];
			delete[] g[i];
			delete[] l[i];
		}
		delete[] s;
		delete[] g;
		delete[] l;
		for (std::vector<const oneDimensionItem*>::iterator iter = oneDim4HeightVec.begin(); iter != oneDim4HeightVec.end();
			++iter)
			delete (*iter);
		for (std::vector<const oneDimensionItem*>::iterator iter = oneDim4WidthVec.begin(); iter != oneDim4WidthVec.end();
			++iter)
			delete (*iter);
	}
	return result;
}


//The branch and bound algorithm -----------------------------------------------------------------------------end


/*
extract info from t_cplex to do y check
Args:
	1) t_allItems: the items considered in the algorithm
	2) t_cplex: the cplex model 
	3) t_allVars: a map of variables, key is the idx of items and value is the corresponding variable in the model
*/
std::vector<StripPacking::coordinate> StripPacking::BLEU::extractInfo4Ycheck(const std::vector<const item*>& t_allItems,
	const IloCplex& t_cplex, const IloArray<IloNumVarArray>& t_variables) const
{
	std::vector<coordinate> result(t_allItems.size(), coordinate(0, 0));
	for (int i = 0; i < t_variables.getSize(); ++i)
	{
		for (int j = 0; j < t_variables[i].getSize(); ++j)
		{
			double varValue = t_cplex.getValue(t_variables[i][j]);
			if (varValue >=1.0 - BLEU::tolerance)
			{
				result[t_allItems[i]->idxHelper].x = j;
			}
		}
	}
	return result;
}


/*
Given the width and height and the items, go to solve the problem
t_Items: should be sorted by increasing order of indexHelper 
*/
// The combinatorialBenders algorithm
const StripPacking::solutionStatus StripPacking::BLEU::combinatorialBenders(const std::vector<const item*>& t_Items, const int t_binWidth,
	const int t_binHeight, int & t_increment)
{
	StripPacking::BLEU::nodeLimitFlag = false;
	std::map<int, const item*> allItemsMap;
	int maxHeight = t_binHeight;
	std::map<int, std::set<int>> mapPosWidth, mapPosHeight;
	for (size_t idx = 0; idx < t_Items.size(); ++idx)
	{
		allItemsMap.insert(std::pair<int, const item*>(t_Items[idx]->idx, t_Items[idx]));
		auto possiblePositionsWidth = computeFX(t_binWidth - t_Items[idx]->width, idx,
			t_Items, true);
		auto possiblePositionsHeight = computeFX(maxHeight - t_Items[idx]->height, idx,
			t_Items, false);
		mapPosWidth.insert(std::pair<int, std::set<int>>(t_Items[idx]->idx, possiblePositionsWidth));
		mapPosHeight.insert(std::pair<int, std::set<int>>(t_Items[idx]->idx, possiblePositionsHeight));
	}
	// data preparation
	std::set<int> allPositions;
	for (const auto& it : mapPosWidth)
		for (const auto& it2 : it.second)
			allPositions.insert(it2);
	IloEnv env;
	IloModel model(env);
	IloNumVar z(env, 0, IloInfinity, "ObjZ");
	IloArray<IloNumVarArray> variables(env, t_Items.size());// i is the index in the t_Items, and j is the position
	for (int i = 0; i < variables.getSize(); ++i)
	{
		variables[i] = IloNumVarArray(env, t_binWidth + 1, 0,1, ILOINT);
		const item* tmp = t_Items[i];
		IloExpr expr(env);
		auto widthPos = mapPosWidth.find(tmp->idx)->second;
		for (int j = 0; j < variables[i].getSize(); ++j)
		{
			variables[i][j].setName(getVarName(tmp->idx, j).c_str());
			expr += variables[i][j];
			if (widthPos.find(j) == widthPos.end())
			{
				model.add(variables[i][j] == 0);
			}
		}
		model.add(expr == 1);
		expr.end();
	}
	// second constraints set
	
	for (const auto q : allPositions)
	{
		IloExpr expr(env);
		for (int i = 0; i < variables.getSize(); ++i)
		{
			const item* tmp = t_Items[i];
			// calculate W(j, q)
			for (int j =0; j<variables[i].getSize();++j)
			{
				if (j <= q && j >= q -tmp->width  + 1)
				{
					expr += tmp->height*variables[i][j];
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
	cplex.setParam(IloCplex::Param::Preprocessing::RepeatPresolve, 3);
	cplex.setParam(IloCplex::Param::Preprocessing::Reduce, 3);
	cplex.setParam(IloCplex::Param::MIP::Strategy::Probe, 3);
	cplex.setParam(IloCplex::Param::Preprocessing::Symmetry, 5);
	auto start = std::chrono::high_resolution_clock::now();
	double elapsedTimeBD = 0.0;
	while (true)
	{
		elapsedTimeBD = (std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start)).count() / 1000000.0;
		if (elapsedTimeBD > _timeLimit)
		{
			env.end();
			t_increment += 1;
			return solutionStatus::pending;
		}
		cplex.setParam(IloCplex::Param::TimeLimit, _timeLimit- elapsedTimeBD);
		cplex.solve();
		//std::cout << "mip status is " << cplex.getCplexStatus() << std::endl;
		if (cplex.getCplexStatus() == IloCplex::CplexStatus::AbortTimeLim)
		{
			env.end();
			t_increment += 1;
			BLEU::algStatus = algorithmStatus::approximate;
			return solutionStatus::pending;
		}
		if (cplex.getStatus() == IloAlgorithm::Status::Infeasible)
		{
			env.end();
			t_increment += 1;
			return solutionStatus::infeasible;
		}	
		double obj = cplex.getObjValue();
		if (obj <= t_binHeight)
		{
			// do y check 
			std::vector<coordinate> coordinates = this->extractInfo4Ycheck(t_Items, cplex, variables);
			if (this->yCheckAlgorithm(t_binWidth, t_binHeight, coordinates, t_Items))
			{
				env.end();
				return solutionStatus::feasible;
			}
			else
			{ 
				//// add a combinatorial cut
				const std::vector<std::vector<int>> subsetItems = this->findSubset(t_binHeight, t_binWidth, t_Items, coordinates, allItemsMap);
				for (size_t i = 0; i < subsetItems.size(); ++i)
				{
					std::vector<IloNumVar> selectedVars;
					for (const auto& it : subsetItems[i])
					{
						const item* tmp = allItemsMap.find(it)->second;
						selectedVars.push_back(variables[tmp->idxHelper][coordinates[tmp->idxHelper].x]);
					}
					this->addBendersCut(cplex, t_Items, variables, selectedVars);
				}
			}
		}
		else
		{
			env.end();
			t_increment += 1;
			return solutionStatus::infeasible;
		}
	}
}


void StripPacking::BLEU::addBendersCut(IloCplex& t_cplex, const std::vector<const item*>& t_allItems,
	const IloArray<IloNumVarArray>& t_variables, const std::vector<IloNumVar>& t_selectedVars) const
{
	IloExpr bendersCut(t_cplex.getModel().getEnv());
	for (const auto& it : t_selectedVars)
	{
		bendersCut += it;
	}
	t_cplex.getModel().add(bendersCut <= int(t_selectedVars.size()) - 1);
	bendersCut.end();
}

/*
This is the center of the combinatorial cuts, it find a subset of variables to strenthen the benders cut
The description of the method can be found at section 4.
*/
const std::vector<std::vector<int>> StripPacking::BLEU::findSubset(const int t_binHeight, const int t_binWidth,const std::vector<const item*>& t_allItems,
	const std::vector<coordinate>& t_Cords, const std::map<int, const item*>& t_allItemsMap) const
{
	std::vector<std::vector<int>> result;
	std::vector<int> wholeSet;
	for (const auto& it : t_allItems) wholeSet.push_back(it->idx);
	// first step vertical cuts
	result = this->verticalCuts(t_binHeight, t_binWidth, t_allItems, t_Cords);
	if (result.empty()) result.push_back(wholeSet);
	// second step
	result = this->findSubSetSecondStep(t_binWidth, t_binHeight, result, t_allItemsMap, t_Cords);
	// second step
//		 third step
	//result = this->findSubSetThirdStep(t_binWidth, t_binHeight, result, t_allItemsMap, t_Cords);
	return result;
}

const std::vector<std::vector<int>> StripPacking::BLEU::verticalCuts(const int t_binHeight, const int t_binWidth,
	const std::vector<const item*>& t_allItems, const std::vector<coordinate>& t_Cords) const
{
	std::stack<std::vector<int>> startEndColumns;
	std::vector<int> initialInterval(2, -1);
	initialInterval[0] = 0;
	initialInterval[1] = t_binWidth - 1;
	startEndColumns.push(initialInterval);
	std::vector<std::vector<int>> allSubsets;
	while (!startEndColumns.empty())
	{
		auto currentInterval = startEndColumns.top();
		startEndColumns.pop();
		auto nextIntervals = this->findSubsetItems4VerticalCut(t_binWidth, t_binHeight, currentInterval[0],
			currentInterval[1], t_allItems, t_Cords, allSubsets);
		if (!nextIntervals.empty())
			for (const auto& it : nextIntervals) startEndColumns.push(it);
	}
	return allSubsets;
}

const std::vector<std::vector<int>> StripPacking::BLEU::findSubsetItems4VerticalCut(const int t_binWidth, const int t_binHeight, const int t_startColumn, const int t_endColumn,
	const std::vector<const item*>& t_allItems, const std::vector<coordinate>& t_Cords,
	std::vector<std::vector<int>>& t_subsetItems) const
{
	std::vector<std::vector<int>> result;
	for (int i = t_startColumn; i < t_endColumn; ++i)
	{
		if (!this->ifCrossOverColumn(i, t_allItems, t_Cords))
		{
			// split the columns
			// the left subset:
			std::vector<coordinate> leftCords;
			auto leftColumnsItems = this->getItemsPackedColumn(t_allItems, t_Cords, t_startColumn, i);
			bool leftfeasibility = this->yCheckAlgorithm(t_binWidth, t_binHeight, t_Cords, leftColumnsItems);
			if (!leftfeasibility)
			{
				std::vector<int> leftSubset(2, -1);
				leftSubset[0] = t_startColumn;
				leftSubset[1] = i;
				result.push_back(leftSubset);
				std::vector<int> subsetItems;
				for (const auto& it : leftColumnsItems) subsetItems.push_back(it->idx);
				t_subsetItems.push_back(subsetItems);
			}
			// right subset:
			std::vector<coordinate> rightCords;
			auto rightColumnsItems = this->getItemsPackedColumn(t_allItems, t_Cords, i + 1, t_endColumn);
			bool rightfeasibility = this->yCheckAlgorithm(t_binWidth, t_binHeight, t_Cords, rightColumnsItems);
			if (!rightfeasibility)
			{
				std::vector<int> rightSubset(2, -1);
				rightSubset[0] = i + 1;
				rightSubset[1] = t_endColumn;
				result.push_back(rightSubset);
				std::vector<int> subsetItems;
				for (const auto& it : rightColumnsItems) subsetItems.push_back(it->idx);
				t_subsetItems.push_back(subsetItems);
			}
			return result;
		}
	}
	return result;
}

/*
given a column, return if there exists an item ``cross over'' it
*/
const bool StripPacking::BLEU::ifCrossOverColumn(const int t_Column,const std::vector<const item*>& t_allItems, const std::vector<coordinate>& t_Cords) const
{
	for (size_t i = 0; i < t_allItems.size(); ++i)
	{
		const item* tmp = t_allItems[i];
		if (t_Cords[tmp->idxHelper].x < t_Column && t_Cords[tmp->idxHelper].x + tmp->width > t_Column)
			return true;
	}
	return false;
}

/*
given a set of columns, return all the items packed on the columns(must end before the left-most column)
the function has to be invoked when you are sure that the given columns contain complete items rather than partial items 
(which means some items are partially packed in the column)
create new items!!
*/
const std::vector<const StripPacking::item*> StripPacking::BLEU::getItemsPackedColumn(
	const std::vector<const item*>& t_allItems, const std::vector<coordinate>& t_Cords,
	const int t_startColumn, const int t_endColumn) const
{
	std::vector<const item*> result;
	int idxHelper = 0;
	for (const auto& it : t_allItems)
	{
		if (t_Cords[it->idxHelper].x >= t_startColumn && t_Cords[it->idxHelper].x <= t_endColumn)
		{
			result.push_back(it);
		}
	}
	return result;
}

/*
Given a column, remove the items packed on the column
Args: 
  t_Column: the given column
  t_Items: the set of indices of the items (the set we are dealing with)
  t_Cords: the coordinates of all items
  t_allItemsMap: a map of <item idx, item>
*/
const std::set<int> StripPacking::BLEU::getRemovedItems(const int t_Column, const std::vector<int>& t_Items,
	const std::vector<coordinate>& t_Cords, const std::map<int, const item*>& t_allItemsMap) const
{
	std::set<int> removedItems;
	for (const auto & it : t_Items)
	{
		auto tmpItem = t_allItemsMap.find(it)->second;
		if (t_Cords[tmpItem->idxHelper].x <= t_Column && t_Cords[tmpItem->idxHelper].x + tmpItem->width -1 >= t_Column)
		{
			removedItems.insert(tmpItem->idx);
		}
	}
	return removedItems;
}


/*
Given a subset of items and the whole coordinates, return the columns containing the subset
Args£º
	t_subset: the subset of items
	t_Cords: the coordinates of all items
*/
const std::vector<int> StripPacking::BLEU::getColumnsFromSubset(const std::vector<int>& t_subset,
	const std::map<int, const item*>& t_allItemsMap, const std::vector<coordinate>& t_Cords) const
{
	std::vector<int> result(2, -1);
	int leftMost = 999999;
	int rightMost = -1;
	for (const auto & it : t_subset)
	{
		auto tmpItem = t_allItemsMap.find(it)->second;
		if (t_Cords[tmpItem->idxHelper].x < leftMost) leftMost = t_Cords[tmpItem->idxHelper].x;
		if (t_Cords[tmpItem->idxHelper].x + tmpItem->width -1 > rightMost) rightMost = t_Cords[tmpItem->idxHelper].x + tmpItem->width -1;
	}
	result[0] = leftMost;
	result[1] = rightMost;
	return result;
}



std::vector<std::vector<int>> StripPacking::BLEU::findSubSetSecondStep(const int t_binWidth, const int t_binHeight,const std::vector<std::vector<int>>& t_currentSubSets,
	const std::map<int, const item*>& t_allItemsMap, const std::vector<coordinate>& t_Cords) const
{
	std::vector<std::vector<int>> result;
	for (const auto& it : t_currentSubSets)
	{
		
		std::vector<int> processingsubset = it;
		// returned columns is the indices in the original columns set
		auto columns = this->getColumnsFromSubset(processingsubset, t_allItemsMap, t_Cords);
		int startCol = columns[0];
		int rightCol = columns[1];
		for (int i = startCol; i < rightCol; ++i)
		{
			auto removedItems = this->getRemovedItems(i, processingsubset, t_Cords, t_allItemsMap);
			if (removedItems.empty()) continue;
			std::vector<const item*> itemsYcheck;
			auto candidate =  this->attemptRemove(t_binHeight, t_binWidth, processingsubset, removedItems, t_Cords, t_allItemsMap);
			if (candidate.size() < processingsubset.size()) processingsubset = candidate;
			else	break;		// go from the right-hand side
		}
		// right-handside
		columns = this->getColumnsFromSubset(processingsubset, t_allItemsMap, t_Cords);
		startCol = columns[0];
		rightCol = columns[1];
		for (int i = rightCol; i > startCol; --i)
		{
			auto removedItems = this->getRemovedItems(i, processingsubset, t_Cords, t_allItemsMap);
			if (removedItems.empty()) continue;
			std::vector<const item*> itemsYcheck;
			auto candidate = this->attemptRemove(t_binHeight, t_binWidth, processingsubset, removedItems, t_Cords, t_allItemsMap);
			if (candidate.size() < processingsubset.size()) processingsubset = candidate;
			else
			{
				result.push_back(processingsubset);
				break;		// go from the right-hand side
			}
		}
	}
	return result;
}



std::vector<std::vector<int>> StripPacking::BLEU::findSubSetThirdStep(const int t_binWidth, const int t_binHeight, const std::vector<std::vector<int>>& t_currentSubSets,
	const std::map<int, const item*>& t_allItemsMap, const std::vector<coordinate>& t_Cords) const
{
	std::vector<std::vector<int>> result;
	for (const auto& it : t_currentSubSets)
	{
		std::vector<const item*> removingItems;
		for (const auto & jt : it) removingItems.push_back(t_allItemsMap.find(jt)->second);
		std::sort(removingItems.begin(), removingItems.end(), compareItemByArea);
		std::vector<const item*> processingItems = removingItems;
		for (size_t i = 0; i<removingItems.size(); ++i)
		{
			// remove the items
			for (auto pt = processingItems.begin(); pt != processingItems.end(); ++pt)
			{
				if ((*pt)->idx == removingItems[i]->idx)
				{
					processingItems.erase(pt);
					break;
				}
			}
			if (this->yCheckAlgorithm(t_binWidth, t_binHeight, t_Cords, processingItems))
			{
				processingItems.push_back(removingItems[i]);
			}	
		}
		std::vector<int> tmpVec;
		for (const auto & jt : processingItems) tmpVec.push_back(jt->idx);
		result.push_back(tmpVec);
	}
	return result;
}

/*
given a set of removed Items and check if removed, a given range of columns are feasible or not
*/

std::vector<int> StripPacking::BLEU::attemptRemove(const int t_binHeight, const int t_binWidth, const std::vector<int>& t_processingsubset,
	const std::set<int>& t_removedItems,
	const std::vector<coordinate>& t_Cords, const std::map<int, const item*>& t_allItemsMap) const
{
	std::vector<int> result = t_processingsubset;
	std::vector<const item*> itemsYcheck;
	for (const auto& pt : t_processingsubset)
	{
		if (t_removedItems.find(pt) == t_removedItems.end())
		{
			auto tmpItem = t_allItemsMap.find(pt)->second;
			itemsYcheck.push_back(tmpItem);
		}
	}
	if (!this->yCheckAlgorithm(t_binWidth, t_binHeight, t_Cords, itemsYcheck))
	{
		result.clear();
		for (const auto & jt : itemsYcheck) result.push_back(jt->idx);
	}
	return result;
}





















/*
Cut all items into pieces along the direction of height
*/
void StripPacking::BLEU::cutItemsAlongHeight()
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
void StripPacking::BLEU::preprocessing()
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
void StripPacking::BLEU::preprocessingFixItems()
{
	// find the item with the minimal width
	int minWidth = _allItems.back()->width;
	int sumHeight = 0;
	int idx = 0;
	for (const auto& it : _allItems)
	{
		if (it->width + minWidth > _W)
			sumHeight += it->height;
		else
		{
			const_cast<item*>(it)->idxHelper = idx++;				// idxHelper stores the idx in the _processedItems.
			_processedItems.push_back(it);
		}
	}
	_processedH = sumHeight;				// store the occupied height
}

/*
Calculate the maximal width that a subset of items can be packed side by side if the maximal width is strictly less than W,
than W = the maximal width
*/
void StripPacking::BLEU::preprocessingReduceW()
{
	for (const auto& it : _processedItems)
		_allWidths.push_back(it->width);
	_processedW = subSetSum(_allWidths,  _W);
}




/*
modify width for items according to 2.2.3 in the paper "An exact algorithm for the two-dimensional strip packing problem"
*/
void StripPacking::BLEU::preprocessingModifyItemWidth()
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
1) modify item height and preprocess t_items (delete a few items so the helper idx is also changed)
2) modify the bin width 
*/
void StripPacking::BLEU::preprocessItemHeight(std::vector<item*>& t_items, 
	const int t_binHeight, int& t_binWidth)
{
	for (auto& i_it : t_items)
	{
		std::vector<int> integers;
		for (auto & j_it : t_items)
		{
			if (i_it->idx == j_it->idx) continue;
			integers.push_back(j_it->height);
		}
		int maxHeight = subSetSum(integers, t_binHeight - i_it->height) + (i_it)->height;
		if (maxHeight < t_binHeight)
		{
			(i_it)->height = (i_it)->height + t_binHeight - maxHeight;
		}
	}
	int minHeight = BigNumber;
	for (const auto& it : t_items)
	{
		if (minHeight > it->height) 
			minHeight = it->height;
	}
	std::list<item*> tmpItems;
	for (const auto& it : t_items)
	{
		if (it->height + minHeight <= t_binHeight)
			tmpItems.push_back(it);
		else t_binWidth -= it->width;
	}
	t_items.clear();
	int i = 0;
	for (auto & it : tmpItems)
	{
		it->idxHelper = i;
		++i;
		t_items.push_back(it);
	}
}

/*
section 5.1 lower bounds plus upper bounds
*/
void StripPacking::BLEU::bounds()
{
	int lb1 = this->LowerBound1();
	int lb2 = this->LowerBound2();
	int lb4 = this->LowerBound4();
	_bestLowerBound = std::max({ lb1, lb2, lb4 });
	int lb3 = this->LowerBound3();
	int lb5 = this->LowerBound5();
	_bestLowerBound = std::max({ _bestLowerBound, lb3, lb5 });
}

const int StripPacking::BLEU::LowerBound1() const
{
	int sum = 0;
	for (const auto& it : _processedItems)
	{
		sum += it->height*it->width;
	}
	return int(std::ceil(sum / _W)) + _processedH;
}

/*
Section 3.2 in the paper "An exact algorithm for the two-dimensional strip packing problem" 
dual feasible functions 1, 2 ,3
For understanding the concept of dual feasible functions, refer to the paper : "New  classes of fast lower  bounds  for  bin  packingproblems"
*/
const int StripPacking::BLEU::LowerBound2() const
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
const int StripPacking::BLEU::LowerBound3() const
{
	// step 1:
	if (_processedItems.empty()) return _bestLowerBound;
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
const int StripPacking::BLEU::LowerBound4()const
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
			//cplex.exportModel("NCBP.lp");
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
const int StripPacking::BLEU::LowerBound5()const
{
	int maxHeight = _bestLowerBound - _processedH;
	std::map<int, std::set<int>> mapPosWidth, mapPosHeight;
	for (size_t idx = 0; idx < _processedItems.size(); ++idx)
	{
		auto possiblePositionsWidth = computeFX(_processedW - _processedItems[idx]->width, idx,
			_processedItems, true);
		auto possiblePositionsHeight = computeFX(maxHeight - _processedItems[idx]->height, idx,
			_processedItems, false);
		mapPosWidth.insert(std::pair<int, std::set<int>>(_processedItems[idx]->idx, possiblePositionsWidth));
		mapPosHeight.insert(std::pair<int, std::set<int>>(_processedItems[idx]->idx, possiblePositionsHeight));
	}
	auto lb = solve(_processedItems, mapPosWidth, mapPosHeight, false) + _processedH;
	if (std::floor(lb) - lb < BLEU::tolerance) lb = std::floor(lb);
	else lb = std::ceil(lb);
	return lb;
	
}




/*
if it should be rotated then rotate the instance, if not do nothing
*/

const bool StripPacking::BLEU::ifRotateInstance(const std::vector<item*>& t_items,const int t_binHeight, const int t_binWidth) const
{
	int sumH = 0;
	int sumW = 0;
	for (size_t idx = 0; idx < t_items.size(); ++idx)
	{
		auto possiblePositionsWidth = computeFX(t_binWidth - t_items[idx]->width, idx,
			t_items, true);
		auto possiblePositionsHeight = computeFX(t_binHeight - t_items[idx]->height, idx,
			t_items, false);
		sumH  += possiblePositionsHeight.size();
		sumW += possiblePositionsWidth.size();
	}
	return (sumH < sumW);
}


void StripPacking::BLEU::rotateInstance(std::vector<item*>& t_Items, int& t_binWidth, int& t_binHeight) const
{
	for (const auto& it : t_Items)
	{
		int tmpW = it->width;
		it->width = it->height;
		it->height = tmpW;
	}
	int tmpWidth = t_binWidth;
	t_binWidth = t_binHeight;
	t_binHeight = tmpWidth;
}



const double StripPacking::BLEU::DualFeasibleFunction1(const int t_alpha, const int t_width) const
{
	double tmp = (t_alpha + 1.0)*((double(t_width) / _processedW));
	double intPart;
	if (std::modf(tmp, &intPart) < BLEU::tolerance) // if tmp is an integer
		return t_width;
	else
		return (std::floor(tmp)*(_processedW / double(t_alpha)));
}


const double StripPacking::BLEU::DualFeasibleFunction2(const int t_alpha, const int t_width) const
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

const	 double StripPacking::BLEU::DualFeasibleFunction3(const int t_alpha, const int t_width) const
{
	if (t_width > (_processedW / 2.0))
		return 2 * (std::floor(_processedW / double(t_alpha)) - std::floor((_processedW - t_width) / double(t_alpha)));
	if (std::abs(t_width - (_processedW / 2.0)) < StripPacking::BLEU::tolerance)
		return std::floor(_processedW / double(t_alpha));
	if (t_width < (_processedW / 2.0))
		return 2 * std::floor(t_width / double(t_alpha));
}


void StripPacking::BLEU::dumpSolution(const char* file_name) const
{
	std::ofstream ofs(file_name);
	std::ostringstream ss;
	for (size_t i = 0; i < _processedItems.size(); ++i)
	{
		auto tmp = _processedItems[i];
		ss.str("");
		ss.clear();
		ss << tmp->idx << "," << _finalSolution[tmp->idxHelper].x << "," << _finalSolution[tmp->idxHelper].y << "\n";
		ofs << ss.str();
	}
	ofs.close();
	ofs.open("Rectangle.output");
	for (size_t i = 0; i < _allItems.size(); ++i)
	{
		ss.str("");
		ss.clear();
		ss << _allItems[i]->idx << "," << _allItems[i]->width << "," << _allItems[i]->height << "\n";
		ofs << ss.str();
	}
	ofs.close();
}

void StripPacking::BLEU::dumpSolution(const char* file_name,const std::vector<const item*>& t_Items,
	const std::vector<coordinate>& t_Solution) const
{
	std::ofstream ofs(file_name);
	std::ostringstream ss;
	for (size_t i = 0; i < t_Items.size(); ++i)
	{
		auto tmp = t_Items[i];
		ss.str("");
		ss.clear();
		ss << tmp->idx << "," << t_Solution[tmp->idxHelper].x << "," << t_Solution[tmp->idxHelper].y << "\n";
		ofs << ss.str();
	}
	ofs.close();
	ofs.open("Rectangle.output");
	for (size_t i = 0; i < t_Items.size(); ++i)
	{
		ss.str("");
		ss.clear();
		ss << t_Items[i]->idx << "," << t_Items[i]->width << "," << t_Items[i]->height << "\n";
		ofs << ss.str();
	}
	ofs.close();
}

void StripPacking::BLEU::dumpSolution(const char* file_name, const std::vector<item*>& t_Items,
	const std::vector<coordinate>& t_Solution) const
{
	std::ofstream ofs(file_name);
	std::ostringstream ss;
	for (size_t i = 0; i < t_Items.size(); ++i)
	{
		auto tmp = t_Items[i];
		ss.str("");
		ss.clear();
		ss << tmp->idx << "," << t_Solution[tmp->idxHelper].x << "," << t_Solution[tmp->idxHelper].y << "\n";
		ofs << ss.str();
	}
	ofs.close();
	ofs.open("Rectangle.output");
	for (size_t i = 0; i < t_Items.size(); ++i)
	{
		ss.str("");
		ss.clear();
		ss << t_Items[i]->idx << "," << t_Items[i]->width << "," << t_Items[i]->height << "\n";
		ofs << ss.str();
	}
	ofs.close();
}


void StripPacking::BLEU::calculateUB()
{
	auto strip = StripPacking::createData4JFCAlg(this->_processedItems, this->_processedW, _bestLowerBound - _processedH);
	auto startClock = std::chrono::high_resolution_clock::now();
	_upperBound = un_spp_h2_solve(&strip);
	//_upperBound = un_spp_h2_quick_solve(&strip);
	auto durationClock = std::chrono::high_resolution_clock::now() - startClock;
	XYZTimer::timerMetaH += std::chrono::duration_cast<std::chrono::milliseconds>(durationClock).count() / 1000.0;
	
	strip_free(&strip);
}

strip_t StripPacking::createData4JFCAlg(const std::vector<const item*>& t_items,
	const int t_binWidth, const int t_Height)
{
	strip_t strip;
	strip_init_wh(&strip, t_binWidth, t_Height, t_items.size());
	int i = 0;
	for (const auto& it : t_items)
	{
		strip.items[i].origin_id = i;
		strip.items[i].no = i;
		strip.items[i].w = it->width;
		strip.items[i].h = it->height;
		i++;
	}
	strip.nb = t_items.size();
	strip.seq_count = 1;
	strip_init_phase2(&strip);
	return strip;
}


// helper functions
const std::set<int> StripPacking::BLEU::getItemsByCol(const int t_Col, 
	const std::vector<item*>& t_Items, const std::vector<coordinate>& t_Cords) const
{
	std::set<int> result;
	for (const auto& it : t_Items)
	{
		if (t_Cords[it->idxHelper].x == -1) continue;
		if (t_Cords[it->idxHelper].x <= t_Col && t_Col <= t_Cords[it->idxHelper].x + it->width - 1)
			result.insert(it->idxHelper);
	}
	return result;
}