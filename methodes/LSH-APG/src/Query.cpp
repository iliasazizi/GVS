#include "Query.h"
#include "basis.h"
#include <iostream>
#include <set>
#include <queue>
#include <algorithm>
#include <vector>

void Performance::update(queryN* query, Preprocess& prep)
{
	num++;
	cost += query->cost;
	timeHash += query->timeHash;
	timeSift += query->timeSift;
	timeVerify += query->timeVerify;
	timeTotal += query->timeTotal;
	prunings += query->prunings;
	maxHop += query->maxHop;
	if (costs.size() == 0) {
		costs.resize(query->costs.size(),0);
	}
	for (int i = 0; i < query->costs.size(); ++i) {
		costs[i] += query->costs[i];
	}

	unsigned num0 = query->res.size();
	if (num0 > query->k)
		num0 = query->k;
	resNum += num0;

	std::set<unsigned> set1, set2;
	std::vector<unsigned> set_intersection;
	set_intersection.clear();
	set1.clear();
	set2.clear();

	for (unsigned j = 0; j < num0; j++)
	{
		//if(query->res[0])
#ifdef USE_SQRDIST
		float dist = sqrt(query->res[j].dist);
#else
		float dist = query->res[j].dist;
#endif

		if (prep.benchmark.dist[query->flag][j] == 0) {
			ratio += 1.0f;
		} else {
			ratio += sqrt(query->res[j].dist) / sqrt(prep.benchmark.dist[query->flag][j]);
		}
	}

	for (unsigned j = num0-1; j >= 0; j--)
	{
		// std::cout << query->res[j].dist << "   " << sqrt(prep.benchmark.dist[query->flag][num0-1]) << std::endl;
		if (sqrt(query->res[j].dist) <= sqrt(prep.benchmark.dist[query->flag][num0-1])){
			NN_num += (j+1);
			break;
		}
	}
}

Performance::~Performance()
{

}


