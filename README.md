# N-Body (work in progress)
One of the first projects I made when I was learning to code was a "gravity simulator". It worked by applying a force from each object to each object every frame. This runs in O(n^2) time. I was thinking recently about how I could render huge amounts of objects every frame and I had the idea to use a quad-tree to approximate the forces. Distant objects can be averaged together to create similar forces and it can run in O(n log n) time.

