/* compiles with command line  gcc xlibdemo.c -lX11 -lm -L/usr/X11R6/lib */
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <bits/stdc++.h>
#include <vector>
#include "cell.h"

Display *display_ptr;
Screen *screen_ptr;
int screen_num;
char *display_name = NULL;
unsigned int display_width, display_height;

Window win;
int border_width;
unsigned int win_width, win_height;
int win_x, win_y;

XWMHints *wm_hints;
XClassHint *class_hints;
XSizeHints *size_hints;
XTextProperty win_name, icon_name;
char *win_name_string = "Example Window";
char *icon_name_string = "Icon for Example Window";

XEvent report;

GC gc, gc_yellow, gc_red, gc_grey;
unsigned long valuemask = 0;
XGCValues gc_values, gc_yellow_values, gc_red_values, gc_grey_values;
Colormap color_map;
XColor tmp_color1, tmp_color2;

int zoopath( int a[][4], int b, int c[][2], int d);

void find_edges(Point src, Point dest, vector<Edge> edges, int obstacle_rec[][4], int b);

int find_valid_edges(Edge e1, Edge e2);

// convert Windows to graph
//void rec2graph(int rec[][4], int graph[][500], int num_of_rec);

// check whether cell is a valid cell or not
//bool is_valid(int row, int col);

// find the shortest path between source cell and destination cell
//double BFS(int mat[][500], Point src, Point dest);

// these arrays are used to get row and column num of 8 neighbors of a given cell
//int rowNum[] = {-1, -1, -1,  0, 0,  1, 1, 1};
//int colNum[] = {-1,  0,  1, -1, 1, -1, 0, 1};

