#ifndef _UTIL_H_
#define _UTIL_H_

#include <chrono>
#include <random>
#include <fstream>
#include <numeric>
#include <set>
#include<string>
#include <vector>
#include <random>
namespace myMathsLib
{
	// computate the distance between point (x1, y1) and point (x2, y2)
	double compute_distance(double x1, double y1, double x2, double y2);
	//calculate mean, standard variance, sum of a vector of float, int, double 
	template<class T>
	float getMean(const std::vector<T>& t_nums);
	template<class T>
	float getStd(const std::vector<T>& t_nums);
	template<class T>
	float getSum(const std::vector<T>& t_nums);
}

template<class T>
float myMathsLib::getMean(const std::vector<T>& t_nums)
{
	return getSum(t_nums)/t_nums.size();
}

template<class T>
float myMathsLib::getStd(const std::vector<T>& t_nums)
{
	float mean = getMean(t_nums);
	float variance = 0.0;
	for (size_t i = 0; i < t_nums.size(); ++i)
	{
		variance += (t_nums[i] - mean)*(t_nums[i]-mean);
	}
	return std::sqrt(variance / (t_nums.size()));
}

template<class T>
float myMathsLib::getSum(const std::vector<T>& t_nums)
{
	return std::accumulate(t_nums.begin(), t_nums.end(), 0.0);
}


template<class T>
class Pair
{
public:
	Pair(const T t_x, const T t_y);
	T x;
	T y;
};
template<class T>
Pair<T>::Pair(const T t_x, const T t_y) :x(t_x), y(t_y)
{
}

/*
mean function for use
*/
template <class T>
T mean(const std::vector<T>& t_data);

template <class T>
T mean(const std::vector<T>& t_data)
{
	T sum = 0.0;
	for (size_t i = 0; i != t_data.size(); ++i)
		sum += t_data[i];
	return sum / t_data.size();

}

class myRandom
{
public:
	myRandom() {};
	static int counter;
	static int fCounter;
	static std::mt19937 myEngine;
	static void updateEngine();
	static int sampleOwnDistribution(const std::vector<double>& t_distribution);
};

class outFile
{
public:
	static std::ofstream vrp_ofs;
};


template<class T>
class ObjectScore
{
public:
	ObjectScore(const T* t_obj, const double t_score) :_score(t_score)
	{
		_obj = t_obj;
	}
	const T* _obj;
	double _score;
};

template<class T>
bool compareObjSc(const T& t_objScore1, const T& t_objScore2);

template<class T>
inline bool compareObjSc(const T& t_objScore1, const T& t_objScore2)
{
	return t_objScore1._score < t_objScore2._score;
}


void splitString(std::string& t_str, const std::string& t_sep, std::vector<std::string>& t_splitted);


/*
A simple dynamic programming for a standard subset sum problem
Given a set of integers, return a subset of the integers so that the sum of the subset is maximal and not exceeding t_limit
*/
int subSetSum(const std::vector<int>& t_v, const int t_limit);



#endif