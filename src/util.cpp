#include "util.h"
#include <iostream>

int myRandom::counter = 0;
int myRandom::fCounter = 0;
std::mt19937 myRandom::myEngine(std::chrono::system_clock::now().time_since_epoch().count());

std::ofstream outFile::vrp_ofs("VRP_solution.output");
void myRandom::updateEngine()
{
	counter++;
	myEngine = std::mt19937(std::chrono::system_clock::now().time_since_epoch().count() + counter);
}

/*
The argument t_distribution is a vector of double-type which is the selection probability of choosing the corresponding component
Here we implement a mechanism similar to Roulette to sample out a position index ranging from 0 to t_distribution.size()-1
*/
int myRandom::sampleOwnDistribution(const std::vector<double>& t_distribution)
{
	std::uniform_real_distribution<double> distribution(0, 1);
	double sample = distribution(myRandom::myEngine);
	double accum = 0.0;
	for (size_t i = 0; i < t_distribution.size(); ++i)
	{
		accum += t_distribution[i];
		if (sample <= accum)
		{
			return i;
		}
	}
	return t_distribution.size() - 1;
}

double myMathsLib::compute_distance(double x1, double y1, double x2, double y2)
{
	//return std::sqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2));
	double tmp = std::sqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2));
	char sz[64];
	sprintf_s(sz, "%.1lf\n", tmp); 
	double result = std::atof(sz);
	return result;
}

void splitString(std::string& t_str, const std::string& t_sep, std::vector<std::string>& t_splitted)
{
	size_t found;
	found = t_str.find(t_sep);
	while (found != std::string::npos)
	{
		t_splitted.push_back(t_str.substr(0, found));
		t_str = t_str.substr(found + 1);
		found = t_str.find(t_sep);
	}
	t_splitted.push_back(t_str);
}



int subSetSum(const std::vector<int>& t_v, const int t_limit)
{
	int** values = new int*[t_v.size()+1];
	for (size_t i = 0; i < t_v.size()+1; ++i)
	{
		values[i] = new int[t_limit+1];
		if (i == 0)
		{
			for (size_t j = 0; j < t_limit + 1; ++j) values[i][j] = 0;
		}
		values[i][0] = 0;
	}

	for (size_t i = 1; i < t_v.size()+1; ++i)
	{
		for (size_t j = 1; j < t_limit + 1; ++j)
		{
			if (t_v[i - 1] > j) values[i][j] = values[i - 1][j];
			else values[i][j] = std::max(values[i - 1][j - t_v[i - 1]] + t_v[i - 1], values[i-1][j]);
		}
	}
	int result = values[t_v.size()][t_limit];
	for (size_t i = 0; i < t_v.size(); ++i) delete values[i];
	delete values;
	return result;
}
