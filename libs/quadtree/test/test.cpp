#include <iostream>
#include <vector>
#include <quadtree/quadtree.hpp>
#include <random>
#include <chrono>

//#define COMPARE_SLOW
#define clamp(x, y, z) (std::min(std::max(x, y), z))

using namespace quadtree;
using namespace std;

pairf *slowNearest(pairf point, vector<pairf*> points) {
	float dist = INFINITY;
	pairf *nearest;
	for (int i = 0; i < points.size(); i++) {
		float cdist = distance(point, *points[i]);
		if (cdist < dist) {
			dist = cdist;
			nearest = points[i];
		}
	}

	return nearest;
}

int main() {
	QuadTree<> tree(2);
	vector<pairf*> points;

	random_device rd{};
	mt19937 gen{rd()};
	normal_distribution<float> norm(0.f, 0.3f);

	for (int i = 0; i < 100000; i++) {
		pair<float, float> *p = new pair<float, float>(clamp(norm(rd), -1.f, 1.f), clamp(norm(rd), -1.f, 1.f));
		points.push_back(p);
	}
	
	// Time indexing the array
	auto start = chrono::system_clock::now();
	for (int i = 0; i < points.size(); i++) {
		tree.insert(*points[i], tree.root);
	}
	auto end = chrono::system_clock::now();
	chrono::duration<double> seconds = end - start;
	cout << "Indexed in " << seconds.count() << "s" << endl;


	chrono::duration<double> treeTime;
	chrono::duration<double> slowTime;
	start = chrono::system_clock::now();
	for (int i = 0; i < points.size(); i++) {
		// Add the amount of time to query the tree
		start = chrono::system_clock::now();
		pairf* nearest = tree.nearest(*points[i]);
		end = chrono::system_clock::now();
		treeTime += end - start;

#ifdef COMPARE_SLOW
		// Add the amount of time for a slow query
		start = chrono::system_clock::now();
		pairf* nearestSlow = slowNearest(*points[i], points);
		end = chrono::system_clock::now();
		slowTime += end - start;

		if (nearest != points[i] || nearestSlow != points[i]) {
			cout << "Wrong nearest on index " << i << endl;
			cout << nearest->first << " vs " << points[i]->first << ", " << nearest->second << ", " << points[i]->second << endl;
		}
#endif
	}
	end = chrono::system_clock::now();
	treeTime += end - start;
	cout << "Queried all in " << treeTime.count() << "s using tree" << endl;
#ifdef COMPARE_SLOW
	cout << "           and " << slowTime.count() << "s using loop" << endl;
#endif


	return 0;
}
