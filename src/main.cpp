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
		std::vector<const StripPacking::item*> allItems;
		int W = readData(filePath, allItems);
		StripPacking::BLEU alg(allItems,W, 5);
		int h = alg.takeOff();
		std::cout << "preprocess takes " << XYZTimer::timerPreprocess
			<< "," << "B&B takes " << XYZTimer::timerBB
			<<","<<"BD takes "<<XYZTimer::timerBD
			<<","<<"Metaheuristic takes "<<XYZTimer::timerMetaH<<std::endl;
		std::cout << "heigh is " << h << std::endl;
		XYZTimer::reset();
		//std::cout<<"possible height is" <<alg.takeOff()<<std::endl;
		for (auto it = allItems.begin(); it != allItems.end(); ++it)
			delete (*it);
	}
	system("pause");
}