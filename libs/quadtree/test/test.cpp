#include <iostream>
#include <vector>
#include <quadtree/quadtree.hpp>
#include <random>
#include <chrono>

#define clamp(x, y, z) (std::min(std::max(x, y), z))

using namespace quadtree;
using namespace std;

/*struct Body {
	float x, y;
	float mass;
};

struct Reducer : TreeReducer<Body, 4> {
	unsigned int assignBody(const Body &body, int depth) {
		return assign(pair<float, float>(body.x, body.y), depth);
	}

	Body reduceBody(const Body bodies[4]) {
		Body out{0.f,0.f,0.f};
		for (int i = 0; i < 4; i++) {
			out.x += bodies[i].x / 4;
			out.y += bodies[i].y / 4;
			out.mass += bodies[i].mass / 4;
		}
		return out;
	}

	float distance(Body a, Body b) {
		return (a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y);
	}
};*/

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
//	Reducer reducer;
//	QuadTree<Body, 4> tree(reducer);
//	vector<Body> bodies;
//
//	random_device rd{};
//	mt19937 gen{rd()};
//	normal_distribution<float> norm(0.f, 0.3f);
//
//	for (int i = 0; i < 100; i++) {
//		Body rand{.x=clamp(norm(gen), -1.f, 1.f), .y=clamp(norm(gen), -1.f, 1.f), .mass=pow(norm(gen), 2.f) * 100};
//		bodies.push_back(rand);
//	}
//
//	tree.index(bodies);
//
//	cout << tree.nearest(bodies[0])->x << tree.nearest(bodies[0])->y << endl;
//
//	// Compare speed of getting the nearest body from quadtree vs normal
//
//	for (int i = 0; i < 100; i++) {
//
//	}
//
	QuadTree<> tree(2);
	vector<pairf*> points;

	random_device rd{};
	mt19937 gen{rd()};
	normal_distribution<float> norm(0.f, 0.3f);

	for (int i = 0; i < 30000; i++) {
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

		// Add the amount of time for a slow query
		start = chrono::system_clock::now();
		pairf* nearestSlow = slowNearest(*points[i], points);
		end = chrono::system_clock::now();
		slowTime += end - start;

		if (nearest != points[i] || nearestSlow != points[i]) {
			cout << "Wrong nearest on index " << i << endl;
			cout << nearest->first << " vs " << points[i]->first << ", " << nearest->second << ", " << points[i]->second << endl;
		}
	}
	end = chrono::system_clock::now();
	treeTime += end - start;
	cout << "Queried all in " << treeTime.count() << "s using tree" << endl;
	cout << "           and " << slowTime.count() << "s using loop" << endl;


	return 0;
}
