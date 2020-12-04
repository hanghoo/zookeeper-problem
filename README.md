# zookeeper-problem
Dijkstra's algorithm, minimum spanning trees

Compile line: g++ zookeeper.c -lX11 -lm

Project description: The zookeeper has to visit all cages in the zoo (rectangles) but does not enter any cage. You write a function that gets an array of rectangles, and produces a closed polygonal path that touches each rectangle, but does not enter the interior of any rectangle. You write a function int zoopath( int *rectangles, int n, int *path, int m) The rectangles are given as an n × 4-array, the i-th rectangle being
[rectangles[i][0], rectangles[i][1]] × [rectangles[i][2], rectangles[i][3]]
For the polygonal path you receive a m × 2-array, which you fill in up to whatever path length you require; the return value of the function is the length of the path. I will provide again an xlib-based main function which calls your function, shows the rectangles and the path.

More description of the project is included on file "Project2.pdf".
