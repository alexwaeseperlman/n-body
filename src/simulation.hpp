#include <quadtree/quadtree.hpp>
#include <vector>
#include <unordered_map>

#define MAX_BODIES 100000

namespace simulation {
	struct Body {
		std::pair<float, float> position;
		float radius, mass;
		std::pair<float, float> velocity;
	};

	using Bounds = std::pair<std::pair<float, float>, std::pair<float, float>>;

	float distance(const Body &a, const Body &b);
	bool inBounds(const Body &x, const Bounds &b);
	float minDistance(const Body &x, const Bounds &b);

	class Simulation {
		public:
			// Max collisions is the number of collisions that can be handled per body
			void step(float time, int maxCollisions=5);
			void addBody(Body &point);
			const std::vector<Body> &getData() {
				return data;
			}

		private:
			const quadtree::TreeReducer<Body, Bounds> reducer{
				.distance = distance, 
				.inBounds = inBounds, 
				.minDistance = minDistance, 
				.getBounds = quadtree::getBounds 
			};
			quadtree::QuadTree<Body> points = quadtree::QuadTree<Body>(4, Bounds{ {-1.f, -1.f}, {1.f, 1.f} }, reducer);
			std::vector<Body> data;
			void handleCollision(Body *a, Body *b);

			Body *collisions[MAX_BODIES];
	};
}
