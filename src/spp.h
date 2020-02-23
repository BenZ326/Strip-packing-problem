#pragma once
#include <vector>
#include "datareader.h"
#include <list>
#include <ilcplex/ilocplex.h>
#include <map>
#include <set>
#include <algorithm>


enum solutionStatus {
	feasible,
	infeasible,
	pending,
	numberStatus
};
/*
All fast utility function and basic structure of strip packing problem
*/

/*
pieces for parallel scheduling problem with contiguity constraints
*/
class itemPiece
{
public:
	itemPiece(const std::string t_id, const int t_height) :id(t_id), height(t_height) {}
	const std::string id;
	const int height;
};

/*
pieces for 1CBP
*/
class itemPieceWidth
{
public:
	itemPieceWidth(const int t_id, const int t_width) :id(t_id), width(t_width) {}
	const int id;
	const int width;
};


struct whPair
{
	whPair() {}
	whPair(int t_h, int t_w) : h(t_h), w(t_w) {};
	whPair(const whPair& t_wh) :h(t_wh.h), w(t_wh.w) {}
	bool operator == (const whPair& o1) const
	{
		return ((this->h == o1.h) && (this->w == o1.w));
	}
	bool operator<(const whPair& wh) const
	{
		return (h < wh.h) || ((h == wh.h) && (w < wh.w));
	}
	whPair operator=(const whPair& wh)
	{
		this->h = wh.h;
		this->w = wh.w;
		return *this;
	}
	int h;
	int w;
};

class item
{
public:
	item(const int t_idx, const int t_width, const int t_height);
	int idx;
	int width;
	int height;
	int idxHelper;						// an alternative identifier
	static std::ostringstream ss;
};


struct coordinate
{
public:
	coordinate(int t_x, int t_y) : x(t_x), y(t_y) {};
	bool operator  == (const coordinate& cor1) const
	{
		return ((x == cor1.x) && (y == cor1.y));
	}
	bool operator<(const coordinate& cor1) const
	{
		return (x < cor1.x || (x == cor1.x) && (y < cor1.y));
	}
	coordinate operator=(const coordinate& cor1)
	{
		x = cor1.x;
		y = cor1.y;
		return *this;
	}
	int x;
	int y;
};

constexpr int BigNumber = 999999;
std::list<int> computeFX(const int t_x, const int t_idx, const std::vector<const item*>& t_items, bool flag);
int getMaximalHeight(const std::vector<const item*>& t_items);



/*
Instance of the SPP
*/
double solve(const std::vector<const item*>& t_allItems, const std::map<int, std::list<int>>& t_mapPosWidth,
	const std::map<int, std::list<int>>& t_mapPosHeight);


inline const std::string getVarName(const int t_itemIdx, const int t_xPos);

const std::string getVarName(const int t_itemIdx, const int t_xPos)
{
	item::ss.str("");
	item::ss.clear();
	item::ss << "item" << t_itemIdx << "assign" << t_xPos;
	return item::ss.str();
}



/*
Utilities for strip packing algorithms
*/
inline bool compareItemByWidth(const item* t_i, const item* t_j);
bool compareItemByWidth(const item* t_i, const item* t_j)
{
	return (t_i->width > t_j->width || (t_i->width == t_j->width && t_i->height > t_j->height) ||
		(t_i->width == t_j->width && t_i->height == t_j->height && t_i->idx > t_j->idx));
}

inline bool compareItemByHeight(const item* t_i, const item* t_j);
bool compareItemByHeight(const item* t_i, const item* t_j)
{
	return t_i->height < t_j->height || (t_i->height == t_j->height && t_i->idx > t_j->idx);
}

inline bool compareItemByIdx(const item* t_i, const item* t_j);
bool compareItemByIdx(const item* t_i, const item* t_j)
{
	return t_i->idx > t_j->idx;
}

class compareItemByWHDifference
{
public:
	const int _H;
	const int _W;
	compareItemByWHDifference(const int t_H, const int t_W) : _H(t_H), _W(t_W) {}
	bool operator()(const item* t_i, const item*  t_j) const
	{
		return  std::min(_W - t_i->width, _H - t_i->height) > std::min(_W - t_j->width, _H - t_j->height);
	}
};

class compareItemByHeight
{
	// for heap
public:
	bool operator()(const item* t_i, const item*  t_j) const
	{
		return  t_i->height > t_j->height || (t_i->height == t_j->height && t_i->idx > t_j->idx);
	}
};











