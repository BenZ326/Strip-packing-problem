#pragma once
#include <vector>
#include "datareader.h"
#include <list>
#include <ilcplex/ilocplex.h>
#include <map>
#include <set>


/*
All fast utility function and basic structure of strip packing problem
*/

class itemPiece
{
public:
	itemPiece(const std::string t_id, const int t_height) :id(t_id), height(t_height) {}
	const std::string id;
	const int height;
};
class item
{
public:
	item(const int t_idx, const int t_width, const int t_height);
	const int idx;
	int width;
	const int height;
	std::vector<const itemPiece*> pieces;
	static std::ostringstream ss;
};


constexpr int BigNumber = 999999;
std::list<int> computeFX(const int t_x, const int t_idx, const std::vector<const item*>& t_items, bool flag);
int getMaximalHeight(const std::vector<const item*>& t_items);



/*
Instance of the SPP
*/
void solve(const std::vector<const item*>& t_allItems, const std::map<int, std::list<int>>& t_mapPosWidth,
	const std::map<int, std::list<int>>& t_mapPosHeight);


inline const std::string getVarName(const int t_itemIdx, const int t_xPos);

const std::string getVarName(const int t_itemIdx, const int t_xPos)
{
	item::ss.str("");
	item::ss.clear();
	item::ss << "item" << t_itemIdx << "assign" << t_xPos;
	return item::ss.str();
}













