#include "datareader.h"
#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <memory>
#include <stdexcept>
#include "spp.h"

namespace {
int parseSingleIntegerLine(const std::string& line, const std::string& fieldName)
{
	std::istringstream iss(line);
	int value = 0;
	std::string extra;
	if (!(iss >> value) || (iss >> extra))
	{
		throw std::runtime_error("Invalid " + fieldName + " line: \"" + line + "\"");
	}
	return value;
}
}

int readData(const std::string& t_file, std::vector<const StripPacking::item*>& t_items)
{
	t_items.clear();
	std::ifstream ff(t_file);
	if (!ff.is_open())
	{
		throw std::runtime_error("Cannot open input file: " + t_file);
	}

	std::string line;
	if (!std::getline(ff, line))
	{
		throw std::runtime_error("Missing item count line in file: " + t_file);
	}
	const int expectedItemCount = parseSingleIntegerLine(line, "item count");
	if (expectedItemCount <= 0)
	{
		throw std::runtime_error("Item count must be positive in file: " + t_file);
	}

	if (!std::getline(ff, line))
	{
		throw std::runtime_error("Missing strip width line in file: " + t_file);
	}
	const int maxWidth = parseSingleIntegerLine(line, "strip width");
	if (maxWidth <= 0)
	{
		throw std::runtime_error("Strip width must be positive in file: " + t_file);
	}

	std::vector<std::unique_ptr<StripPacking::item>> parsedItems;
	parsedItems.reserve(static_cast<size_t>(expectedItemCount));

	while (std::getline(ff, line))
	{
		if (line.empty())
		{
			continue;
		}

		std::istringstream iss(line);
		int itemId = 0;
		int width = 0;
		int height = 0;
		std::string extra;
		if (!(iss >> itemId >> width >> height) || (iss >> extra))
		{
			throw std::runtime_error("Invalid item row in file " + t_file + ": \"" + line + "\"");
		}
		if (width <= 0 || height <= 0)
		{
			throw std::runtime_error("Item width/height must be positive in file " + t_file + ": \"" + line + "\"");
		}
		parsedItems.push_back(std::make_unique<StripPacking::item>(itemId, width, height));
		if (parsedItems.size() > static_cast<size_t>(expectedItemCount))
		{
			throw std::runtime_error("Input file has more than n item rows: " + t_file);
		}
	}

	if (parsedItems.size() != static_cast<size_t>(expectedItemCount))
	{
		throw std::runtime_error("Input file has " + std::to_string(parsedItems.size()) + " item rows but n is "
			+ std::to_string(expectedItemCount) + ": " + t_file);
	}

	for (auto& it : parsedItems)
	{
		t_items.push_back(it.release());
	}
	return maxWidth;
}