int main(int argc, char **argv)
{ int rectangles[200][4];
  int path[1000][2];
  int pathlength, rectanglenumber;
  int i;

  // provide the graph. Convert Windows to graph
  int graph[500][500] = {0};
  /* opening display: basic connection to X Server */
  if( argc == 2 )
  {  FILE *input;
     input = fopen( argv[1], "r");
     if( input != NULL )
       printf("opened input file %s\n", argv[1] );
     else
       {  printf("cannot open input file %s\n", argv[1] );
	  exit( 0 );
       }
     i=0;
     while( fscanf( input, "[%d,%d]x[%d,%d]\n",
		    &(rectangles[i][0]), &(rectangles[i][1]),
		    &(rectangles[i][2]), &(rectangles[i][3]) ) == 4 )
       i+=1;
     printf("found %d rectangles in file %s\n", i, argv[1] );
     rectanglenumber = i; pathlength = 0;
  }
  if( (display_ptr = XOpenDisplay(display_name)) == NULL )
    { printf("Could not open display. \n"); exit(-1);}
  printf("Connected to X server  %s\n", XDisplayName(display_name) );
  screen_num = DefaultScreen( display_ptr );
  screen_ptr = DefaultScreenOfDisplay( display_ptr );
  color_map  = XDefaultColormap( display_ptr, screen_num );
  display_width  = DisplayWidth( display_ptr, screen_num );
  display_height = DisplayHeight( display_ptr, screen_num );

  printf("Width %d, Height %d, Screen Number %d\n", 
           display_width, display_height, screen_num);
  
  /* creating the window */
  border_width = 10;
  win_x = 0; win_y = 0;
  win_width = display_width/2;
  win_height = (int) (win_width / 1.7); /*rectangular window*/
  
  win= XCreateSimpleWindow( display_ptr, RootWindow( display_ptr, screen_num),
                            win_x, win_y, win_width, win_height, border_width,
                            BlackPixel(display_ptr, screen_num),
                            WhitePixel(display_ptr, screen_num) );
  /* now try to put it on screen, this needs cooperation of window manager */
  size_hints = XAllocSizeHints();
  wm_hints = XAllocWMHints();
  class_hints = XAllocClassHint();
  if( size_hints == NULL || wm_hints == NULL || class_hints == NULL )
    { printf("Error allocating memory for hints. \n"); exit(-1);}

  size_hints -> flags = PPosition | PSize | PMinSize  ;
  size_hints -> min_width = 60;
  size_hints -> min_height = 60;

  XStringListToTextProperty( &win_name_string,1,&win_name);
  XStringListToTextProperty( &icon_name_string,1,&icon_name);
  
  wm_hints -> flags = StateHint | InputHint ;
  wm_hints -> initial_state = NormalState;
  wm_hints -> input = False;

  class_hints -> res_name = "x_use_example";
  class_hints -> res_class = "examples";

  XSetWMProperties( display_ptr, win, &win_name, &icon_name, argv, argc,
                    size_hints, wm_hints, class_hints );

  /* what events do we want to receive */
  XSelectInput( display_ptr, win, 
            ExposureMask | StructureNotifyMask | ButtonPressMask );
  
  /* finally: put window on screen */
  XMapWindow( display_ptr, win );

  XFlush(display_ptr);

  /* create graphics context, so that we may draw in this window */
  gc = XCreateGC( display_ptr, win, valuemask, &gc_values);
  XSetForeground( display_ptr, gc, BlackPixel( display_ptr, screen_num ) );
  XSetLineAttributes( display_ptr, gc, 6, LineSolid, CapRound, JoinRound);
  /* and three other graphics contexts, to draw in yellow and red and grey*/
  gc_yellow = XCreateGC( display_ptr, win, valuemask, &gc_yellow_values);
  XSetLineAttributes(display_ptr, gc_yellow, 3, LineSolid,CapRound, JoinRound);
  if( XAllocNamedColor( display_ptr, color_map, "yellow", 
			&tmp_color1, &tmp_color2 ) == 0 )
    {printf("failed to get color yellow\n"); exit(-1);} 
  else
    XSetForeground( display_ptr, gc_yellow, tmp_color1.pixel );
  gc_red = XCreateGC( display_ptr, win, valuemask, &gc_red_values);
  XSetLineAttributes( display_ptr, gc_red, 3, LineSolid, CapRound, JoinRound);
  if( XAllocNamedColor( display_ptr, color_map, "red", 
			&tmp_color1, &tmp_color2 ) == 0 )
    {printf("failed to get color red\n"); exit(-1);} 
  else
    XSetForeground( display_ptr, gc_red, tmp_color1.pixel );
  gc_grey = XCreateGC( display_ptr, win, valuemask, &gc_grey_values);
  if( XAllocNamedColor( display_ptr, color_map, "light grey", 
			&tmp_color1, &tmp_color2 ) == 0 )
    {printf("failed to get color grey\n"); exit(-1);} 
  else
    XSetForeground( display_ptr, gc_grey, tmp_color1.pixel );

  /* and now it starts: the event loop */
  while(1)
    { XNextEvent( display_ptr, &report );
      switch( report.type )
	{
	case Expose:
          /* re-draw the rectangles. This event happens
             each time some part of the window gets exposed (becomes visible) */

    for( i=0; i< rectanglenumber; i++)
	  {
	      XDrawLine(display_ptr, win, gc,
			rectangles[i][0], rectangles[i][2],
			rectangles[i][0], rectangles[i][3] );
	      XDrawLine(display_ptr, win, gc,
			rectangles[i][1], rectangles[i][2],
			rectangles[i][1], rectangles[i][3] );
	      XDrawLine(display_ptr, win, gc,
			rectangles[i][0], rectangles[i][2],
			rectangles[i][1], rectangles[i][2] );
	      XDrawLine(display_ptr, win, gc,
			rectangles[i][0], rectangles[i][3],
			rectangles[i][1], rectangles[i][3] );
          }
	  if( pathlength > 0 )
	  { for( i=0; i< pathlength -1; i++)
	    {
	      XDrawLine(display_ptr, win, gc_red,
			path[i][0], path[i][1],
			path[i+1][0], path[i+1][1]);
	    }
	  }
	  else
	    printf("No path defined\n");
          break;
        case ConfigureNotify:
          /* This event happens when the user changes the size of the window*/
          win_width = report.xconfigure.width;
          win_height = report.xconfigure.height;
          break;
        case ButtonPress:
          /* This event happens when the user pushes a mouse button. I draw
            a circle to show the point where it happened, but do not save 
            the position; so when the next redraw event comes, these circles
	    disappear again. */
          {  /*
             int x, y;
  	     x = report.xbutton.x;
             y = report.xbutton.y;
             */
	     printf("calling zoopath function \n");

	     pathlength = zoopath(rectangles, rectanglenumber, path, 1000);
             printf("zoopath generated path of length %d\n", pathlength);
          }
          /* no break here; fall through to redraw case */
        default: /* redraw everything */
	  for( i=0; i< rectanglenumber; i++)
	  {
	      XDrawLine(display_ptr, win, gc,
			rectangles[i][0], rectangles[i][2],
			rectangles[i][0], rectangles[i][3] );
	      XDrawLine(display_ptr, win, gc,
			rectangles[i][1], rectangles[i][2],
			rectangles[i][1], rectangles[i][3] );
	      XDrawLine(display_ptr, win, gc,
			rectangles[i][0], rectangles[i][2],
			rectangles[i][1], rectangles[i][2] );
	      XDrawLine(display_ptr, win, gc,
			rectangles[i][0], rectangles[i][3],
			rectangles[i][1], rectangles[i][3] );
          }
	  if( pathlength > 0 )
	  { for( i=0; i< pathlength -1; i++)
	    {
	      XDrawLine(display_ptr, win, gc_red,
			path[i][0], path[i][1],
			path[i+1][0], path[i+1][1]);
	    }
	  }
	  else
	    printf("No path defined\n");

	  
          break;
	}

    }
  exit(0);
}

