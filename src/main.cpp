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
		int W = readData(filePath, allItems);
		BLEU alg(allItems,W);
		std::cout<<"possible height is" <<alg.takeOff()<<std::endl;
		for (auto it = allItems.begin(); it != allItems.end(); ++it)
			delete (*it);
	}
	system("pause");
}