#include "datareader.h"
#include <experimental/filesystem>
#include <string>
#include "spp.h"
#include <iostream>
#include <map>
#include "BLEU.h"
#include "util.h"
int main()
{
	const std::string instancesFolder = "./2sp/";
	for (const auto& entry : std::experimental::filesystem::directory_iterator(instancesFolder))
	{
		std::string filePath = entry.path().relative_path().string();
		std::cout << filePath;
		std::vector<const item*> allItems;
		int W= readData(filePath, allItems);
		int maxHeight = getMaximalHeight(allItems);
		BLEU alg(allItems, W);
		alg.preprocessing();
		alg.bounds();
		std::cout << std::endl;
		//std::map<int, std::list<int>> mapPosWidth, mapPosHeight;
		//for (size_t idxItem = 0; idxItem < allItems.size(); ++idxItem)
		//{
		//	auto possiblePositionsWidth = computeFX(W - allItems[idxItem]->width, idxItem, allItems, true);
		//	auto possiblePositionsHeight = computeFX(H - allItems[idxItem]->height, idxItem, allItems, false);
		//	mapPosWidth.insert(std::pair<int, std::list<int>>(allItems[idxItem]->idx, possiblePositionsWidth));
		//	mapPosHeight.insert(std::pair<int, std::list<int>>(allItems[idxItem]->idx, possiblePositionsHeight));
		//}
		//solve(allItems, mapPosWidth, mapPosHeight);
		//break;
	}
	system("pause");
}