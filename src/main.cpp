#include "datareader.h"
#include <experimental/filesystem>
#include <string>
#include "spp.h"
#include <iostream>
#include <map>
#include "BLEU.h"
#include "util.h"
#include "heuristic.h"
int main()
{
	const std::string instancesFolder = "./2sp/";
	for (int i = 0; i < 1; ++i) {

	
	for (const auto& entry : std::experimental::filesystem::directory_iterator(instancesFolder))
	{
		std::string filePath = entry.path().relative_path().string();
		std::cout << filePath;
		std::vector<const StripPacking::item*> allItems;
		StripPacking::Heuristic hrs;
		int W = readData(filePath, allItems);
		std::vector<const StripPacking::item*> copyItems(allItems.begin(), allItems.end());
		int totalArea = 0;
		const StripPacking::item* bin1 = new StripPacking::item(10, 5, 2);
		std::vector<const StripPacking::item*> bins;
		bins.push_back(bin1);
		bool res = hrs.generalBestFitHeurisitic(copyItems, bins);
		auto str = res ? "can" : "cannot";
		std::cout << "\n the bin " <<  str<< " contain the items\n";
	/*	StripPacking::BLEU alg(allItems,W, 2.0);
		auto optimalHeight = alg.takeOff();
		std::cout << "optimal height is " << optimalHeight<<"\n";*/
		delete bin1;
		for (auto it = allItems.begin(); it != allItems.end(); ++it)
			delete (*it);
	}
	}
	system("pause");
}