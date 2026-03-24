#include "datareader.h"
#include <filesystem>
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
	const int solverTimeLimitSeconds = 60;
	if (!std::filesystem::exists(instancesFolder))
	{
		std::cerr << "Input folder not found: " << instancesFolder << "\n";
		return 1;
	}
	for (const auto& entry : std::filesystem::directory_iterator(instancesFolder))
	{
		std::string filePath = entry.path().relative_path().string();
		std::cout << filePath << "\n";
		
		std::vector<const StripPacking::item*> allItems;
		StripPacking::Heuristic hrs;
		int W = readData(filePath, allItems);
		std::vector<const StripPacking::item*> copyItems(allItems.begin(), allItems.end());
		int totalArea = 0;
		StripPacking::BLEU::algStatus = StripPacking::algorithmStatus::exact;
	    StripPacking::BLEU alg(allItems, W, solverTimeLimitSeconds);
		auto height = alg.takeOff();
		if (height >= 0)
		{
			std::cout << "best_height=" << height << " status=feasible";
		}
		else
		{
			std::cout << "best_height=unknown status=pending_or_timeout";
		}
		std::cout << " alg_status=" << static_cast<int>(StripPacking::BLEU::algStatus) << "\n";
		#ifdef DUMP_SOL
		alg.dumpSolution(entry.path().filename().string());
		#endif
		for (auto it = allItems.begin(); it != allItems.end(); ++it)
			delete (*it);
	}
	return 0;
}
