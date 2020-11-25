#include <stdbool.h>
#include <vector>
using namespace std; 

/* To store cell coordinates and rectangle # */
class Point
{
public:
	long int x; // x coordinate
	long int y; // y coordinate
	int id; // rectangle number	
};

/* To store edges */
class Edge
{
public:
	Point start; // start point
	Point end;   // end point
	int weight; // distance of the edge
};

/* vertex for Dijkastra */
class Vertex
{
public:
	Point vex;
	vector<Edge> neighbor; // save the neighbor edges
};



