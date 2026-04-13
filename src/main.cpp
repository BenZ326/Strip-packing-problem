#include "datareader.h"
#include <filesystem>
#include <string>
#include "spp.h"
#include <iostream>
#include "BLEU.h"
#include <exception>
#include <stdexcept>

namespace {
int parseNonNegativeIntArg(const std::string& name, const char* raw)
{
	try
	{
		std::string text(raw);
		size_t consumed = 0;
		const int value = std::stoi(text, &consumed);
		if (consumed != text.size() || value < 0)
		{
			throw std::invalid_argument("invalid");
		}
		return value;
	}
	catch (const std::exception&)
	{
		throw std::runtime_error("Invalid " + name + " value, it must be a non-negative integer");
	}
}
}

int main(int argc, char** argv)
{
	auto printUsage = []() {
		std::cout
			<< "Usage: spp [--time_buget <seconds>] [--problem_path <path>]\n"
			<< "  --time_buget / --time_budget: solver time limit in seconds (default: 60)\n"
			<< "  --problem_path: path to a single .TXT instance file (default: ./test/2sp/HT01.TXT)\n"
			<< "Examples:\n"
			<< "  spp\n"
			<< "  spp --time_buget 120 --problem_path ./test/2sp/HT01.TXT\n"
			<< "  spp --problem_path ./test/2sp/HT01.TXT\n";
	};

	std::string problemPath = "./test/2sp/HT01.TXT";
	int solverTimeLimitSeconds = 60;

	for (int i = 1; i < argc; ++i)
	{
		const std::string arg = argv[i];
		if (arg == "--time_buget" || arg == "--time_budget")
		{
			if (i + 1 >= argc)
			{
				std::cerr << "Missing value for " << arg << "\n";
				return 1;
			}
			try
			{
				solverTimeLimitSeconds = parseNonNegativeIntArg(arg, argv[++i]);
			}
			catch (const std::exception& ex)
			{
				std::cerr << ex.what() << "\n";
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
		struct ItemGuard {
			std::vector<const StripPacking::item*>& items;
			~ItemGuard() {
				for (auto* item : items) delete item;
			}
		};

		std::vector<const StripPacking::item*> allItems;
		ItemGuard guard{ allItems };
		int W = readData(filePath, allItems);
		StripPacking::BLEU::algStatus = StripPacking::algorithmStatus::exact;
		StripPacking::BLEU alg(allItems, W, solverTimeLimitSeconds);
		return alg.takeOff();
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

	if (solverTimeLimitSeconds == 0)
	{
		std::cout << "state: timeout\n";
		return 0;
	}

	int height = -1;
	try
	{
		height = solveOne(problemPath);
	}
	catch (const std::exception& ex)
	{
		std::cerr << ex.what() << "\n";
		return 1;
	}

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
