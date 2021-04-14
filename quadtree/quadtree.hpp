#pragma once
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
		TreeReducer<Data, Bounds> reducer;

		Bounds bounds;

		bool container = false;
		TreeNode *children[sections];
		TreeNode(int binSize, bool container, Bounds bounds, TreeReducer<Data, Bounds> reducer): binSize(binSize), bounds(bounds) {
			values = new Data*[binSize];
			this->reducer=reducer;
			if (container) makeContainer();
		}

		void makeContainer() {
			for (int i = 0; i < sections; i++) {
				auto b = reducer.getBounds(this->bounds, i);
				children[i] = new TreeNode<Data, Bounds, sections>(binSize, false, b, reducer);
			}
			for (int i = 0; i < stored; i++) {
				for (int j = 0; j < sections; j++) {
					if (reducer.inBounds(*values[i], children[j]->bounds)) {
						children[j]->values[children[j]->stored] = values[i];
						children[j]->stored++;
					}
					break;
				}
			}
			stored = 0;
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

			root = new Node(binSize, true, rootBounds, reducer);
		}
		~QuadTree() {
			delete root;
		}

		Data* nearest(Data obj);

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
	std::priority_queue<std::pair<float, Node*>> dfs;

	int visited = 0;
	dfs.emplace(0.f, root);
	
	float minDist = INFINITY;
	Data *closest;

	while (dfs.size() > 0) {
		Node *current = dfs.top().second;
		float dist = -dfs.top().first;
		dfs.pop();

		if (current->leafCount == 0 || dist >= minDist) {
			//continue;
		}
		if (current->container) {
			for (int i = 0; i < sections; i++) {
				dfs.emplace(-reducer.minDistance(obj, current->children[i]->bounds), current->children[i]);
			}
		}
		else {
			for (int i = 0; i < current->stored; i++) {
				dist = reducer.distance(obj, *current->values[i]);
				visited++;
//				std::cout << obj.first << " " << obj.second << " " << current->values[i]->first << " " << current->values[i]->second << std::endl;
				if (dist <= minDist) {
					minDist = dist;
					closest = current->values[i];
				}
			}
		}
	}

	std::cout << visited << " visited" << std::endl;

	return closest;
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
