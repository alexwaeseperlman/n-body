#pragma once
#include <algorithm>
#include <map>
#include <unordered_set>
#include <vector>
#include <math.h>
#include <queue>
#include <iostream>
#include <unordered_map>

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
		std::unordered_set<Data*> values;
		int leafCount = 0;
		TreeReducer<Data, Bounds> *reducer;

		Bounds bounds;

		bool container = false;
		std::array<TreeNode*, sections> children;
		TreeNode* parent;

		TreeNode(bool container, Bounds bounds, TreeReducer<Data, Bounds> *reducer, TreeNode* parent=nullptr): bounds(bounds), parent(parent) {
			this->reducer = reducer;
			if (container) makeContainer();
		}

		// Move all stored data into child nodes
		void makeContainer() {
			if (this->container) return;
			leafCount = values.size();

			for (int i = 0; i < sections; i++) {
				auto b = reducer->getBounds(this->bounds, i);
				children[i] = new TreeNode<Data, Bounds, sections>(false, b, reducer, this);
			}

			for (auto value : values) {
				for (int j = 0; j < sections; j++) {
					if (reducer->inBounds(*value, children[j]->bounds)) {
						children[j]->values.insert(value);
						break;
					}
				}
			}
			values.clear();
			this->container = true;
		}

		// Absorb all data from child nodes/containers
		void makeStorage() {
			if (!this->container) return;
			for (int i = 0; i < sections; i++) {
				if (children[i]->container) children[i]->makeStorage();
				for (Data* data : children[i]->values) {
					values.insert(data);
				}
				delete children[i];
			}

			this->container = false;
		}

		~TreeNode() {
			if (container) for (int i = 0; i < sections; i++) delete children[i];
		}
	};

	
	template<class Data = pairf, class Bounds=std::pair<pairf, pairf>, unsigned int sections=4>
	class QuadTree {
	public:
		using Node=TreeNode<Data, Bounds, sections>;
		int binSize;
		QuadTree(
				int binSize,
				Bounds rootBounds = std::pair<pairf, pairf>{ {-1.f, -1.f}, {1.f, 1.f} },
				TreeReducer<Data, Bounds> reducer = {.distance=distance, .inBounds=inBounds, .minDistance=minDistance, .getBounds=getBounds}) 
			: binSize(binSize), rootBounds(rootBounds), reducer(reducer) {

			root = new Node(true, rootBounds, &this->reducer);
		}
		~QuadTree() {
			delete root;
		}

		std::unordered_map<Data*, Node*> dataLocations;

		Data* nearest(Data obj) const;
		std::map<float, Data*> nearest(Data obj, int n) const;

		// Insert a node, creating containers as necessary
		// TODO: There should be a maximum recursion depth for insert 
		// 		 if the limit is exceeded it's ok to create a node with > binSize data
		Node* insert(Data& data, Node* node);

		// Initialize the quadtree with a vector
		void initialize(std::vector<Data> &data);

		// Store the parent node of every piece of data under node in dataLocations
		void indexData(Node* node);

		// Update the index of a piece of data after a change
		bool update(Data& data);

		// Update the every item in the tree
		void reindex() { reindex(root); }
		void reindex(Node* node);

		Node* remove(Data& data);

		TreeReducer<Data, Bounds> reducer;
		Node *root;
		Bounds rootBounds;
	};
}

template<class Data, class Bounds, unsigned int sections>
quadtree::TreeNode<Data, Bounds, sections>*
quadtree::QuadTree<Data, Bounds, sections>::insert(Data& data, Node *node) {
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
	if (node->values.size() < binSize) {
		node->values.insert(&data);
		dataLocations[&data] = node;
		return node;
	}

	// Reindex the node if necessary
	node->makeContainer();
	indexData(node);

	return insert(data, node);
}

template<class Data, class Bounds, unsigned int sections>
Data* quadtree::QuadTree<Data, Bounds, sections>::nearest(Data obj) const {
	return nearest(obj, 1).begin()->second;
}
template<class Data, class Bounds, unsigned int sections>
std::map<float, Data*> quadtree::QuadTree<Data, Bounds, sections>::nearest(Data obj, int n) const {
	auto compare = [](std::pair<float, void*> l, std::pair<float, void*> r) {
		return l.first < r.first;
	};

	std::priority_queue<std::pair<float, Node*>, std::vector<std::pair<float, Node*>>, decltype(compare)> dfs(compare);
	dfs.emplace(0.f, root);
	
	float minDist = INFINITY;
	// Save memory by using a raw heap instead of a priority queue
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
				if ((containsSubNodes || current->children[i]->values.size() > 0) && childMinDist < minDist)
					dfs.emplace(-childMinDist, current->children[i]);
			}
		}
		// Update the closest elements
		else {
			for (auto value : current->values) {
				float currentDist = reducer.distance(obj, *value);
				if (currentDist <= minDist) {
					// Push the current element to the heap
					closest[n] = std::pair<float, Data*>(currentDist, value);
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
void quadtree::QuadTree<Data, Bounds, sections>::indexData(Node* node) {
	if (node->container) {
		for (int i = 0; i < sections; i++) indexData(node->children[i]);
		return;
	}
	for (auto value : node->values) {
		dataLocations[value] = node;
	}
}

template<class Data, class Bounds, unsigned int sections>
quadtree::TreeNode<Data, Bounds, sections>* quadtree::QuadTree<Data, Bounds, sections>::remove(Data& data) {
	Node* node = dataLocations[&data];

	node->values.erase(&data);

	dataLocations.erase(&data);

	// Propagate the new number of leaf nodes up the tree
	// Store the last node that doesn't need to be a container
	Node* topNonContainer = nullptr;

	while (node->parent != nullptr) {
		if (node->container) node->leafCount--;
		if (node->leafCount <= binSize) topNonContainer = node;
		node = node->parent;
	}

	if (topNonContainer) {
		topNonContainer->makeStorage();
		indexData(topNonContainer);
		return topNonContainer;
	}

	return node;
}


template<class Data, class Bounds, unsigned int sections>
bool quadtree::QuadTree<Data, Bounds, sections>::update(Data& target) {
	if (!dataLocations[&target]) return false;

	Node *node = remove(target);
	
	// Move upwards until the new value is in bounds
	while (node->parent && !reducer.inBounds(target, node->bounds)) {
		node = node->parent;
	}
	insert(target, root);
	return true;
}

template<class Data, class Bounds, unsigned int sections>
void quadtree::QuadTree<Data, Bounds, sections>::reindex(Node* root) {
	// Reset the leafNodes and storage of all subtrees
	std::queue<Node *> bfs;
	bfs.push(root);
	while (bfs.size()) {
		Node* top = bfs.front();
		bfs.pop();
		
		top->leafCount = 0;
		top->values.clear();
		if (top->container) for (int i = 0; i < sections; i++) bfs.push(top->children[i]);
	}

	for (auto data : dataLocations) {
		insert(*data.first, root);
	}
}

template<class Data, class Bounds, unsigned int sections>
void quadtree::QuadTree<Data, Bounds, sections>::initialize(std::vector<Data> &data) {
	delete root;
	root = new Node(true, rootBounds, &reducer);
	root->bounds = rootBounds;
	for (int i = 0; i < data.size(); i++) {
		insert(data[i], root);
	}
}


