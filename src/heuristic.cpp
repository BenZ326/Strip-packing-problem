#include "heuristic.h"
#include "skyline.h"
#include <fstream>
std::vector<StripPacking::coordinate>  StripPacking::Heuristic::solutions;


const int  StripPacking::Heuristic::parseSol(const  StripPacking::Skyline* t_skyline)
{
	int result = 0;
	auto cur = t_skyline->next;
	while (cur != nullptr && cur->length > 0)
	{
		result = std::max(cur->corY, result);
		cur = cur->next;
		delete cur->prev;
	}
	return result;
}

// given a fixed order of items
const int StripPacking::Heuristic::leftBottomHeuristic(const std::vector<const StripPacking::item*>& t_allItems, const int t_binWidth)
{
	Heuristic::solutions.clear();
	for (int i = 0; i < t_allItems.size(); ++i)
		Heuristic::solutions.push_back(coordinate(0, 0));
	// initialize the skyline 
	Skyline* stripBottom = new Skyline(t_binWidth);
	Skyline* leftSide = new Skyline(false);
	Skyline* rightSide = new Skyline(false);
	leftSide->next = stripBottom;
	rightSide->prev = stripBottom;
	stripBottom->next = rightSide;
	stripBottom->prev = leftSide;
	Skyline* selectedSkyline = nullptr;
	for (const auto& it : t_allItems)
	{
		while (1)
		{
			selectedSkyline = selectSkyline(leftSide, skylineSelectionMode::leftBottom);
			if (selectedSkyline->length < it->width)
			{
				liftSkyline(selectedSkyline);
				continue;
			}
			else break;
		}

		addItemOverSkyline(selectedSkyline, it);
	}
	int result = this->parseSol(leftSide);
	delete leftSide;
	delete rightSide;
	return result;
}

void StripPacking::Heuristic::dumpSolution(const std::vector<const StripPacking::item*>& t_allItems)
{
	std::ofstream rec("Rectangle.output");
	std::ofstream sol("Solution.output");
	for (const auto& it : t_allItems)
	{
		rec << it->idx << "," << it->width << ","
			<< it->height << "\n";
		sol << it->idx << "," << Heuristic::solutions[it->idx].x << ","
			<< Heuristic::solutions[it->idx].y << "\n";
	}
	rec.close();
	sol.close();
}

const StripPacking::item* StripPacking::Heuristic::findBestItem(std::vector<const StripPacking::item*>& t_allItems, const StripPacking::Skyline* t_skyline)
{
	auto ptr = t_allItems.begin();
	const StripPacking::item* res = nullptr;
	while (ptr != t_allItems.end() && (*ptr)->width > t_skyline->length)
	{
		if ((*ptr)->width == t_skyline->length)
		{
			res = (*ptr);
			ptr = t_allItems.erase(ptr);
			return res;
		}
		else ptr++;
	}
	if (ptr == t_allItems.end()) return nullptr;
	res = (*ptr);
	t_allItems.erase(ptr);
	return res;
}

const int StripPacking::Heuristic::bestFitHeuristic(std::vector<const StripPacking::item*>& t_allItems, const int t_binWidth)
{
	// sort all items by the non-increasing order of width
	Heuristic::solutions.clear();
	for (int i = 0; i < t_allItems.size(); ++i)
		Heuristic::solutions.push_back(coordinate(0, 0));
	// initialize the skyline 
	Skyline* stripBottom = new Skyline(t_binWidth);
	Skyline* leftSide = new Skyline(false);
	Skyline* rightSide = new Skyline(false);
	leftSide->next = stripBottom;
	rightSide->prev = stripBottom;
	stripBottom->next = rightSide;
	stripBottom->prev = leftSide;
	std::sort(t_allItems.begin(), t_allItems.end(), [](const StripPacking::item* t1, const StripPacking::item* t2) {
		return t1->width > t2->width || (t1->width == t2->width && t1->height > t2->height);
	});
	Skyline* selectedSkyline = nullptr;

	while (!t_allItems.empty())
	{
		// find the lowerest skyline
		selectedSkyline = selectSkyline(leftSide,StripPacking::skylineSelectionMode::bestFit);
		auto bestFitItem = this->findBestItem(t_allItems, selectedSkyline);
		int scenario = bestFitItem == nullptr? 2 :
			(bestFitItem->width == selectedSkyline->length ? 0 : 1);
		// if perfectly fit scenario =0
		// if less, scenario = 1
		// if none can fit, scenario = 2
		switch (scenario)
		{
		case 0:
		{
			addItemOverSkyline(selectedSkyline, bestFitItem);
			break;
		}
		case 1:
		{
			addItemOverSkyline(selectedSkyline, bestFitItem);			// niche placement policy: place the item at the leftside of the niche
			break;
		}
		case 2:
		{
			liftSkyline(selectedSkyline);
			break;
		}
		default:
			break;
		}
	}

	int result = this->parseSol(leftSide);
	delete leftSide;
	delete rightSide;
	return result;

}

const int StripPacking::Heuristic::iteratedGreedy(std::vector<const StripPacking::item*>& t_allItems, const int t_binWidth)
{
	bool improved = true;
	int primalBound = this->leftBottomHeuristic(t_allItems, t_binWidth);
	std::cout << "\nthe initial best primal bound is " << primalBound << "\n";
	while (improved)
	{
		improved = false;
		// explore the insertion neighborhood
		for (int i = 0; i < t_allItems.size(); ++i)
		{
			const StripPacking::item* selectedItem = t_allItems[i];
			for (int j = 0; j < t_allItems.size(); ++j)
			{
				if (i == j) continue;
				insertItem(t_allItems, i, j);
				int height = this->leftBottomHeuristic(t_allItems, t_binWidth);
				if (height < primalBound) {
					primalBound = height;
					improved = true;
					break;
				}
				else insertItem(t_allItems, j, i);
			}
		}
	}
	return primalBound;
}