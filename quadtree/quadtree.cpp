#include "quadtree.hpp"

#define clamp(x, l, h) (std::min(std::max(x, l), h))

using pairf = std::pair<float, float>;

float quadtree::distance(const std::pair<float, float> &a, const std::pair<float, float> &b) {
	return (a.first - b.first) * (a.first - b.first) + (a.second - b.second) * (a.second - b.second);
}

bool quadtree::inBounds(const pairf &data, const std::pair<pairf, pairf> &bounds) {
	return data.first >= bounds.first.first && data.second >= bounds.first.second
		&& data.first <= bounds.second.first && data.second <= bounds.second.second;
}

float quadtree::minDistance(const pairf &point, const std::pair<pairf, pairf> &bounds) {
	// Find the closest point and return the distance
	pairf closestPoint = std::pair<float, float>(
		clamp(point.first, bounds.first.first, bounds.second.first),
		clamp(point.second, bounds.first.second, bounds.second.second)
	);

	return quadtree::distance(point, closestPoint);
}

// Return the bounding box of child node i
std::pair<pairf, pairf> quadtree::getBounds(const std::pair<pairf, pairf> &data, int i) {
	pairf size((data.second.first - data.first.first) / 2.f, (data.second.second - data.first.second) / 2.f);

	pairf start(size.first * (float)(i & 1) + data.first.first, size.second * (float)((i >> 1) & 1) + data.first.second);

	return { { start.first, start.second }, { start.first + size.first, start.second + size.second } };
}
