#include "simulation.hpp"
#include "quadtree/quadtree.hpp"
#include <algorithm>
#include <stack>
#include <memory>

float simulation::distance(const Body &a, const Body &b) {
	float A = (a.position.first - b.position.first);
	float B = (a.position.second - b.position.second);
	return std::max(A * A + B * B - a.radius * a.radius - b.radius * b.radius, 0.f);
}

bool simulation::inBounds(const Body &x, const Bounds &b) {
	return quadtree::inBounds(x.position, b);
}

float simulation::minDistance(const Body &x, const Bounds &b) {
	return quadtree::minDistance(x.position, b) - x.radius * x.radius;
}

void simulation::Simulation::step(float time, int maxCollisions) {
	#pragma omp parallel for
	for (int i = 0; i < data.size(); i++) {
		data[i].position.first += data[i].velocity.first * time;
		data[i].position.second += data[i].velocity.second * time;
	}
	points.reindex();


	// Check for collisions between bodies and handle them
	// TODO: Allow for multiple collisions per body
	/*#pragma omp parallel for
	for (int i = 0; i < data.size(); i++) {
		auto nearest = *points.nearest(data[i], 1).begin();
		if (nearest.first <= 0.f) collisions[i] = nearest.second;
		else collisions[i] = nullptr;
	}

	// TODO: It is more efficient to reindex the entire tree if there are many collisions.
	// 		 This should be handled efficiently
	// TODO: Parallelize handleCollision. It might be as simple as making every body property atomic.
	for (int i = 0; i < data.size(); i++) {
		if (collisions[i]) handleCollision(&data[i], collisions[i]);
	}*/
}

void simulation::Simulation::handleCollision(Body *a, Body *b) {
	// Do nothing for now
}

void simulation::Simulation::addBody(Body &point) {
	data.push_back(point);
	points.insert(point, points.root);
}
