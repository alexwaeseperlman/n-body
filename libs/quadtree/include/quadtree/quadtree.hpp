#pragma once
#include <map>
#include <vector>
#include <math.h>
#include <queue>
#include <iostream>

namespace quadtree {
	template <class Data, class Bounds>
	struct TreeReducer {
		// Calculate the distance between two pieces of data
		float (*distance)(const Data &a, const Data &b);
		// Check if data is within a given bounding box
		bool (*inBounds)(const Data &data, const Bounds &bounds);
		// Get the minimum distance from a piece of data to a given bounding box
		float (*minDistance)(const Data &data, const Bounds &bounds);
		// Get the bounding box of a section based on its parent
		Bounds (*getBounds)(const Bounds &parent, int i);
	};

	using pairf = std::pair<float, float>;

	// Default tree reducer methods
	float distance(const pairf &a, const pairf &b);
	bool inBounds(const pairf &data, const std::pair<pairf, pairf> &bounds);
	float minDistance(const pairf &data, const std::pair<pairf, pairf> &bounds);
	std::pair<pairf, pairf> getBounds(const std::pair<pairf, pairf> &data, int i);

	template<class Data, class Bounds, unsigned int sections>
	struct TreeNode {
		Data **values;
		int stored = 0;
		int leafCount = 0;
		int binSize;
		TreeReducer<Data, Bounds> *reducer;

		Bounds bounds;

		bool container = false;
		TreeNode *children[sections];
		TreeNode(int binSize, bool container, Bounds bounds, TreeReducer<Data, Bounds> *reducer): binSize(binSize), bounds(bounds) {
			values = new Data*[binSize];
			this->reducer = reducer;
			if (container) makeContainer();
		}

		// Move all stored data into child nodes
		void makeContainer() {
			leafCount = stored;

			for (int i = 0; i < sections; i++) {
				auto b = reducer->getBounds(this->bounds, i);
				children[i] = new TreeNode<Data, Bounds, sections>(binSize, false, b, reducer);
			}

			for (int i = 0; i < stored; i++) {
				for (int j = 0; j < sections; j++) {
					if (reducer->inBounds(*values[i], children[j]->bounds)) {
						children[j]->values[children[j]->stored] = values[i];
						children[j]->stored++;
						break;
					}
				}
			}
			this->container = true;
		}

		~TreeNode() {
			if (container) for (int i = 0; i < sections; i++) delete children[i];
			delete values;
		}
	};

	
	template<class Data = pairf, class Bounds=std::pair<pairf, pairf>, unsigned int sections=4>
	class QuadTree {
	public:
		using Node=TreeNode<Data, Bounds, sections>;
		int binSize;
		QuadTree(
				int binSize,
				Bounds rootBounds=std::pair<pairf, pairf>{ {-1.f, -1.f}, {1.f, 1.f} },
				TreeReducer<Data, Bounds> reducer = {.distance=distance, .inBounds=inBounds, .minDistance=minDistance, .getBounds=getBounds}) 
			: binSize(binSize), rootBounds(rootBounds), reducer(reducer) {

			root = new Node(binSize, true, rootBounds, &this->reducer);
		}
		~QuadTree() {
			delete root;
		}

		Data* nearest(Data obj);
		std::map<float, Data*> nearest(Data obj, int n);

		Node* insert(Data &data, Node* node) const;
		void initialize(std::vector<Data> &data);

		TreeReducer<Data, Bounds> reducer;
		Node *root;
		Bounds rootBounds;
	};
}

template<class Data, class Bounds, unsigned int sections>
quadtree::TreeNode<Data, Bounds, sections>*
quadtree::QuadTree<Data, Bounds, sections>::insert(Data &data, Node *node) const {
	// Find a leaf node
	while (node->container) {
		// Take the first container that contains node in its bounding box
		for (int i = 0; i < sections; i++) {
			if (reducer.inBounds(data, node->children[i]->bounds)) {
				node->leafCount++;
				node = node->children[i];
				break;
			}
		}
	}
	node->leafCount++;

	// If possible store this in the nodes bin
	if (node->stored < node->binSize) {
		node->values[node->stored] = &data;
		node->stored++;
		return node;
	}
	// Reindex the node if necessary
	node->makeContainer();
	return insert(data, node);
}

template<class Data, class Bounds, unsigned int sections>
Data* quadtree::QuadTree<Data, Bounds, sections>::nearest(Data obj) {
	return nearest(obj, 1)[0];
}
template<class Data, class Bounds, unsigned int sections>
std::map<float, Data*> quadtree::QuadTree<Data, Bounds, sections>::nearest(Data obj, int n) {
	auto compare = [](std::pair<float, void*> l, std::pair<float, void*> r) {
		return l.first < r.first;
	};

	std::priority_queue<std::pair<float, Node*>, std::vector<std::pair<float, Node*>>, decltype(compare)> dfs(compare);
	dfs.emplace(0.f, root);
	
	float minDist = INFINITY;
	// Save memory by using a raw array instead of a set.
	// Closest items are stored as a heap and the largest one is removed every iteration
	std::pair<float, Data*>* closest = new std::pair<float, Data*>[n + 1];
	for (int i = 0; i < n + 1; i++) closest[i].first = INFINITY;
	std::make_heap(closest, closest + n + 1, compare);

	while (dfs.size() > 0) {
		auto top = dfs.top();
		Node* current = top.second;
		float dist = -top.first;
		dfs.pop();


		if (dist >= minDist) {
			continue;
		}
		// Queue up child nodes for search if the current node is a container
		if (current->container) {
			for (int i = 0; i < sections; i++) {
				float childMinDist = reducer.minDistance(obj, current->children[i]->bounds);
				bool containsSubNodes = (current->children[i]->container && current->children[i]->leafCount > 0);
				if ((containsSubNodes || current->children[i]->stored > 0) && childMinDist < minDist)
					dfs.emplace(-childMinDist, current->children[i]);
			}
		}
		// Update the closest elements
		else {
			for (int i = 0; i < current->stored; i++) {
				dist = reducer.distance(obj, *current->values[i]);
				if (dist <= minDist) {
					// Push the current element to the heap
					closest[n] = std::pair<float, Data*>(dist, current->values[i]);
					std::push_heap(closest, closest + n + 1, compare);

					// Use the highest distance in the heap as minDist.
					// This is the smallest distance that is guaranteed to not be in the top n
					// closest elements.
					minDist = closest[0].first;
					// Remove the highest so the heap now contains the n smallest visited values
					std::pop_heap(closest, closest + n + 1, compare);
				}
			}
		}
	}

	// Make a map out of the distances
	std::map<float, Data*> out;
	for (int i = 0; i < n; i ++) {
		if (closest[i].second) out.insert(closest[i]);
	}

	delete[] closest;

	return out;
}

template<class Data, class Bounds, unsigned int sections>
void quadtree::QuadTree<Data, Bounds, sections>::initialize(std::vector<Data> &data) {
	delete root;
	root = new Node(binSize, true, rootBounds, reducer);
	root->bounds = rootBounds;
	for (int i = 0; i < data.size(); i++) {
		insert(&data[i], root);
	}
}
