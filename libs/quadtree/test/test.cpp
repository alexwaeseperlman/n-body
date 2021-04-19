#include <iostream>
#include <vector>
#include <quadtree/quadtree.hpp>
#include <random>
#include <chrono>

#define COMPARE_SLOW false
#define CHECK_ANSWERS true
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
	QuadTree<> tree(80);
	vector<pairf> points;

	random_device rd{};
	mt19937 gen{rd()};
	normal_distribution<float> norm(0.f, 0.2f);

	for (int i = 0; i < 1000000; i++) {
		pair<float, float> p(clamp(norm(rd), -.8f, .8f), clamp(norm(rd), -.8f, .8f));
		points.push_back(p);
	}
	
	// Time indexing the array
	auto start = chrono::system_clock::now();
	tree.initialize(points);
	auto end = chrono::system_clock::now();
	chrono::duration<double> seconds = end - start;
	cout << "Indexed in " << seconds.count() << "s" << endl;

	// Time moving each element
	start = chrono::system_clock::now();
	for (int i = 0; i < points.size(); i++) {
		auto delta = points[points.size() - i - 1];
		points[i].first += delta.first * 0.1f;
		points[i].second += delta.second * 0.1f;

		tree.update(points[i]);
	}
	end = chrono::system_clock::now();
	seconds = end - start;
	cout << "Moved all in " << seconds.count() << "s" << endl;

	// Time moving each element
	for (int i = 0; i < points.size(); i++) {
		auto delta = points[points.size() - i - 1];
		points[i].first += delta.first * 0.1f;
		points[i].second += delta.second * 0.1f;
	}
	start = chrono::system_clock::now();
	tree.reindex();
	end = chrono::system_clock::now();
	seconds = end - start;
	cout << "Reindexed in " << seconds.count() << "s" << endl;

	chrono::duration<double> treeTime;
	chrono::duration<double> slowTime;

	// Tree benchmark
	start = chrono::system_clock::now();
#pragma omp parallel for
	for (int i = 0; i < points.size(); i++) {
		pairf const* nearest = tree.nearest(points[i]);

#if CHECK_ANSWERS
		if (nearest != &points[i]) {
			cout << "Wrong nearest on index " << i << endl;
			cout << nearest->first << " vs " << points[i].first << ", " << nearest->second << ", " << points[i].second << endl;
		}
#endif
	}
	end = chrono::system_clock::now();
	treeTime = end - start;

	cout << "Queried all in " << treeTime.count() << "s using tree" << endl;


	// Slow benchmark
#if COMPARE_SLOW
	start = chrono::system_clock::now();
#pragma omp parallel for
	for (int i = 0; i < points.size(); i++) {
		// Add the amount of time for a slow query
		pairf* nearest = slowNearest(*points[i], points);
		slowTime += end - start;
		
#if CHECK_ANSWERS
		if (nearest != points[i]) {
			cout << "Wrong nearest on index " << i << endl;
			cout << nearest->first << " vs " << points[i]->first << ", " << nearest->second << ", " << points[i]->second << endl;
		}
#endif
	}
	end = chrono::system_clock::now();
	slowTime = end - start;

	cout << "           and " << slowTime.count() << "s using loop" << endl;
#endif

	return 0;
}
