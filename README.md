# N-Body (work in progress)
One of the first projects I made when I was learning to code was a "gravity simulator". It worked by applying a force from each object to each other object every frame. This runs in O(n^2) time. I was thinking recently about how I could render huge amounts of objects every frame and I had the idea to use a quad-tree to approximate the forces. Distant objects can be averaged together to approximate forces and it can run in O(n log n) time.

## Current status
There is a working quadtree implementation, although it could use some optimization. Here is its current performance on 30000 randomly generated points:
```
Indexed in 0.00946163s
Found nearest point for each point in 0.0522884s using tree
                                      6.59628s   without tree

```
This is good enough for up to around a hundred thousand points, but after that it would be too slow for collision detection every frame. 

## Ideas for optimization
- The current benchmark runs in a single thread which means there is a lot of room for improvement by using multiple threads for queries.
- The current implementation for finding the closest point in quad A to point B searches sub-quads of A in order of the minimum distance to point B based on their bounding box. It might be more efficient to search in order of the average distance of points contained within each sub-quad of A to point B.
