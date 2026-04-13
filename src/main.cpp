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
			<< "Usage: spp [--time_buget <seconds>] [--problem_path <path>]\n"
			<< "  --time_buget: solver time limit in seconds (default: 60)\n"
			<< "  --problem_path: path to a single .TXT instance file (default: ./test/2sp/HT01.TXT)\n"
			<< "Examples:\n"
			<< "  spp\n"
			<< "  spp --time_buget 120 --problem_path ./test/2sp/HT01.TXT\n"
			<< "  spp --problem_path ./test/2sp/HT01.TXT\n";
	};

	std::string problemPath = "";
	int solverTimeLimitSeconds = 60;

	for (int i = 1; i < argc; ++i)
	{
		const std::string arg = argv[i];
		if (arg == "--time_buget")
		{
			if (i + 1 >= argc)
			{
				std::cerr << "Missing value for --time_buget\n";
				return 1;
			}
			solverTimeLimitSeconds = std::atoi(argv[++i]);
			if (solverTimeLimitSeconds <= 0)
			{
				std::cerr << "Invalid --time_buget value, it must be a positive integer\n";
				return 1;
			}
			continue;
		}
		if (arg == "--problem_path")
		{
			if (i + 1 >= argc)
			{
				std::cerr << "Missing value for --problem_path\n";
				return 1;
			}
			problemPath = argv[++i];
			continue;
		}

		std::cerr << "Unknown argument: " << arg << "\n";
		printUsage();
		return 1;
	}

	// a lambda function to solve one instance.
	auto solveOne = [&](const std::string& filePath) -> int {
		std::vector<const StripPacking::item*> allItems;
		int W = readData(filePath, allItems);
		StripPacking::BLEU::algStatus = StripPacking::algorithmStatus::exact;
		StripPacking::BLEU alg(allItems, W, solverTimeLimitSeconds);
		const int height = alg.takeOff();
		for (auto it = allItems.begin(); it != allItems.end(); ++it)
			delete (*it);
		return height;
	};
	if (!std::filesystem::exists(problemPath))
	{
		std::cerr << "Input file not found: " << problemPath << "\n";
		return 1;
	}

	if (!std::filesystem::is_regular_file(problemPath))
	{
		std::cerr << "Input path must be a single file: " << problemPath << "\n";
		return 1;
	}

	if (std::filesystem::path(problemPath).extension() != ".TXT")
	{
		std::cerr << "Input file must have .TXT extension: " << problemPath << "\n";
		return 1;
	}

	const int height = solveOne(problemPath);
	if (height >= 0)
	{
		std::cout << "state: completed\n";
		std::cout << "height: " << height << "\n";
	}
	else
	{
		std::cout << "state: timeout\n";
	}
	return 0;
}