int zoopath(int a[][4], int b, int c[][2], int d)
{
  // pick 2 rectangles from the 11 rectangles and calculate shortest distance
  /*for(int i=0; i<b-1; i++){
    for(int j=i+1; j<b, j++){
       TODO: this part can archive pick all possible 2 rectangles 
    }
  }*/

  // get the center coordinate of each rectangle
  Point src = {55,55,0};
  Point dest = {55,235,1};
  vector<Edge> edges;

  find_edges(src, dest, edges, a, b);
  
  return(0);
}

/* find possible edge for src and dest */
/* obtain the valid edges for each source and destination 
   for Dijkastra's algorithm */
void find_edges(Point src, Point dest, vector<Edge> edges, int obstacle_rec[][4], int b){

  /* find any edge of a rectangle */
  Edge tmp;
  vector<Edge> rec_edges; // temporary save rectangle sides
  /*for(int i=0; i<b; i++){
    int x_1 = obstacle_rec[i][2];
    int x_2 = obstacle_rec[i][3];
    int y_1 = obstacle_rec[i][0];
    int y_2 = obstacle_rec[i][1];
    printf("X_1: %d, X_2: %d, Y_1: %d, Y_2: %d\n\n\n", x_1, x_2, y_1, y_2);
  }*/

  /* creat rectangle edges(without src and dest) and save them begin */
  for(int i=0; i<b; i++){
    if(i != src.id && i != dest.id){ // exlude the source and destination rec
      // (x1,y1), (x1, y2) horizontal line
      tmp.src_x = obstacle_rec[i][2];
      tmp.src_y = obstacle_rec[i][0];
      tmp.dest_x = obstacle_rec[i][2];
      tmp.dest_y = obstacle_rec[i][1];
      //tmp.dist = sqrt(pow((tmp.dest_y-tmp.src_y),2) + pow((tmp.dest_x-tmp.src_x),2));
      //printf("the edge distance is: %2d\n", tmp.dist);
      //printf("X_1: %d, X_2: %d, Y_1: %d, Y_2: %d\n", tmp.src_x, tmp.dest_x, tmp.src_y, tmp.dest_y);
      //printf("the edge distance is: %.2f\n", tmp.dist);
      rec_edges.push_back(tmp);

      // (x2,y1), (x2, y2) horizontal line
      tmp.src_x = obstacle_rec[i][3];
      tmp.src_y = obstacle_rec[i][0];
      tmp.dest_x = obstacle_rec[i][3];
      tmp.dest_y = obstacle_rec[i][1];
      //tmp.dist = sqrt(pow((tmp.dest_y-tmp.src_y),2) + pow((tmp.dest_x-tmp.src_x),2));
      //printf("the edge distance is: %.2f\n", tmp.dist);
      rec_edges.push_back(tmp);

      // (x1,y1), (x2, y1) vertical line
      tmp.src_x = obstacle_rec[i][2];
      tmp.src_y = obstacle_rec[i][0];
      tmp.dest_x = obstacle_rec[i][3];
      tmp.dest_y = obstacle_rec[i][0];
      //tmp.dist = sqrt(pow((tmp.dest_y-tmp.src_y),2) + pow((tmp.dest_x-tmp.src_x),2));
      //printf("the edge distance is: %.2f\n", tmp.dist);
      rec_edges.push_back(tmp);

      // (x1,y2), (x2, y2) vertical line
      tmp.src_x = obstacle_rec[i][2];
      tmp.src_y = obstacle_rec[i][1];
      tmp.dest_x = obstacle_rec[i][3];
      tmp.dest_y = obstacle_rec[i][1];
      //tmp.dist = sqrt(pow((tmp.dest_y-tmp.src_y),2) + pow((tmp.dest_x-tmp.src_x),2));
      //printf("the edge distance is: %.2f\n", tmp.dist);
      rec_edges.push_back(tmp);
    }
    else
      continue;    
  }
  printf("The size of vector(should be 36) is: %ld\n", rec_edges.size());
  /* above end */

  /* create edges by connecting all the rectangles corners with another rectangle corner 
     ignoring the corners of source, destination and disgonal */
  for(int i=0; i<b-1; i++){
    for(int j=i+1; j<b; j++){
      if( i != src.id && i != dest.id && j != src.id && j != dest.id){
        // case 1: i:(x1,y1); j:(x1,y1)
        // x1:2; x2:3; y1:0; y2:1
        tmp.src_x = obstacle_rec[i][2];
        tmp.src_y = obstacle_rec[i][0];
        tmp.dest_x = obstacle_rec[j][2];
        tmp.dest_y = obstacle_rec[j][0];
        edges.push_back(tmp);
        
        // case 2: i:(x1,y1); j:(x1,y2)
        // x1:2; x2:3; y1:0; y2:1
        tmp.src_x = obstacle_rec[i][2];
        tmp.src_y = obstacle_rec[i][0];
        tmp.dest_x = obstacle_rec[j][2];
        tmp.dest_y = obstacle_rec[j][1];
        edges.push_back(tmp);

        // case 3: i:(x1,y1); j:(x2,y1)
        // x1:2; x2:3; y1:0; y2:1
        tmp.src_x = obstacle_rec[i][2];
        tmp.src_y = obstacle_rec[i][0];
        tmp.dest_x = obstacle_rec[j][3];
        tmp.dest_y = obstacle_rec[j][0];
        edges.push_back(tmp);

        // case 4: i:(x1,y1); j:(x2,y2)
        // x1:2; x2:3; y1:0; y2:1
        tmp.src_x = obstacle_rec[i][2];
        tmp.src_y = obstacle_rec[i][0];
        tmp.dest_x = obstacle_rec[j][3];
        tmp.dest_y = obstacle_rec[j][1];
        edges.push_back(tmp);


        // case 5: i:(x1,y2); j:(x1,y1)
        // x1:2; x2:3; y1:0; y2:1
        tmp.src_x = obstacle_rec[i][2];
        tmp.src_y = obstacle_rec[i][1];
        tmp.dest_x = obstacle_rec[j][2];
        tmp.dest_y = obstacle_rec[j][0];
        edges.push_back(tmp);
        
        // case 6: i:(x1,y2); j:(x1,y2)
        // x1:2; x2:3; y1:0; y2:1
        tmp.src_x = obstacle_rec[i][2];
        tmp.src_y = obstacle_rec[i][1];
        tmp.dest_x = obstacle_rec[j][2];
        tmp.dest_y = obstacle_rec[j][1];
        edges.push_back(tmp);

        // case 7: i:(x1,y2); j:(x2,y1)
        // x1:2; x2:3; y1:0; y2:1
        tmp.src_x = obstacle_rec[i][2];
        tmp.src_y = obstacle_rec[i][1];
        tmp.dest_x = obstacle_rec[j][3];
        tmp.dest_y = obstacle_rec[j][0];
        edges.push_back(tmp);

        // case 8: i:(x1,y2); j:(x2,y2)
        // x1:2; x2:3; y1:0; y2:1
        tmp.src_x = obstacle_rec[i][2];
        tmp.src_y = obstacle_rec[i][1];
        tmp.dest_x = obstacle_rec[j][3];
        tmp.dest_y = obstacle_rec[j][1];
        edges.push_back(tmp);


        // case 9: i:(x2,y1); j:(x1,y1)
        // x1:2; x2:3; y1:0; y2:1
        tmp.src_x = obstacle_rec[i][3];
        tmp.src_y = obstacle_rec[i][0];
        tmp.dest_x = obstacle_rec[j][2];
        tmp.dest_y = obstacle_rec[j][0];
        edges.push_back(tmp);
        
        // case 10: i:(x2,y1); j:(x1,y2)
        // x1:2; x2:3; y1:0; y2:1
        tmp.src_x = obstacle_rec[i][3];
        tmp.src_y = obstacle_rec[i][0];
        tmp.dest_x = obstacle_rec[j][2];
        tmp.dest_y = obstacle_rec[j][1];
        edges.push_back(tmp);

        // case 11: i:(x2,y1); j:(x2,y1)
        // x1:2; x2:3; y1:0; y2:1
        tmp.src_x = obstacle_rec[i][3];
        tmp.src_y = obstacle_rec[i][0];
        tmp.dest_x = obstacle_rec[j][3];
        tmp.dest_y = obstacle_rec[j][0];
        edges.push_back(tmp);

        // case 12: i:(x2,y1); j:(x2,y2)
        // x1:2; x2:3; y1:0; y2:1
        tmp.src_x = obstacle_rec[i][3];
        tmp.src_y = obstacle_rec[i][0];
        tmp.dest_x = obstacle_rec[j][3];
        tmp.dest_y = obstacle_rec[j][1];
        edges.push_back(tmp);


        // case 13: i:(x2,y2); j:(x1,y1)
        // x1:2; x2:3; y1:0; y2:1
        tmp.src_x = obstacle_rec[i][3];
        tmp.src_y = obstacle_rec[i][1];
        tmp.dest_x = obstacle_rec[j][2];
        tmp.dest_y = obstacle_rec[j][0];
        edges.push_back(tmp);
        
        // case 14: i:(x2,y2); j:(x1,y2)
        // x1:2; x2:3; y1:0; y2:1
        tmp.src_x = obstacle_rec[i][3];
        tmp.src_y = obstacle_rec[i][1];
        tmp.dest_x = obstacle_rec[j][2];
        tmp.dest_y = obstacle_rec[j][1];
        edges.push_back(tmp);

        // case 15: i:(x2,y2); j:(x2,y1)
        // x1:2; x2:3; y1:0; y2:1
        tmp.src_x = obstacle_rec[i][3];
        tmp.src_y = obstacle_rec[i][1];
        tmp.dest_x = obstacle_rec[j][3];
        tmp.dest_y = obstacle_rec[j][0];
        edges.push_back(tmp);

        // case 16: i:(x2,y2); j:(x2,y2)
        // x1:2; x2:3; y1:0; y2:1
        tmp.src_x = obstacle_rec[i][3];
        tmp.src_y = obstacle_rec[i][1];
        tmp.dest_x = obstacle_rec[j][3];
        tmp.dest_y = obstacle_rec[j][1];
        edges.push_back(tmp);
      }

      else
        continue;
    }
  }
  printf("The size of vector(should be 576) is: %ld\n", edges.size());
  /* above end */


  /* create edges by connecting source center/destination center with any rectangle corner */
  for(int i=0; i<b; i++){
    if(i != src.id){ // for source rectangle center
      // case 1: (src.x, src.y) is the center of the rectangle, (x1, y1)
      // x1:2; x2:3; y1:0; y2:1 
      tmp.src_x = src.x;
      tmp.src_y = src.y;
      tmp.dest_x = obstacle_rec[i][2];
      tmp.dest_y = obstacle_rec[i][0];
      edges.push_back(tmp);

      // case 2: (src.x, src.y) is the center of the rectangle, (x1, y2)
      // x1:2; x2:3; y1:0; y2:1 
      tmp.src_x = src.x;
      tmp.src_y = src.y;
      tmp.dest_x = obstacle_rec[i][2];
      tmp.dest_y = obstacle_rec[i][1];
      edges.push_back(tmp);

      // case 3: (src.x, src.y) is the center of the rectangle, (x2, y1)
      // x1:2; x2:3; y1:0; y2:1 
      tmp.src_x = src.x;
      tmp.src_y = src.y;
      tmp.dest_x = obstacle_rec[i][3];
      tmp.dest_y = obstacle_rec[i][0];
      edges.push_back(tmp);

      // case 4: (src.x, src.y) is the center of the rectangle, (x2, y2)
      // x1:2; x2:3; y1:0; y2:1 
      tmp.src_x = src.x;
      tmp.src_y = src.y;
      tmp.dest_x = obstacle_rec[i][3];
      tmp.dest_y = obstacle_rec[i][1];
      edges.push_back(tmp);
    }
  }
  printf("The size of vector(should be 616) is: %ld\n", edges.size());

  for(int i=0; i<b; i++){
    if(i != dest.id){ // for destination rectangle center
      // case 1: (dest.x, dest.y) is the center of the rectangle, (x1, y1)
      // x1:2; x2:3; y1:0; y2:1 
      tmp.src_x = dest.x;
      tmp.src_y = dest.y;
      tmp.dest_x = obstacle_rec[i][2];
      tmp.dest_y = obstacle_rec[i][0];
      edges.push_back(tmp);

      // case 2: (dest.x, dest.y) is the center of the rectangle, (x1, y2)
      // x1:2; x2:3; y1:0; y2:1 
      tmp.src_x = dest.x;
      tmp.src_y = dest.y;
      tmp.dest_x = obstacle_rec[i][2];
      tmp.dest_y = obstacle_rec[i][1];
      edges.push_back(tmp);

      // case 3: (dest.x, dest.y) is the center of the rectangle, (x2, y1)
      // x1:2; x2:3; y1:0; y2:1 
      tmp.src_x = dest.x;
      tmp.src_y = dest.y;
      tmp.dest_x = obstacle_rec[i][3];
      tmp.dest_y = obstacle_rec[i][0];
      edges.push_back(tmp);

      // case 4: (dest.x, dest.y) is the center of the rectangle, (x2, y2)
      // x1:2; x2:3; y1:0; y2:1 
      tmp.src_x = dest.x;
      tmp.src_y = dest.y;
      tmp.dest_x = obstacle_rec[i][3];
      tmp.dest_y = obstacle_rec[i][1];
      edges.push_back(tmp);
    }
  }
  printf("The size of vector(should be 656) is: %ld\n", edges.size());

  /* judge whether the edge crosses with rectangle sides (exclude source and destination rectangle) */
  // rectangle sides size
  int rec_size = rec_edges.size();
  printf("The size of rectangle sides(should be 36) is: %d\n", rec_size);

  // vertex connection line num (exclude rectangle sides)
  int ver_size = edges.size();
  printf("The size of vertex connection(should be 656) is: %d\n", ver_size);

  vector<Edge> valid_edges; // store valid candidate edges
  int count = 0;
  int num = 0;

  for(int i=0; i<ver_size; i++){
    for(int j=0; j<rec_size; j++){
      /* orientation(abp)*orientation(abq) < 0 and
         orientation(pqa)*orientation(pqb) < 0.
         Here orientation(ABC) = Ax*By - Ay*Bx + Bx* Cy - By*Cx + Cx*Ay - Cy* Ax
      */
      /* a,b = rec_edges[i]; p,q = edges[j]
      */
      if(!find_valid_edges(rec_edges[j], edges[i])){ // cross 
        //tmp.erase(edges.begin() + j);
        //printf("Current edge is deleted %d\n", j);
        count++;
      }
      else // not cross
        continue; 

      //printf("the value of count is %d\n", count);       
    } // inner loop end; traverse all rectangle sides; if exist 1 time cross, this candidate doesn't satisfy 

    // if count == 0 means this edge doesn't cross with any rectangle sides
    if(count == 0){
      valid_edges.push_back(edges[i]);
    }
    else{
      num += 1;
    }
    count = 0; // update count 
  } // outside loop end

  printf("The num is: %d\n", num);
  printf("The valid edges are: %ld\n", valid_edges.size());

}// func end

