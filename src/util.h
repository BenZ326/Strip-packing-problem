#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <algorithm>

inline int subSetSum(const std::vector<int>& t_values, int t_cap)
{
	if (t_cap <= 0) return 0;
	std::vector<char> reachable(static_cast<size_t>(t_cap) + 1, 0);
	reachable[0] = 1;
	for (int v : t_values)
	{
		if (v <= 0) continue;
		for (int s = t_cap; s >= v; --s)
		{
			if (reachable[static_cast<size_t>(s - v)]) reachable[static_cast<size_t>(s)] = 1;
		}
	}
	for (int s = t_cap; s >= 0; --s)
	{
		if (reachable[static_cast<size_t>(s)]) return s;
	}
	return 0;
}

inline void splitString(const std::string& t_str, const std::string& t_sep, std::vector<std::string>& t_out)
{
	t_out.clear();
	if (t_sep.empty())
	{
		t_out.push_back(t_str);
		return;
	}
	size_t start = 0;
	while (start <= t_str.size())
	{
		size_t pos = t_str.find(t_sep, start);
		if (pos == std::string::npos)
		{
			t_out.push_back(t_str.substr(start));
			break;
		}
		t_out.push_back(t_str.substr(start, pos - start));
		start = pos + t_sep.size();
	}
}
