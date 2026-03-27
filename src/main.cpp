#include "datareader.h"
#include <filesystem>
#include <string>
#include "spp.h"
#include <iostream>
#include <map>
#include "BLEU.h"
#include "util.h"
#include "heuristic.h"
#include <cstdlib>

int main(int argc, char** argv)
{
	auto printUsage = []() {
		std::cout
			<< "Usage: spp [folder] [single] [single_instance_path]\n"
			<< "  folder: instance folder (default: ./test/2sp)\n"
			<< "  single: 0/1, solve one instance only when 1 (default: 0)\n"
			<< "  single_instance_path: path to a single instance, required when single=1\n"
			<< "Examples:\n"
			<< "  spp\n"
			<< "  spp ./test/2sp\n"
			<< "  spp ./test/2sp 1 ./test/2sp/HT01.TXT\n";
	};

	if (argc >= 2)
	{
		const std::string arg1 = argv[1];
		if (arg1 == "--help" || arg1 == "-h")
		{
			printUsage();
			return 0;
		}
	}

	std::string instancesFolder = "./test/2sp";
	bool runSingle = false;
	std::string singleInstancePath;

	if (argc >= 2) instancesFolder = argv[1];
	if (argc >= 3) runSingle = (std::atoi(argv[2]) == 1);
	if (argc >= 4) singleInstancePath = argv[3];
	if (runSingle && singleInstancePath.empty())
	{
		std::cerr << "single=1 requires arg3: path to single instance\n";
		return 1;
	}

	const int solverTimeLimitSeconds = 60;

	// a lambda function to solve one instance.
	auto solveOne = [&](const std::string& filePath) {
		std::vector<const StripPacking::item*> allItems;
		StripPacking::Heuristic hrs;
		int W = readData(filePath, allItems);
		std::vector<const StripPacking::item*> copyItems(allItems.begin(), allItems.end());
		int totalArea = 0;
		StripPacking::BLEU::algStatus = StripPacking::algorithmStatus::exact;
		StripPacking::BLEU alg(allItems, W, solverTimeLimitSeconds);
		auto height = alg.takeOff();
		for (auto it = allItems.begin(); it != allItems.end(); ++it)
			delete (*it);
	};
	if (runSingle)
	{
		if (!std::filesystem::exists(singleInstancePath))
		{
			std::cerr << "Single instance not found: " << singleInstancePath << "\n";
			return 1;
		}
		solveOne(singleInstancePath);
		return 0;
	}

	if (!std::filesystem::exists(instancesFolder))
	{
		std::cerr << "Input folder not found: " << instancesFolder << "\n";
		return 1;
	}

	for (const auto& entry : std::filesystem::directory_iterator(instancesFolder))
	{
		if (!entry.is_regular_file() || entry.path().extension() != ".TXT") continue;
		std::string filePath = entry.path().string();
		solveOne(filePath);
	}
	return 0;
}
