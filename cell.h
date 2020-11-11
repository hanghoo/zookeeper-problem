#include <stdbool.h>
//#include <vector>
using namespace std; 


/* To store matrix cell coordinates */
class Point
{
public:
	int x; // x coordinate
	int y; // y coordinate
	int id; // inner value of rectangle
};

/* To store edges */
class Edge
{
public:
	int src_x; // source x
	int src_y; // source y
	int dest_x; // destination x
	int dest_y; // destination y
	double dist; // distance of the edge
};