/* selected non-cross edges */
int find_valid_edges(Edge e1, Edge e2){ // e1 means rectangle sides; e2 means candidate edges

  /* orientation(abp)*orientation(abq) < 0 and
         orientation(pqa)*orientation(pqb) < 0.
         Here orientation(ABC) = Ax*By - Ay*Bx + Bx* Cy - By*Cx + Cx*Ay - Cy* Ax
         Here orientation(PQA) = Px*Qy - Py*Qx + Qx* Ay - Qy*Ax + Ax*Py - Ay* Px
      */
      /* a,b = rec_edges[i]; p,q = edges[j]
      */
      if( ( (e1.src_x * e1.dest_y - e1.src_y * e1.dest_x + 
             e1.dest_x * e2.src_y - e1.dest_y * e2.src_x +
             e2.src_x * e1.src_y - e2.src_y * e1.src_x) * 
            (e1.src_x * e1.dest_y - e1.src_y * e1.dest_x +
             e1.dest_x * e2.dest_y - e1.dest_y * e2.dest_x +
             e2.dest_x * e1.src_y - e2.dest_y * e1.src_x ) < 0) 
        && 
          ( (e2.src_x * e2.dest_y - e2.src_y * e2.dest_x +
             e2.dest_x * e1.src_y - e2.dest_y * e1.src_x +
             e1.src_x * e2.src_y - e1.src_y * e2.src_x) * 
            (e2.src_x * e2.dest_y - e2.src_y * e2.dest_x +
             e2.dest_x * e1.dest_y - e2.dest_y * e1.dest_x +
             e1.dest_x * e2.src_y - e1.dest_y * e2.src_x) < 0)) {
        
        return 0; // 0 means these 2 edges cross
      }
      else
        return 1; // 1 means these 2 edges not cross

}
